#pragma once
#include "GameStateBase.h"
#include "SceneManager.h"

class GSPlay : public GameStateBase {
private:
    float m_gameTime;
    
    // Menu button object ID in scene (must match scene config)
    static const int MENU_BUTTON_ID = 301;
    static const int ANIM_OBJECT_ID = 1000; // Thêm ID cho object animation
    std::shared_ptr<class Animation2D> m_anim;
    
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