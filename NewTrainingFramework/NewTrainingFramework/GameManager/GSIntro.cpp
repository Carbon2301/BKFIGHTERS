#include "stdafx.h"
#include "GSIntro.h"
#include "GameStateMachine.h"
#include "../Core/Globals.h"
#include <iostream>

GSIntro::GSIntro() 
    : GameStateBase(StateType::INTRO), m_loadingTimer(0.0f), m_loadingDuration(1.0f) {
}

GSIntro::~GSIntro() {
}

void GSIntro::Init() {
    std::cout << "=== LOADING SCREEN ===" << std::endl;
    std::cout << "Initializing resources..." << std::endl;
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    SceneManager* sceneManager = SceneManager::GetInstance();
    if (!sceneManager->LoadSceneForState(StateType::INTRO)) {
        std::cout << "Failed to load intro scene!" << std::endl;
        sceneManager->Clear();
        
        Camera* camera = sceneManager->CreateCamera();
        if (camera) {
            float aspect = (float)Globals::screenWidth / (float)Globals::screenHeight;
            camera->SetOrthographic(-aspect, aspect, -1.0f, 1.0f, 0.1f, 100.0f);
            camera->SetLookAt(Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
            sceneManager->SetActiveCamera(0);
        }
        
        Object* backgroundObj = sceneManager->CreateObject(0);
        if (backgroundObj) {
            backgroundObj->SetModel(0);
            backgroundObj->SetTexture(0, 0);
            backgroundObj->SetShader(0);
            backgroundObj->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
            backgroundObj->SetScale(Vector3(2.0f, 2.0f, 1.0f));
        }
    }
    
    m_loadingTimer = 0.0f;
    std::cout << "Loading screen initialized" << std::endl;
}

void GSIntro::Update(float deltaTime) {
    m_loadingTimer += deltaTime;
    
    SceneManager::GetInstance()->Update(deltaTime);
    
    static float lastProgressTime = 0.0f;
    if (m_loadingTimer - lastProgressTime > 0.5f) {
        float progress = (m_loadingTimer / m_loadingDuration) * 100.0f;
        if (progress > 100.0f) progress = 100.0f;
        std::cout << "Loading... " << (int)progress << "%" << std::endl;
        lastProgressTime = m_loadingTimer;
    }
    
    if (m_loadingTimer >= m_loadingDuration) {
        std::cout << "Loading complete! Transitioning to main menu..." << std::endl;
        GameStateMachine::GetInstance()->ChangeState(StateType::MENU);
    }
}

void GSIntro::Draw() {
    SceneManager::GetInstance()->Draw();
}

void GSIntro::HandleKeyEvent(unsigned char key, bool bIsPressed) {
    if (bIsPressed) {
        if (key != 0) {
            std::cout << "Loading skipped by user input" << std::endl;
            GameStateMachine::GetInstance()->ChangeState(StateType::MENU);
        }
    }
}

void GSIntro::HandleMouseEvent(int x, int y, bool bIsPressed) {
    if (bIsPressed) {
        std::cout << "Loading skipped by mouse click at (" << x << ", " << y << ")" << std::endl;
        GameStateMachine::GetInstance()->ChangeState(StateType::MENU);
    }
}

void GSIntro::HandleMouseMove(int x, int y) {
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
} 