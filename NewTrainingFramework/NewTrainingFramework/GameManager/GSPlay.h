#pragma once
#include "GameStateBase.h"
#include "SceneManager.h"
#include "../GameObject/AnimationManager.h"
#include "../GameObject/Character.h"
#include "../GameObject/InputManager.h"
#include "../GameObject/WallCollision.h"
#include "../../Utilities/Math.h"
#include <vector>

class GSPlay : public GameStateBase {
private:
    float m_gameTime;
    
    static const int MENU_BUTTON_ID = 301;
    static const int ANIM_OBJECT_ID = 1000;
    std::shared_ptr<AnimationManager> m_animManager;
    
    static bool s_showHitboxHurtbox;
    static bool s_showPlatformBoxes;
    static bool s_showWallBoxes;
    static bool s_showLadderBoxes;
    static bool s_showTeleportBoxes;
    
    // Health system variables
    float m_player1Health;
    float m_player2Health;
    const float MAX_HEALTH = 100.0f;
    const float HEALTH_DAMAGE = 10.0f; // Máu mất mỗi lần nhấn T
    
    // Cloud movement system
    float m_cloudSpeed;
    struct Bullet { float x; float y; float vx; float vy; float life; int objIndex; float angleRad; float faceSign; int ownerId; };
    std::vector<Bullet> m_bullets;
    const float BULLET_SPEED = 3.5f;
    const float BULLET_LIFETIME = 2.0f;
    const float BULLET_SPAWN_OFFSET_X = 0.12f;
    const float BULLET_SPAWN_OFFSET_Y = -0.01f;
    const float BULLET_COLLISION_WIDTH  = 0.02f;
    const float BULLET_COLLISION_HEIGHT = 0.02f;
    int m_bulletObjectId = 1300; // from scene
    std::vector<std::unique_ptr<Object>> m_bulletObjs;
    std::vector<int> m_freeBulletSlots;
    int CreateOrAcquireBulletObject();
    void DrawBullets(Camera* cam);
    
    void SpawnBulletFromCharacter(const Character& ch);
    void SpawnBulletFromCharacterWithJitter(const Character& ch, float jitterDeg);
    void UpdateBullets(float dt);
    void UpdateGunBursts();
    void UpdateGunReloads();

    std::unique_ptr<WallCollision> m_wallCollision;

    bool m_p1ShotPending = false;
    bool m_p2ShotPending = false;
    float m_p1GunStartTime = -1.0f;
    float m_p2GunStartTime = -1.0f;
    static constexpr float GUN_MIN_REVERSE_TIME = 0.15f;
    static constexpr float GUN_MIN_HOLD_ANIM1   = 0.15f;
    float GetGunRequiredTime() const { return GUN_MIN_REVERSE_TIME + GUN_MIN_HOLD_ANIM1; }
    void TryCompletePendingShots();
    const float CLOUD_MOVE_SPEED = 0.2f;
    const float CLOUD_SPACING = 1.74f;
    const float CLOUD_LEFT_BOUNDARY = -6.0f;
    const int TOTAL_CLOUDS = 10;
    
    // HUD gun display
    int m_player1GunTexId = 40;
    int m_player2GunTexId = 40;
    Vector3 m_hudGun1BaseScale = Vector3(0.0f, 0.0f, 1.0f);
    Vector3 m_hudGun2BaseScale = Vector3(0.0f, 0.0f, 1.0f);

    static constexpr int   M4A1_BURST_COUNT = 5;
    static constexpr float M4A1_BURST_INTERVAL = 0.08f;
    bool  m_p1BurstActive = false;
    int   m_p1BurstRemaining = 0;
    float m_p1NextBurstTime = 0.0f;
    bool  m_p2BurstActive = false;
    int   m_p2BurstRemaining = 0;
    float m_p2NextBurstTime = 0.0f;

    static constexpr float SHOTGUN_RELOAD_TIME = 0.30f;
    bool  m_p1ReloadPending = false;
    float m_p1ReloadExitTime = -1.0f;
    bool  m_p2ReloadPending = false;
    float m_p2ReloadExitTime = -1.0f;
    
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
    
    static bool IsShowHitboxHurtbox() { return s_showHitboxHurtbox; }
    static bool IsShowPlatformBoxes() { return s_showPlatformBoxes; }
    static bool IsShowWallBoxes() { return s_showWallBoxes; }
    static bool IsShowLadderBoxes() { return s_showLadderBoxes; }
    static bool IsShowTeleportBoxes() { return s_showTeleportBoxes; }
    
    // Health system methods
    void UpdateHealthBars();
    float GetPlayer1Health() const { return m_player1Health; }
    float GetPlayer2Health() const { return m_player2Health; }
    
    // Cloud movement system
    void UpdateCloudMovement(float deltaTime);
    
    // Fan rotation system
    void UpdateFanRotation(float deltaTime);

private:
    void DrawHudPortraits();
    // Item pickup system
    void HandleItemPickup();
    static const int AXE_OBJECT_ID = 1100;
    static const int SWORD_OBJECT_ID = 1101;
    static const int PIPE_OBJECT_ID  = 1102;
    static const int HUD_TEX_AXE   = 30;
    static const int HUD_TEX_SWORD = 31;
    static const int HUD_TEX_PIPE  = 32;
    bool m_isAxeAvailable = false;
    bool m_isSwordAvailable = false;
    bool m_isPipeAvailable  = false;
    
    // HUD weapons
    void UpdateHudWeapons();
    Vector3 m_hudWeapon1BaseScale = Vector3(0.0f, 0.0f, 1.0f);
    Vector3 m_hudWeapon2BaseScale = Vector3(0.0f, 0.0f, 1.0f);

    // Item lifetime tracking
    struct ItemLife { int id; float timer; };
    std::vector<ItemLife> m_itemLives;
}; 