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
    
    // Load scene using SceneManager
    SceneManager* sceneManager = SceneManager::GetInstance();
    if (!sceneManager->LoadSceneForState(StateType::INTRO)) {
        std::cout << "Failed to load intro scene!" << std::endl;
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
        
        // Create default background object
        Object* backgroundObj = sceneManager->CreateObject(0);
        if (backgroundObj) {
            backgroundObj->SetModel(0);    // Sprite2D model
            backgroundObj->SetTexture(0, 0); // Background texture
            backgroundObj->SetShader(0);   // Basic shader
            backgroundObj->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
            backgroundObj->SetScale(Vector3(2.0f, 2.0f, 1.0f));
        }
    }
    
    m_loadingTimer = 0.0f;
    std::cout << "Loading screen initialized" << std::endl;
}

void GSIntro::Update(float deltaTime) {
    m_loadingTimer += deltaTime;
    
    // Update scene
    SceneManager::GetInstance()->Update(deltaTime);
    
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
    // Use SceneManager to draw all objects
    SceneManager::GetInstance()->Draw();
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
    // SceneManager will be cleaned up by the next state or when destroyed
} 