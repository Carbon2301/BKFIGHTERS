#include "stdafx.h"
#include "GSIntro.h"
#include "GameStateMachine.h"
#include "../Core/Globals.h"
#include <iostream>

GSIntro::GSIntro() 
    : GameStateBase(StateType::INTRO), m_loadingTimer(0.0f), m_loadingDuration(3.0f) {
}

GSIntro::~GSIntro() {
}

void GSIntro::Init() {
    std::cout << "=== LOADING SCREEN ===" << std::endl;
    std::cout << "Initializing resources..." << std::endl;
    
    // Enable blending for transparent textures
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Setup 2D camera for intro screen
    m_camera2D = std::make_unique<Camera>();
    float aspect = (float)Globals::screenWidth / (float)Globals::screenHeight;
    
    // Use orthographic projection for 2D
    m_camera2D->SetOrthographic(-aspect, aspect, -1.0f, 1.0f, 0.1f, 100.0f);
    m_camera2D->SetLookAt(Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
    
    // Create background object (will use Sprite2D model with a loading texture)
    m_backgroundObject = std::make_unique<Object>(0);
    m_backgroundObject->SetModel(0);    // Sprite2D model
    m_backgroundObject->SetTexture(0, 0); // Background texture
    m_backgroundObject->SetShader(0);   // Basic shader
    m_backgroundObject->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
    m_backgroundObject->SetScale(Vector3(2.0f, 2.0f, 1.0f)); // Scale to cover screen
    
    m_loadingTimer = 0.0f;
    std::cout << "Loading screen initialized" << std::endl;
}

void GSIntro::Update(float deltaTime) {
    m_loadingTimer += deltaTime;
    
    // Show loading progress
    static float lastProgressTime = 0.0f;
    if (m_loadingTimer - lastProgressTime > 0.5f) {
        float progress = (m_loadingTimer / m_loadingDuration) * 100.0f;
        if (progress > 100.0f) progress = 100.0f;
        std::cout << "Loading... " << (int)progress << "%" << std::endl;
        lastProgressTime = m_loadingTimer;
    }
    
    // Auto-transition to menu after loading is complete
    if (m_loadingTimer >= m_loadingDuration) {
        std::cout << "Loading complete! Transitioning to main menu..." << std::endl;
        GameStateMachine::GetInstance()->ChangeState(StateType::MENU);
    }
}

void GSIntro::Draw() {
    if (m_backgroundObject && m_camera2D) {
        const Matrix& viewMatrix = m_camera2D->GetViewMatrix();
        const Matrix& projMatrix = m_camera2D->GetProjectionMatrix();
        m_backgroundObject->Draw(const_cast<Matrix&>(viewMatrix), const_cast<Matrix&>(projMatrix));
    }
}

void GSIntro::HandleKeyEvent(unsigned char key, bool bIsPressed) {
    if (bIsPressed) {
        // Allow skipping loading screen with any key
        if (key != 0) {
            std::cout << "Loading skipped by user input" << std::endl;
            GameStateMachine::GetInstance()->ChangeState(StateType::MENU);
        }
    }
}

void GSIntro::Resume() {
    std::cout << "GSIntro: Resume" << std::endl;
}

void GSIntro::Pause() {
    std::cout << "GSIntro: Pause" << std::endl;
}

void GSIntro::Exit() {
    std::cout << "GSIntro: Exit" << std::endl;
}

void GSIntro::Cleanup() {
    std::cout << "GSIntro: Cleanup" << std::endl;
    m_backgroundObject.reset();
    m_camera2D.reset();
} 