#include "stdafx.h"
#include "GameStateMachine.h"
#include "GSIntro.h"
#include "GSMenu.h"
#include "GSPlay.h"
#include <iostream>

GameStateMachine* GameStateMachine::s_instance = nullptr;

GameStateMachine* GameStateMachine::GetInstance() {
    if (!s_instance) {
        s_instance = new GameStateMachine();
    }
    return s_instance;
}

void GameStateMachine::DestroyInstance() {
    if (s_instance) {
        delete s_instance;
        s_instance = nullptr;
    }
}

GameStateMachine::GameStateMachine() {
}

GameStateMachine::~GameStateMachine() {
    Cleanup();
}

void GameStateMachine::ChangeState(StateType stateType) {
    m_pNextState = CreateState(stateType);
    if (m_pNextState) {
        std::cout << "Scheduling state change to state: " << (int)stateType << std::endl;
    }
}

void GameStateMachine::PushState(StateType stateType) {
    std::cout << "Pushing state: " << (int)stateType << std::endl;
    
    // Pause current state and push to stack
    if (m_pActiveState) {
        m_pActiveState->Pause();
        m_stateStack.push(std::move(m_pActiveState));
    }
    
    // Create new state and make it active
    m_pActiveState = CreateState(stateType);
    if (m_pActiveState) {
        m_pActiveState->Init();
    }
}

void GameStateMachine::PopState() {
    std::cout << "Popping state" << std::endl;
    
    if (m_pActiveState) {
        m_pActiveState->Exit();
        m_pActiveState->Cleanup();
        m_pActiveState.reset();
    }
    
    // Pop previous state from stack and resume
    if (!m_stateStack.empty()) {
        m_pActiveState = std::move(m_stateStack.top());
        m_stateStack.pop();
        if (m_pActiveState) {
            m_pActiveState->Resume();
        }
    }
}

void GameStateMachine::PerformStateChange() {
    if (m_pNextState) {
        // Exit current state
        if (m_pActiveState) {
            m_pActiveState->Exit();
            m_pActiveState->Cleanup();
        }
        
        // Clear stack when changing state completely
        while (!m_stateStack.empty()) {
            m_stateStack.pop();
        }
        
        // Set new state as active
        m_pActiveState = std::move(m_pNextState);
        m_pNextState.reset();
        
        if (m_pActiveState) {
            m_pActiveState->Init();
        }
    }
}

void GameStateMachine::Update(float deltaTime) {
    PerformStateChange();
    
    if (m_pActiveState) {
        m_pActiveState->Update(deltaTime);
    }
}

void GameStateMachine::Draw() {
    if (m_pActiveState) {
        m_pActiveState->Draw();
    }
}

void GameStateMachine::HandleKeyEvent(unsigned char key, bool bIsPressed) {
    if (m_pActiveState) {
        m_pActiveState->HandleKeyEvent(key, bIsPressed);
    }
}

GameStateBase* GameStateMachine::CurrentState() {
    return m_pActiveState.get();
}

bool GameStateMachine::HasState() {
    return m_pActiveState != nullptr;
}

void GameStateMachine::Cleanup() {
    if (m_pActiveState) {
        m_pActiveState->Exit();
        m_pActiveState->Cleanup();
        m_pActiveState.reset();
    }
    
    while (!m_stateStack.empty()) {
        auto state = std::move(m_stateStack.top());
        m_stateStack.pop();
        if (state) {
            state->Exit();
            state->Cleanup();
        }
    }
    
    if (m_pNextState) {
        m_pNextState.reset();
    }
}

std::unique_ptr<GameStateBase> GameStateMachine::CreateState(StateType stateType) {
    switch (stateType) {
        case StateType::INTRO:
            return std::make_unique<GSIntro>();
        case StateType::MENU:
            return std::make_unique<GSMenu>();
        case StateType::PLAY:
            return std::make_unique<GSPlay>();
        default:
            std::cout << "Unknown state type: " << (int)stateType << std::endl;
            return nullptr;
    }
} 