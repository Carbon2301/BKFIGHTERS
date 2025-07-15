#pragma once
#include "GameStateBase.h"
#include "../GameObject/Object.h"
#include "../GameObject/Camera.h"
#include <memory>
#include <vector>

class GSPlay : public GameStateBase {
private:
    std::unique_ptr<Object> m_backgroundObject;
    std::unique_ptr<Object> m_menuButton;
    std::unique_ptr<Camera> m_camera2D;
    
    float m_gameTime;
    
public:
    GSPlay();
    ~GSPlay();

    void Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void HandleKeyEvent(unsigned char key, bool bIsPressed) override;
    void Resume() override;
    void Pause() override;
    void Exit() override;
    void Cleanup() override;
}; 