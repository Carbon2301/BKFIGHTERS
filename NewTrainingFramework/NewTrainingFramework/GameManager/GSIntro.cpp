#include "stdafx.h"
#include "GSIntro.h"
#include "GameStateMachine.h"
#include "../Core/Globals.h"
#include "SceneManager.h"
#include "../GameObject/Object.h"
#include <iostream>
#include "SoundManager.h"
#include <algorithm>

GSIntro::GSIntro() 
    : GameStateBase(StateType::INTRO), m_loadingTimer(0.0f), m_loadingDuration(3.0f) {
}

GSIntro::~GSIntro() {
}

void GSIntro::Init() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    SceneManager* sceneManager = SceneManager::GetInstance();
    if (!sceneManager->LoadSceneForState(StateType::INTRO)) {
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

    float aspect = (float)Globals::screenWidth / (float)Globals::screenHeight;
    m_barWidth = aspect * 1.2f;
    m_barHeight = 0.08f;
    m_barLeftX = -m_barWidth * 0.5f;
    m_barY = -0.8f;

    // Background bar (dark/black)
    m_barBg = sceneManager->CreateObject(901);
    if (m_barBg) {
        m_barBg->SetModel(0);
        m_barBg->SetTexture(21, 0);
        m_barBg->SetShader(0);
        m_barBg->SetPosition(0.0f, m_barY, 0.0f);
        m_barBg->SetScale(m_barWidth, m_barHeight, 1.0f);
    }

    m_barFill = sceneManager->CreateObject(902);
    if (m_barFill) {
        m_barFill->SetModel(0);
        m_barFill->SetTexture(13, 0);
        m_barFill->SetShader(0);
        m_barFill->SetPosition(m_barLeftX, m_barY, 0.0f);
        m_barFill->SetScale(0.001f, m_barHeight - 0.02f, 1.0f); // tiny width to avoid zero scale
    }

    m_loadingTimer = 0.0f;

    m_tasks.clear();
    auto allIds = SoundManager::Instance().GetAllAudioIDs();
    for (int id : allIds) {
        bool isMusic = (id == 0 || id == 22); // 22 is backgroundNoise, 0 is menu
        m_tasks.push_back({id, isMusic});
    }
    m_taskIndex = 0;
}

void GSIntro::Update(float deltaTime) {
    m_loadingTimer += deltaTime;
    
    SceneManager::GetInstance()->Update(deltaTime);
    
    if (m_taskIndex < m_tasks.size()) {
        const LoadTask& task = m_tasks[m_taskIndex];
        if (task.isMusic) SoundManager::Instance().PreloadMusicByID(task.id);
        else SoundManager::Instance().PreloadSFXByID(task.id);
        m_taskIndex++;
    }

    float totalPlanned = (std::max)(m_loadingDuration, m_minShowTime);
    float progressByTime = (std::min)(1.0f, m_loadingTimer / totalPlanned);
    float progressByTasks = m_tasks.empty() ? 1.0f : (float)m_taskIndex / (float)m_tasks.size();
    float t = (std::min)(1.0f, 0.5f * progressByTime + 0.5f * progressByTasks);
    if (t > 1.0f) t = 1.0f;
    float currentWidth = m_barWidth * t;
    if (m_barFill) {
        float centerX = m_barLeftX + currentWidth * 0.5f;
        m_barFill->SetPosition(centerX, m_barY, 0.0f);
        m_barFill->SetScale(currentWidth, m_barHeight - 0.02f, 1.0f);
    }
    
    if (t >= 1.0f && m_loadingTimer >= m_minShowTime) {
        GameStateMachine::GetInstance()->ChangeState(StateType::MENU);
    }
}

void GSIntro::Draw() {
    SceneManager::GetInstance()->Draw();
}

void GSIntro::HandleKeyEvent(unsigned char key, bool bIsPressed) {
    if (!bIsPressed) return;
    if (m_loadingTimer >= m_loadingDuration) {
        GameStateMachine::GetInstance()->ChangeState(StateType::MENU);
    }
}

void GSIntro::HandleMouseEvent(int x, int y, bool bIsPressed) {
    if (!bIsPressed) return;
    if (m_loadingTimer >= m_loadingDuration) {
        GameStateMachine::GetInstance()->ChangeState(StateType::MENU);
    }
}

void GSIntro::HandleMouseMove(int x, int y) {
}

void GSIntro::Resume() {
}

void GSIntro::Pause() {
}

void GSIntro::Exit() {
}

void GSIntro::Cleanup() {
} 