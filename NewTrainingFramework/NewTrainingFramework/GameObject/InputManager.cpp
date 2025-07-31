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
    for (int i = 0; i < 512; i++) {
        m_keyStates[i] = false;
        m_keyPressed[i] = false;
    }
}

InputManager::~InputManager() {
}

void InputManager::UpdateKeyState(unsigned char key, bool pressed) {
    if (pressed && !m_keyStates[key]) {
        m_keyPressed[key] = true;
    }
    m_keyStates[key] = pressed;
}

void InputManager::UpdateKeyState(int key, bool pressed) {
    if (key >= 0 && key < 512) {
        if (pressed && !m_keyStates[key]) {
            m_keyPressed[key] = true;
        }
        m_keyStates[key] = pressed;
    }
}

bool InputManager::IsKeyPressed(unsigned char key) const {
    return m_keyStates[key];
}

bool InputManager::IsKeyPressed(int key) const {
    if (key >= 0 && key < 512) {
        return m_keyStates[key];
    }
    return false;
}

bool InputManager::IsKeyJustPressed(unsigned char key) const {
    return m_keyPressed[key];
}

bool InputManager::IsKeyJustPressed(int key) const {
    if (key >= 0 && key < 512) {
        return m_keyPressed[key];
    }
    return false;
}

bool InputManager::IsKeyReleased(unsigned char key) const {
    return !m_keyStates[key];
}

bool InputManager::IsKeyReleased(int key) const {
    if (key >= 0 && key < 512) {
        return !m_keyStates[key];
    }
    return true;
}

const bool* InputManager::GetKeyStates() const {
    return m_keyStates;
}

void InputManager::Update() {
    for (int i = 0; i < 512; i++) {
        m_keyPressed[i] = false;
    }
} 