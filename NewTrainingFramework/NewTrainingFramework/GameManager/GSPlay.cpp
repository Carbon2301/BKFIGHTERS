#include "stdafx.h"
#include "GSPlay.h"
#include "GameStateMachine.h"
#include "../Core/Globals.h"
#include <iostream>

GSPlay::GSPlay() 
    : GameStateBase(StateType::PLAY), m_gameTime(0.0f) {
}

GSPlay::~GSPlay() {
}

void GSPlay::Init() {
    std::cout << "=== GAMEPLAY MODE ===" << std::endl;
    std::cout << "Game started! Press ESC or M to return to menu" << std::endl;
    
    // Enable blending for transparent textures
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Load scene using SceneManager
    SceneManager* sceneManager = SceneManager::GetInstance();
    if (!sceneManager->LoadSceneForState(StateType::PLAY)) {
        std::cout << "Failed to load play scene!" << std::endl;
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
        
        // Create default gameplay background
        Object* backgroundObj = sceneManager->CreateObject(300);
        if (backgroundObj) {
            backgroundObj->SetModel(0);
            backgroundObj->SetTexture(1, 0); // bg_play1.tga
            backgroundObj->SetShader(0);
            backgroundObj->SetPosition(Vector3(0.0f, 0.0f, -0.1f));
            backgroundObj->SetScale(Vector3(2.5f, 2.5f, 1.0f));
        }
        
        // Create menu button
        Object* menuButton = sceneManager->CreateObject(MENU_BUTTON_ID);
        if (menuButton) {
            menuButton->SetModel(0);
            menuButton->SetTexture(4, 0); // btn_close.tga
            menuButton->SetShader(0);
            menuButton->SetPosition(Vector3(1.2f, 0.8f, 0.0f));
            menuButton->SetScale(Vector3(0.2f, 0.1f, 1.0f));
        }
    }
    
    m_gameTime = 0.0f;
    
    std::cout << "Gameplay initialized" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "- ESC or M: Return to menu" << std::endl;
    std::cout << "- WASD: Move (demo)" << std::endl;
}

void GSPlay::Update(float deltaTime) {
    m_gameTime += deltaTime;
    
    // Update scene
    SceneManager::GetInstance()->Update(deltaTime);
    
    // Simple demo: animate menu button
    Object* menuButton = SceneManager::GetInstance()->GetObject(MENU_BUTTON_ID);
    if (menuButton) {
        float pulse = 1.0f + 0.05f * sin(m_gameTime * 3.0f);
        menuButton->SetScale(Vector3(0.2f * pulse, 0.1f * pulse, 1.0f));
    }
    
    // Show gameplay time every 5 seconds
    static float lastTimeShow = 0.0f;
    if (m_gameTime - lastTimeShow > 5.0f) {
        std::cout << "Game time: " << (int)m_gameTime << " seconds" << std::endl;
        lastTimeShow = m_gameTime;
    }
}

void GSPlay::Draw() {
    // Use SceneManager to draw all objects
    SceneManager::GetInstance()->Draw();
}

void GSPlay::HandleKeyEvent(unsigned char key, bool bIsPressed) {
    std::cout << "GSPlay: Key " << (int)key << " pressed=" << bIsPressed << std::endl;
    
    if (!bIsPressed) return;
    
    switch (key) {
        case 27: // ESC
        case 'M':
        case 'm':
            std::cout << "=== Returning to Menu ===" << std::endl;
            std::cout << "Game paused. Returning to main menu..." << std::endl;
            std::cout << "Calling ChangeState(MENU)..." << std::endl;
            GameStateMachine::GetInstance()->ChangeState(StateType::MENU); // Direct change to menu
            std::cout << "ChangeState() called successfully!" << std::endl;
            break;
            
        // Demo movement controls
        case 'W':
        case 'w':
            std::cout << "Move up (demo)" << std::endl;
            break;
        case 'S':
        case 's':
            std::cout << "Move down (demo)" << std::endl;
            break;
        case 'A':
        case 'a':
            std::cout << "Move left (demo)" << std::endl;
            break;
        case 'D':
        case 'd':
            std::cout << "Move right (demo)" << std::endl;
            break;
            
        case ' ': // Space
            std::cout << "Action button pressed! (demo)" << std::endl;
            break;
    }
}

void GSPlay::Resume() {
    std::cout << "GSPlay: Resume (Back to Game)" << std::endl;
    std::cout << "Welcome back to the game!" << std::endl;
}

void GSPlay::Pause() {
    std::cout << "GSPlay: Pause (Game Paused)" << std::endl;
    std::cout << "Game is now paused..." << std::endl;
}

void GSPlay::Exit() {
    std::cout << "GSPlay: Exit" << std::endl;
    std::cout << "Leaving gameplay mode..." << std::endl;
}

void GSPlay::Cleanup() {
    std::cout << "GSPlay: Cleanup" << std::endl;
    std::cout << "Final game time: " << (int)m_gameTime << " seconds" << std::endl;
    // SceneManager will be cleaned up by the next state or when destroyed
} 