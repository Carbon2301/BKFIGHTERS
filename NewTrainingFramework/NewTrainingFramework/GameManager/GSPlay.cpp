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
    
    // Setup 2D camera for gameplay
    m_camera2D = std::make_unique<Camera>();
    float aspect = (float)Globals::screenWidth / (float)Globals::screenHeight;
    
    // Use orthographic projection for 2D
    m_camera2D->SetOrthographic(-aspect, aspect, -1.0f, 1.0f, 0.1f, 100.0f);
    m_camera2D->SetLookAt(Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
    
    // Create gameplay background
    m_backgroundObject = std::make_unique<Object>(300);
    m_backgroundObject->SetModel(0);    // Sprite2D model
    m_backgroundObject->SetTexture(1, 0); // Background texture (bg_play1.tga)
    m_backgroundObject->SetShader(0);   // Basic shader
    m_backgroundObject->SetPosition(Vector3(0.0f, 0.0f, -0.1f));
    m_backgroundObject->SetScale(Vector3(2.5f, 2.5f, 1.0f)); // Scale to cover screen
    
    // Create menu button (top-right corner) - use Close button texture as menu
    m_menuButton = std::make_unique<Object>(301);
    m_menuButton->SetModel(0);    // Sprite2D model
    m_menuButton->SetTexture(4, 0); // btn_close.tga (reused as menu button)
    m_menuButton->SetShader(0);
    m_menuButton->SetPosition(Vector3(1.2f, 0.8f, 0.0f)); // Top-right
    m_menuButton->SetScale(Vector3(0.2f, 0.1f, 1.0f)); // Same size as menu buttons
    
    m_gameTime = 0.0f;
    
    std::cout << "Gameplay initialized" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "- ESC or M: Return to menu" << std::endl;
    std::cout << "- WASD: Move (demo)" << std::endl;
}

void GSPlay::Update(float deltaTime) {
    m_gameTime += deltaTime;
    
    // Simple demo: animate menu button (optional)
    if (m_menuButton) {
        float pulse = 1.0f + 0.05f * sin(m_gameTime * 3.0f);
        m_menuButton->SetScale(Vector3(0.2f * pulse, 0.1f * pulse, 1.0f)); // Smaller animation
    }
    
    // Show gameplay time every 5 seconds
    static float lastTimeShow = 0.0f;
    if (m_gameTime - lastTimeShow > 5.0f) {
        std::cout << "Game time: " << (int)m_gameTime << " seconds" << std::endl;
        lastTimeShow = m_gameTime;
    }
}

void GSPlay::Draw() {
    // Draw background
    if (m_backgroundObject && m_camera2D) {
        const Matrix& viewMatrix = m_camera2D->GetViewMatrix();
        const Matrix& projMatrix = m_camera2D->GetProjectionMatrix();
        m_backgroundObject->Draw(const_cast<Matrix&>(viewMatrix), const_cast<Matrix&>(projMatrix));
    }
    
    // Draw menu button
    if (m_menuButton && m_camera2D) {
        const Matrix& viewMatrix = m_camera2D->GetViewMatrix();
        const Matrix& projMatrix = m_camera2D->GetProjectionMatrix();
        m_menuButton->Draw(const_cast<Matrix&>(viewMatrix), const_cast<Matrix&>(projMatrix));
    }
}

void GSPlay::HandleKeyEvent(unsigned char key, bool bIsPressed) {
    if (!bIsPressed) return;
    
    switch (key) {
        case 27: // ESC
        case 'M':
        case 'm':
            std::cout << "=== Returning to Menu ===" << std::endl;
            std::cout << "Game paused. Returning to main menu..." << std::endl;
            GameStateMachine::GetInstance()->PopState(); // Pop back to menu
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
    m_backgroundObject.reset();
    m_menuButton.reset();
    m_camera2D.reset();
} 