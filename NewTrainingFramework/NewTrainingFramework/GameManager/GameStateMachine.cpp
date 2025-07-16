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
    std::cout << "Stack size before push: " << m_stateStack.size() << std::endl;
    
    // Pause current state and push to stack
    if (m_pActiveState) {
        std::cout << "Pausing current state and pushing to stack..." << std::endl;
        m_pActiveState->Pause();
        m_stateStack.push(std::move(m_pActiveState));
        std::cout << "Stack size after push: " << m_stateStack.size() << std::endl;
    }
    
    // Create new state and make it active
    m_pActiveState = CreateState(stateType);
    if (m_pActiveState) {
        std::cout << "Initializing new state..." << std::endl;
        m_pActiveState->Init();
    }
}

void GameStateMachine::PopState() {
    std::cout << "Popping state" << std::endl;
    std::cout << "Stack size before pop: " << m_stateStack.size() << std::endl;
    
    if (m_pActiveState) {
        m_pActiveState->Exit();
        m_pActiveState->Cleanup();
        m_pActiveState.reset();
    }
    
    // Pop previous state from stack and resume
    if (!m_stateStack.empty()) {
        std::cout << "Found state in stack, restoring..." << std::endl;
        m_pActiveState = std::move(m_stateStack.top());
        m_stateStack.pop();
        if (m_pActiveState) {
            std::cout << "Resuming previous state..." << std::endl;
            m_pActiveState->Resume();
        }
    } else {
        std::cout << "ERROR: Stack is empty! No state to return to!" << std::endl;
    }
}

void GameStateMachine::PerformStateChange() {
    if (m_pNextState) {
        std::cout << "*** PerformStateChange: Changing state ***" << std::endl;
        std::cout << "Stack size before clearing: " << m_stateStack.size() << std::endl;
        
        // Exit current state
        if (m_pActiveState) {
            m_pActiveState->Exit();
            m_pActiveState->Cleanup();
        }
        
        // Clear stack when changing state completely
        while (!m_stateStack.empty()) {
            m_stateStack.pop();
        }
        std::cout << "Stack cleared! New size: " << m_stateStack.size() << std::endl;
        
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