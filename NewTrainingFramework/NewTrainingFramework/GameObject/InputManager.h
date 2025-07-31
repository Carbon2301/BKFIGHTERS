#pragma once
#include "Character.h"

class InputManager {
private:
    bool m_keyStates[512];
    bool m_keyPressed[512];
    static InputManager* s_instance;

public:
    InputManager();
    ~InputManager();
    
    static InputManager* GetInstance();
    static void DestroyInstance();
    
    void UpdateKeyState(unsigned char key, bool pressed);
    void UpdateKeyState(int key, bool pressed); // Overload for arrow keys
    bool IsKeyPressed(unsigned char key) const;
    bool IsKeyPressed(int key) const;
    bool IsKeyReleased(unsigned char key) const;
    bool IsKeyReleased(int key) const;
    
    void HandleInput(Character& character);
    void HandleInputPlayer2(Character& character);
    void Update();
    
    const bool* GetKeyStates() const { return m_keyStates; }
}; 