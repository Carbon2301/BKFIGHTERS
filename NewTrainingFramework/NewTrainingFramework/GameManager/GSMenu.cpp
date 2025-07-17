#include "stdafx.h"
#include "GSMenu.h"
#include "GameStateMachine.h"
#include "../Core/Globals.h"
#include <iostream>

GSMenu::GSMenu() 
    : GameStateBase(StateType::MENU), m_buttonTimer(0.0f) {
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
    
    std::cout << "Use P for Play, Q for Question/Help, X for Exit" << std::endl;
    std::cout << "ESC to exit game" << std::endl;
}

void GSMenu::Update(float deltaTime) {
    m_buttonTimer += deltaTime;
    
    // Update scene
    SceneManager::GetInstance()->Update(deltaTime);
    
    // Button highlighting removed - using direct key mapping instead of navigation
}

void GSMenu::Draw() {
    // Use SceneManager to draw all objects
    SceneManager::GetInstance()->Draw();
}

void GSMenu::HandleKeyEvent(unsigned char key, bool bIsPressed) {
    if (!bIsPressed) return;
    
    switch (key) {
        case 'P':
        case 'p':
            // Direct play button
            std::cout << "=== Starting Game ===" << std::endl;
            GameStateMachine::GetInstance()->PushState(StateType::PLAY);
            break;
            
        case 'Q':
        case 'q':
            // Direct question/help button
            std::cout << "=== HELP ===" << std::endl;
            std::cout << "Game Controls:" << std::endl;
            std::cout << "- P: Play game" << std::endl;
            std::cout << "- Q: Help/Question" << std::endl;
            std::cout << "- X: Exit game" << std::endl;
            std::cout << "- ESC: Exit game" << std::endl;
            break;
            
        case 'X':
        case 'x':
            // Direct exit button
            std::cout << "=== Closing Game ===" << std::endl;
            std::cout << "Thanks for playing!" << std::endl;
            break;
            
        case 13: // Enter
        case ' ': // Space
            HandleButtonSelection();
            break;
            
        case 27: // ESC
            std::cout << "Exit game requested" << std::endl;
            // You can add exit logic here
            break;
    }
}

void GSMenu::HandleButtonSelection() {
    std::cout << "=== Available Actions ===" << std::endl;
    std::cout << "Game Controls:" << std::endl;
    std::cout << "- P: Play game" << std::endl;
    std::cout << "- Q: Help/Question" << std::endl;
    std::cout << "- X: Exit game" << std::endl;
    std::cout << "- ESC: Exit game" << std::endl;
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