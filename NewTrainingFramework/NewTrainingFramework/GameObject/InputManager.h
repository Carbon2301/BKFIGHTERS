#pragma once
#include <vector>

class Character;

class InputManager {
private:
    static InputManager* s_instance;
    bool m_keyStates[512];
    bool m_keyPressed[512];

    InputManager();

public:
    static InputManager* GetInstance();
    static void DestroyInstance();
    ~InputManager();

    void UpdateKeyState(unsigned char key, bool pressed);
    void UpdateKeyState(int key, bool pressed);
    
    bool IsKeyPressed(unsigned char key) const;
    bool IsKeyPressed(int key) const;
    bool IsKeyJustPressed(unsigned char key) const;
    bool IsKeyJustPressed(int key) const;
    bool IsKeyReleased(unsigned char key) const;
    bool IsKeyReleased(int key) const;
    
    const bool* GetKeyStates() const;
    void Update();
}; 