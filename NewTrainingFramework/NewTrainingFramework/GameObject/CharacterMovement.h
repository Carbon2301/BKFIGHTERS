#pragma once
#include <memory>
#include "../../Utilities/Math.h"
#include "WallCollision.h"
#include "PlatformCollision.h"

enum class CharState {
    Idle,
    MoveLeft,
    MoveRight,
    Die
};

struct PlayerInputConfig {
    int moveLeftKey;
    int moveRightKey;
    int jumpKey;
    int sitKey;
    int rollKey;
    int punchKey;
    int axeKey;
    int kickKey;
    int dieKey;
    
    int rollLeftKey1;
    int rollLeftKey2;
    int rollRightKey1;
    int rollRightKey2;
    
    PlayerInputConfig() : moveLeftKey(0), moveRightKey(0), jumpKey(0), sitKey(0), 
                         rollKey(0), punchKey(0), axeKey(0), kickKey(0), dieKey(0),
                         rollLeftKey1(0), rollLeftKey2(0), rollRightKey1(0), rollRightKey2(0) {}
    
    PlayerInputConfig(int left, int right, int jump, int sit, int roll, int punch, int axe, int kick, int die,
                     int rollLeft1, int rollLeft2, int rollRight1, int rollRight2)
        : moveLeftKey(left), moveRightKey(right), jumpKey(jump), sitKey(sit), 
          rollKey(roll), punchKey(punch), axeKey(axe), kickKey(kick), dieKey(die),
          rollLeftKey1(rollLeft1), rollLeftKey2(rollLeft2), rollRightKey1(rollRight1), rollRightKey2(rollRight2) {}
};

class CharacterMovement {
private:
    // Position and state
    float m_posX, m_posY;
    float m_groundY;
    bool m_facingLeft;
    CharState m_state;
    
    // Jump properties
    bool m_isJumping;
    float m_jumpVelocity;
    float m_jumpStartY;
    bool m_wasJumping;
    
    // Sitting state
    bool m_isSitting;
    
    // Die state
    bool m_isDying;
    bool m_isDead;
    float m_dieTimer;
    float m_knockdownTimer;
    bool m_knockdownComplete;
    bool m_attackerFacingLeft;
    
    // Input configuration
    PlayerInputConfig m_inputConfig;

    // Double-tap cho chạy nhanh
    float m_lastLeftPressTime = -1.0f;
    float m_lastRightPressTime = -1.0f;
    bool m_isRunningLeft = false;
    bool m_isRunningRight = false;
    bool m_prevLeftKey = false;
    bool m_prevRightKey = false;
    static constexpr float DOUBLE_TAP_THRESHOLD = 0.2f; // 200ms

    // Platform collision
    std::unique_ptr<PlatformCollision> m_platformCollision;
    float m_characterWidth;
    float m_characterHeight;
    bool m_isOnPlatform;
    float m_currentPlatformY;
    
    // Wall collision
    std::unique_ptr<WallCollision> m_wallCollision;

    // Constants
    static const float JUMP_FORCE;
    static const float GRAVITY;
    static const float GROUND_Y;
    static const float MOVE_SPEED;

public:
    CharacterMovement();
    CharacterMovement(const PlayerInputConfig& inputConfig);
    ~CharacterMovement();
    
    void Initialize(float startX, float startY, float groundY);
    void SetInputConfig(const PlayerInputConfig& config) { m_inputConfig = config; }
    const PlayerInputConfig& GetInputConfig() const { return m_inputConfig; }
    
    void Update(float deltaTime, const bool* keyStates);
    void UpdateWithHurtbox(float deltaTime, const bool* keyStates, float hurtboxWidth, float hurtboxHeight, float hurtboxOffsetX, float hurtboxOffsetY);
    
    void HandleMovement(float deltaTime, const bool* keyStates);
    void HandleJump(float deltaTime, const bool* keyStates);
    void HandleJumpWithHurtbox(float deltaTime, const bool* keyStates, float hurtboxWidth, float hurtboxHeight, float hurtboxOffsetX, float hurtboxOffsetY);
    void HandleLanding(const bool* keyStates);
    void HandleLandingWithHurtbox(const bool* keyStates, float hurtboxWidth, float hurtboxHeight, float hurtboxOffsetX, float hurtboxOffsetY);
    void HandleDie(float deltaTime);
    
    void SetPosition(float x, float y);
    void TriggerDie(bool attackerFacingLeft = false);
    Vector3 GetPosition() const { return Vector3(m_posX, m_posY, 0.0f); }
    float GetPosX() const { return m_posX; }
    float GetPosY() const { return m_posY; }
    bool IsFacingLeft() const { return m_facingLeft; }
    CharState GetState() const { return m_state; }
    bool IsJumping() const { return m_isJumping; }
    bool IsSitting() const { return m_isSitting; }
    bool IsDying() const { return m_isDying; }
    bool IsDead() const { return m_isDead; }
    float GetDieTimer() const { return m_dieTimer; }
    void SetFacingLeft(bool facingLeft) { m_facingLeft = facingLeft; }
    
    // Platform collision methods
    void AddPlatform(float x, float y, float width, float height);
    void ClearPlatforms();
    void SetCharacterSize(float width, float height);
    bool CheckPlatformCollision(float& newY);
    bool CheckPlatformCollisionWithHurtbox(float& newY, float hurtboxWidth, float hurtboxHeight, float hurtboxOffsetX, float hurtboxOffsetY);
    bool IsOnPlatform() const { return m_isOnPlatform; }
    float GetCurrentPlatformY() const { return m_currentPlatformY; }
    PlatformCollision* GetPlatformCollision() const { return m_platformCollision.get(); }
    
    // Wall collision methods
    void InitializeWallCollision();
    WallCollision* GetWallCollision() const { return m_wallCollision.get(); }

    // Static input configurations
    static const PlayerInputConfig PLAYER1_INPUT;
    static const PlayerInputConfig PLAYER2_INPUT;
}; 