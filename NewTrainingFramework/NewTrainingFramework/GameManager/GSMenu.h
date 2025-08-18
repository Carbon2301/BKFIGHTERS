#pragma once
#include "GameStateBase.h"
#include "SceneManager.h"
#include "../GameObject/Object.h"
#include "../GameObject/Texture2D.h"

class GSMenu : public GameStateBase {
private:
    float m_buttonTimer;
    
    enum ButtonType {
        BUTTON_PLAY = 0,
        BUTTON_EXIT = 1,
        BUTTON_COUNT = 2
    };
    
    static const int BUTTON_ID_PLAY = 201;
    static const int BUTTON_ID_EXIT = 205;
    
public:
    GSMenu();
    ~GSMenu();

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

private:
    // --- Thêm biến quản lý text động ---
    std::shared_ptr<Object> m_textObject;
    std::shared_ptr<Texture2D> m_textTexture;
}; 