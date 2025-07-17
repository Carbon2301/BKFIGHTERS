#pragma once
#include "GameStateBase.h"
#include "SceneManager.h"

class GSPlay : public GameStateBase {
private:
    float m_gameTime;
    
    // Menu button object ID in scene (must match scene config)
    static const int MENU_BUTTON_ID = 301;
    
public:
    GSPlay();
    ~GSPlay();

    void Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void HandleKeyEvent(unsigned char key, bool bIsPressed) override;
    void HandleMouseEvent(int x, int y, bool bIsPressed) override;
    void HandleMouseMove(int x, int y) override;
    void Resume() override;
    void Pause() override;
    void Exit() override;
    void Cleanup() override;
}; 