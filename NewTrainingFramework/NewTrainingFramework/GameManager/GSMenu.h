#pragma once
#include "GameStateBase.h"
#include "SceneManager.h"

class GSMenu : public GameStateBase {
private:
    float m_buttonTimer;
    
    enum ButtonType {
        BUTTON_PLAY = 0,
        BUTTON_HELP = 1,
        BUTTON_CLOSE = 2,
        BUTTON_COUNT = 3
    };
    
    // Button object IDs in scene (must match scene config)
    static const int BUTTON_ID_PLAY = 201;
    static const int BUTTON_ID_HELP = 202;
    static const int BUTTON_ID_CLOSE = 203;
    
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
    void HandleButtonSelection();
}; 