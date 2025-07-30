#pragma once
#include "Character.h"

class InputManager {
private:
    bool m_keyStates[256];
    bool m_keyPressed[256]; // Track key press events
    static InputManager* s_instance;

public:
    InputManager();
    ~InputManager();
    
    static InputManager* GetInstance();
    static void DestroyInstance();
    
    // Input handling
    void UpdateKeyState(unsigned char key, bool pressed);
    bool IsKeyPressed(unsigned char key) const;
    bool IsKeyReleased(unsigned char key) const;
    
    // Handle input for character
    void HandleInput(Character& character);
    void Update(); // Reset key press events
    
    // Get key states for external use
    const bool* GetKeyStates() const { return m_keyStates; }
}; 