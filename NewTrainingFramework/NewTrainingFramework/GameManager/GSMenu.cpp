#include "stdafx.h"
#include "GSMenu.h"
#include "GameStateMachine.h"
#include "../Core/Globals.h"
#include <iostream>

GSMenu::GSMenu() 
    : GameStateBase(StateType::MENU), m_selectedButton(0), m_buttonTimer(0.0f) {
}

GSMenu::~GSMenu() {
}

void GSMenu::Init() {
    std::cout << "=== MAIN MENU ===" << std::endl;
    std::cout << "Welcome to the Game!" << std::endl;
    
    // Enable blending for transparent textures (để bỏ góc đen)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Setup 2D camera for menu
    m_camera2D = std::make_unique<Camera>();
    float aspect = (float)Globals::screenWidth / (float)Globals::screenHeight;
    
    // Use orthographic projection for 2D UI
    m_camera2D->SetOrthographic(-aspect, aspect, -1.0f, 1.0f, 0.1f, 100.0f);
    m_camera2D->SetLookAt(Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
    
    // Create background
    m_backgroundObject = std::make_unique<Object>(100);
    m_backgroundObject->SetModel(0);    // Sprite2D model
    m_backgroundObject->SetTexture(1, 0); // Background texture (bg_play1.tga)
    m_backgroundObject->SetShader(0);   // Basic shader
    m_backgroundObject->SetPosition(Vector3(0.0f, 0.0f, -0.1f)); // Behind UI elements
    m_backgroundObject->SetScale(Vector3(2.5f, 2.5f, 1.0f)); // Scale to cover screen
    
    CreateButtons();
    
    std::cout << "Use W/S keys to navigate, ENTER to select" << std::endl;
    std::cout << "ESC to exit game" << std::endl;
}

void GSMenu::CreateButtons() {
    m_buttonObjects.clear();
    
    // Button positions (arranged vertically with compact spacing for small buttons)
    Vector3 buttonPositions[BUTTON_COUNT] = {
        Vector3(0.0f, 0.25f, 0.0f),    // PLAY (above center)
        Vector3(0.0f, 0.0f, 0.0f),     // HELP (center)
        Vector3(0.0f, -0.25f, 0.0f)    // CLOSE (below center)
    };
    
    // Create buttons with larger size
    for (int i = 0; i < BUTTON_COUNT; i++) {
        auto button = std::make_unique<Object>(200 + i);
        button->SetModel(0);    // Sprite2D model
        button->SetTexture(2 + i, 0); // btn_play.tga, btn_help.tga, btn_close.tga
        button->SetShader(0);
        button->SetPosition(buttonPositions[i]);
        button->SetScale(Vector3(0.2f, 0.1f, 1.0f)); // Even smaller buttons
        
        m_buttonObjects.push_back(std::move(button));
    }
    
    std::cout << "Created " << BUTTON_COUNT << " tiny menu buttons with blending enabled" << std::endl;
}

void GSMenu::Update(float deltaTime) {
    m_buttonTimer += deltaTime;
    
    // Simple button highlighting effect (scale animation)
    for (int i = 0; i < BUTTON_COUNT; i++) {
        if (i == m_selectedButton) {
            // Selected button pulses (even smaller size)
            float pulse = 1.0f + 0.2f * sin(m_buttonTimer * 4.0f);
            m_buttonObjects[i]->SetScale(Vector3(0.2f * pulse, 0.1f * pulse, 1.0f));
        } else {
            // Non-selected buttons stay normal even smaller size
            m_buttonObjects[i]->SetScale(Vector3(0.2f, 0.1f, 1.0f));
        }
    }
}

void GSMenu::Draw() {
    // Draw background first
    if (m_backgroundObject && m_camera2D) {
        const Matrix& viewMatrix = m_camera2D->GetViewMatrix();
        const Matrix& projMatrix = m_camera2D->GetProjectionMatrix();
        m_backgroundObject->Draw(const_cast<Matrix&>(viewMatrix), const_cast<Matrix&>(projMatrix));
    }
    
    // Draw buttons
    if (m_camera2D) {
        const Matrix& viewMatrix = m_camera2D->GetViewMatrix();
        const Matrix& projMatrix = m_camera2D->GetProjectionMatrix();
        
        for (auto& button : m_buttonObjects) {
            button->Draw(const_cast<Matrix&>(viewMatrix), const_cast<Matrix&>(projMatrix));
        }
    }
}

void GSMenu::HandleKeyEvent(unsigned char key, bool bIsPressed) {
    if (!bIsPressed) return;
    
    switch (key) {
        // Note: Could add case 72 (Up arrow) but keeping it simple with W/S keys only
        case 'W':
        case 'w':
            m_selectedButton--;
            if (m_selectedButton < 0) m_selectedButton = BUTTON_COUNT - 1;
            std::cout << "Selected button: " << m_selectedButton << std::endl;
            break;
            
        // Note: Removed case 80 (Down arrow) because it conflicts with 'P' (ASCII 80)
        case 'S':
        case 's':
            m_selectedButton++;
            if (m_selectedButton >= BUTTON_COUNT) m_selectedButton = 0;
            std::cout << "Selected button: " << m_selectedButton << std::endl;
            break;
            
        case 13: // Enter
        case ' ': // Space
            HandleButtonSelection();
            break;
            
        case 27: // ESC
            std::cout << "Exit game requested" << std::endl;
            // You can add exit logic here
            break;
            
        case 'P':
        case 'p':
            // Quick access to play
            std::cout << "Quick Play!" << std::endl;
            GameStateMachine::GetInstance()->PushState(StateType::PLAY);
            break;
    }
}

void GSMenu::HandleButtonSelection() {
    switch (m_selectedButton) {
        case BUTTON_PLAY:
            std::cout << "=== Starting Game ===" << std::endl;
            GameStateMachine::GetInstance()->PushState(StateType::PLAY);
            break;
            
        case BUTTON_HELP:
            std::cout << "=== HELP ===" << std::endl;
            std::cout << "Game Controls:" << std::endl;
            std::cout << "- Use W/S keys to navigate" << std::endl;
            std::cout << "- ENTER or SPACE to select" << std::endl;
            std::cout << "- ESC to go back" << std::endl;
            std::cout << "- P for quick play" << std::endl;
            break;
            
        case BUTTON_CLOSE:
            std::cout << "=== Closing Game ===" << std::endl;
            // In a real game, you would exit here
            std::cout << "Thanks for playing!" << std::endl;
            break;
    }
}

void GSMenu::Resume() {
    std::cout << "GSMenu: Resume (Back to Main Menu)" << std::endl;
    std::cout << "Welcome back to the main menu!" << std::endl;
}

void GSMenu::Pause() {
    std::cout << "GSMenu: Pause" << std::endl;
}

void GSMenu::Exit() {
    std::cout << "GSMenu: Exit" << std::endl;
}

void GSMenu::Cleanup() {
    std::cout << "GSMenu: Cleanup" << std::endl;
    m_backgroundObject.reset();
    m_buttonObjects.clear();
    m_camera2D.reset();
} 