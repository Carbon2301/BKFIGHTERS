#include "stdafx.h"
#include "InputManager.h"
#include <iostream>

InputManager* InputManager::s_instance = nullptr;

InputManager* InputManager::GetInstance() {
    if (!s_instance) {
        s_instance = new InputManager();
    }
    return s_instance;
}

void InputManager::DestroyInstance() {
    if (s_instance) {
        delete s_instance;
        s_instance = nullptr;
    }
}

InputManager::InputManager() {
    // Initialize all key states to false
    for (int i = 0; i < 256; i++) {
        m_keyStates[i] = false;
        m_keyPressed[i] = false;
    }
}

InputManager::~InputManager() {
}

void InputManager::UpdateKeyState(unsigned char key, bool pressed) {
    // Track key press events (when key goes from false to true)
    if (pressed && !m_keyStates[key]) {
        m_keyPressed[key] = true;
    }
    m_keyStates[key] = pressed;
}

bool InputManager::IsKeyPressed(unsigned char key) const {
    return m_keyStates[key];
}

bool InputManager::IsKeyReleased(unsigned char key) const {
    return !m_keyStates[key];
}

void InputManager::HandleInput(Character& character) {
    // Handle combat input (only on key press, not continuous)
    if (m_keyPressed['J'] || m_keyPressed['j']) {
        character.HandlePunchCombo();
    }
    
    if (m_keyPressed['L'] || m_keyPressed['l']) {
        character.HandleAxeCombo();
    }
    
    if (m_keyPressed['K'] || m_keyPressed['k']) {
        character.HandleKick();
    }
}

void InputManager::Update() {
    // Reset key press events
    for (int i = 0; i < 256; i++) {
        m_keyPressed[i] = false;
    }
} 