#pragma once
#include "GameStateBase.h"
#include "SceneManager.h"

class GSIntro : public GameStateBase {
private:
    float m_loadingTimer;
    float m_loadingDuration;
    
public:
    GSIntro();
    ~GSIntro();

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