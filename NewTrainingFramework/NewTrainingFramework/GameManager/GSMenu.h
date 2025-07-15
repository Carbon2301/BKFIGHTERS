#pragma once
#include "GameStateBase.h"
#include "../GameObject/Object.h"
#include "../GameObject/Camera.h"
#include <memory>
#include <vector>

class GSMenu : public GameStateBase {
private:
    std::unique_ptr<Object> m_backgroundObject;
    std::vector<std::unique_ptr<Object>> m_buttonObjects;
    std::unique_ptr<Camera> m_camera2D;
    
    int m_selectedButton;
    float m_buttonTimer;
    
    enum ButtonType {
        BUTTON_PLAY = 0,
        BUTTON_HELP = 1,
        BUTTON_CLOSE = 2,
        BUTTON_COUNT = 3
    };
    
public:
    GSMenu();
    ~GSMenu();

    void Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void HandleKeyEvent(unsigned char key, bool bIsPressed) override;
    void Resume() override;
    void Pause() override;
    void Exit() override;
    void Cleanup() override;

private:
    void CreateButtons();
    void HandleButtonSelection();
}; 