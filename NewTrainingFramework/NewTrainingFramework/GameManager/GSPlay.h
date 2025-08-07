#pragma once
#include "GameStateBase.h"
#include "SceneManager.h"
#include "../GameObject/AnimationManager.h"
#include "../GameObject/Character.h"
#include "../GameObject/InputManager.h"

class GSPlay : public GameStateBase {
private:
    float m_gameTime;
    
    static const int MENU_BUTTON_ID = 301;
    static const int ANIM_OBJECT_ID = 1000;
    std::shared_ptr<AnimationManager> m_animManager;
    
    // Static variable to control hitbox/hurtbox visibility
    static bool s_showHitboxHurtbox;
    
    // Static variable to control platform box visibility
    static bool s_showPlatformBoxes;
    
    // Static variable to control wall box visibility
    static bool s_showWallBoxes;
    
    // Health system variables
    float m_player1Health;
    float m_player2Health;
    const float MAX_HEALTH = 100.0f;
    const float HEALTH_DAMAGE = 10.0f; // Máu mất mỗi lần nhấn T
    
    // Cloud movement system
    float m_cloudSpeed;
    const float CLOUD_MOVE_SPEED = 0.2f; // Tốc độ di chuyển mây (units per second) - đã giảm từ 0.5f xuống 0.2f
    const float CLOUD_SPACING = 1.74f; // Khoảng cách giữa các mây
    const float CLOUD_LEFT_BOUNDARY = -6.0f; // Vị trí mây biến mất (bên trái màn hình)
    const int TOTAL_CLOUDS = 10; // Tổng số mây
    
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
    
    // Static getter for hitbox/hurtbox visibility
    static bool IsShowHitboxHurtbox() { return s_showHitboxHurtbox; }
    
    // Static getter for platform box visibility
    static bool IsShowPlatformBoxes() { return s_showPlatformBoxes; }
    
    // Static getter for wall box visibility
    static bool IsShowWallBoxes() { return s_showWallBoxes; }
    
    // Health system methods
    void UpdateHealthBars();
    float GetPlayer1Health() const { return m_player1Health; }
    float GetPlayer2Health() const { return m_player2Health; }
    
    // Cloud movement system
    void UpdateCloudMovement(float deltaTime);
}; 