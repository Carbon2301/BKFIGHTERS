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
    
    // Enable blending for transparent textures
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Load scene using SceneManager
    SceneManager* sceneManager = SceneManager::GetInstance();
    if (!sceneManager->LoadSceneForState(StateType::MENU)) {
        std::cout << "Failed to load menu scene!" << std::endl;
        // Fallback: create default scene manually if config file fails
        sceneManager->Clear();
        
        // Create default 2D camera
        Camera* camera = sceneManager->CreateCamera();
        if (camera) {
            float aspect = (float)Globals::screenWidth / (float)Globals::screenHeight;
            camera->SetOrthographic(-aspect, aspect, -1.0f, 1.0f, 0.1f, 100.0f);
            camera->SetLookAt(Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
            sceneManager->SetActiveCamera(0);
        }
        
        // Create default objects manually as fallback
        Object* backgroundObj = sceneManager->CreateObject(200);
        if (backgroundObj) {
            backgroundObj->SetModel(0);
            backgroundObj->SetTexture(0, 0);
            backgroundObj->SetShader(0);
            backgroundObj->SetPosition(Vector3(0.0f, 0.0f, -0.1f));
            backgroundObj->SetScale(Vector3(2.5f, 2.5f, 1.0f));
        }
        
        // Create button objects
        Vector3 buttonPositions[BUTTON_COUNT] = {
            Vector3(0.0f, 0.5f, 0.0f),    // PLAY
            Vector3(0.0f, 0.0f, 0.0f),    // HELP  
            Vector3(0.0f, -0.5f, 0.0f)    // CLOSE
        };
        
        int buttonIds[BUTTON_COUNT] = { BUTTON_ID_PLAY, BUTTON_ID_HELP, BUTTON_ID_CLOSE };
        
        for (int i = 0; i < BUTTON_COUNT; i++) {
            Object* button = sceneManager->CreateObject(buttonIds[i]);
            if (button) {
                button->SetModel(0);
                button->SetTexture(2 + i, 0); // btn_play, btn_help, btn_close
                button->SetShader(0);
                button->SetPosition(buttonPositions[i]);
                button->SetScale(Vector3(0.4f, 0.2f, 1.0f));
            }
        }
    }
    
    std::cout << "Use W/S keys to navigate, ENTER to select" << std::endl;
    std::cout << "ESC to exit game" << std::endl;
}

Object* GSMenu::GetButtonObject(int buttonIndex) {
    SceneManager* sceneManager = SceneManager::GetInstance();
    
    switch (buttonIndex) {
        case BUTTON_PLAY:  return sceneManager->GetObject(BUTTON_ID_PLAY);
        case BUTTON_HELP:  return sceneManager->GetObject(BUTTON_ID_HELP);
        case BUTTON_CLOSE: return sceneManager->GetObject(BUTTON_ID_CLOSE);
        default: return nullptr;
    }
}

void GSMenu::Update(float deltaTime) {
    m_buttonTimer += deltaTime;
    
    // Update scene
    SceneManager::GetInstance()->Update(deltaTime);
    
    // Simple button highlighting effect (scale animation)
    for (int i = 0; i < BUTTON_COUNT; i++) {
        Object* button = GetButtonObject(i);
        if (button) {
            if (i == m_selectedButton) {
                // Selected button pulses
                float pulse = 1.0f + 0.2f * sin(m_buttonTimer * 4.0f);
                button->SetScale(Vector3(0.4f * pulse, 0.2f * pulse, 1.0f));
            } else {
                // Non-selected buttons stay normal size
                button->SetScale(Vector3(0.4f, 0.2f, 1.0f));
            }
        }
    }
}

void GSMenu::Draw() {
    // Use SceneManager to draw all objects
    SceneManager::GetInstance()->Draw();
}

void GSMenu::HandleKeyEvent(unsigned char key, bool bIsPressed) {
    if (!bIsPressed) return;
    
    switch (key) {
        case 'W':
        case 'w':
            m_selectedButton--;
            if (m_selectedButton < 0) m_selectedButton = BUTTON_COUNT - 1;
            std::cout << "Selected button: " << m_selectedButton << std::endl;
            break;
            
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
    // SceneManager will be cleaned up by the next state or when destroyed
} 