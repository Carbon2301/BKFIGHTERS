#pragma once
#include "GameStateBase.h"
#include "SceneManager.h"
#include "../GameObject/Object.h"
#include "../GameObject/Texture2D.h"

struct _Mix_Music;
typedef struct _Mix_Music Mix_Music;

class GSMenu : public GameStateBase {
private:
    float m_buttonTimer;
    
    enum ButtonType {
        BUTTON_PLAY = 0,
        BUTTON_HELP = 1,
        BUTTON_CLOSE = 2,
        BUTTON_COUNT = 3
    };
    
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
    void HandleMouseEvent(int x, int y, bool bIsPressed) override;
    void HandleMouseMove(int x, int y) override;
    void Resume() override;
    void Pause() override;
    void Exit() override;
    void Cleanup() override;

private:
    void HandleButtonSelection();

    // --- Thêm biến quản lý text động ---
    std::shared_ptr<Object> m_textObject;
    std::shared_ptr<Texture2D> m_textTexture;
    Mix_Music* m_backgroundMusic;
}; 