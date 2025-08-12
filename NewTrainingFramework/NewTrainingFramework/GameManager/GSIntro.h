#pragma once
#include "GameStateBase.h"
#include "SceneManager.h"
#include <vector>
class Object;

class GSIntro : public GameStateBase {
private:
    float m_loadingTimer;
    float m_loadingDuration;
    Object* m_barBg = nullptr;
    Object* m_barFill = nullptr;
    float m_barWidth = 0.0f;
    float m_barHeight = 0.0f;
    float m_barLeftX = 0.0f;
    float m_barY = 0.0f;
    struct LoadTask { int id; bool isMusic; };
    std::vector<LoadTask> m_tasks;
    size_t m_taskIndex = 0;
    float m_minShowTime = 1.0f;
    
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