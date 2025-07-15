#pragma once
#include "GameStateBase.h"
#include "../GameObject/Object.h"
#include "../GameObject/Camera.h"
#include <memory>

class GSIntro : public GameStateBase {
private:
    std::unique_ptr<Object> m_backgroundObject;
    std::unique_ptr<Camera> m_camera2D;
    float m_loadingTimer;
    float m_loadingDuration;
    
public:
    GSIntro();
    ~GSIntro();

    void Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void HandleKeyEvent(unsigned char key, bool bIsPressed) override;
    void Resume() override;
    void Pause() override;
    void Exit() override;
    void Cleanup() override;
}; 