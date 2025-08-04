#pragma once
#include <memory>
#include <vector>
#include "AnimationManager.h"
#include "../../Utilities/Math.h"

class InputManager;
class Camera;

enum class CharState {
    Idle,
    MoveLeft,
    MoveRight
};

// Input configuration for different players
struct PlayerInputConfig {
    int moveLeftKey;
    int moveRightKey;
    int jumpKey;
    int sitKey;
    int rollKey;
    int punchKey;
    int axeKey;
    int kickKey;
    
    PlayerInputConfig(int left, int right, int jump, int sit, int roll, int punch, int axe, int kick)
        : moveLeftKey(left), moveRightKey(right), jumpKey(jump), sitKey(sit), 
          rollKey(roll), punchKey(punch), axeKey(axe), kickKey(kick) {}
};

class Character {
private:
    // Basic properties
    float m_posX, m_posY;
    float m_groundY; // Vị trí mặt đất khởi tạo
    bool m_facingLeft;
    CharState m_state;
    
    // Movement properties
    bool m_isJumping;
    float m_jumpVelocity;
    float m_jumpStartY;
    
    // Animation
    std::shared_ptr<AnimationManager> m_animManager;
    int m_lastAnimation;
    int m_objectId; // Object ID for this character
    std::unique_ptr<class Object> m_characterObject; // Own Object instance for this character
    std::unique_ptr<class Object> m_hitboxObject; // Object for hitbox visualization
    
    // Combo system variables
    int m_comboCount;
    float m_comboTimer;
    bool m_isInCombo;
    bool m_comboCompleted;
    
    // Axe combo system variables
    int m_axeComboCount;
    float m_axeComboTimer;
    bool m_isInAxeCombo;
    bool m_axeComboCompleted;
    
    // Kick system variables
    bool m_isKicking;
    
    // Sitting state tracking
    bool m_isSitting;
    
    // GetHit state tracking
    bool m_isHit;
    float m_hitTimer;
    static const float HIT_DURATION;
    
    // Hitbox system variables
    bool m_showHitbox;
    float m_hitboxTimer;
    static const float HITBOX_DURATION;
    float m_hitboxWidth;
    float m_hitboxHeight;
    float m_hitboxOffsetX;
    float m_hitboxOffsetY;
    
    // Hurtbox system variables
    std::unique_ptr<class Object> m_hurtboxObject; // Object for hurtbox visualization
    float m_hurtboxWidth;
    float m_hurtboxHeight;
    float m_hurtboxOffsetX;
    float m_hurtboxOffsetY;
    
    // Input configuration
    PlayerInputConfig m_inputConfig;

    // Constants
    static const float JUMP_FORCE;
    static const float GRAVITY;
    static const float GROUND_Y;
    static const float MOVE_SPEED;
    static const float COMBO_WINDOW;

    // Helper methods
    void HandleMovement(float deltaTime, const bool* keyStates);
    void HandleJump(float deltaTime, const bool* keyStates);
    void CancelCombosOnOtherAction(const bool* keyStates);
    void UpdateComboTimers(float deltaTime);
    void UpdateAnimationState();

public:
    Character();
    ~Character();
    
    // Initialization
    void Initialize(std::shared_ptr<AnimationManager> animManager, int objectId);
    void SetInputConfig(const PlayerInputConfig& config) { m_inputConfig = config; }
    
    // Core update
    void Update(float deltaTime);
    void Draw(class Camera* camera);
    
    // Input handling - Unified input processing
    void ProcessInput(float deltaTime, InputManager* inputManager);
    
    // Movement
    void SetPosition(float x, float y);
    Vector3 GetPosition() const { return Vector3(m_posX, m_posY, 0.0f); }
    bool IsFacingLeft() const { return m_facingLeft; }
    CharState GetState() const { return m_state; }
    bool IsJumping() const { return m_isJumping; }
    
    // Combat
    void HandlePunchCombo();
    void HandleAxeCombo();
    void HandleKick();
    void CancelAllCombos();
    void HandleRandomGetHit();
    
    // Hitbox management
    void ShowHitbox(float width, float height, float offsetX, float offsetY);
    void UpdateHitboxTimer(float deltaTime);
    void DrawHitbox(class Camera* camera);
    bool IsHitboxActive() const { return m_showHitbox && m_hitboxTimer > 0.0f; }
    
    // Hurtbox management
    void SetHurtbox(float width, float height, float offsetX, float offsetY);
    void DrawHurtbox(class Camera* camera);
    
    // Animation
    void PlayAnimation(int animIndex, bool loop);
    int GetCurrentAnimation() const;
    bool IsAnimationPlaying() const;
    
    // Combo getters
    bool IsInCombo() const { return m_isInCombo; }
    int GetComboCount() const { return m_comboCount; }
    float GetComboTimer() const { return m_comboTimer; }
    bool IsComboCompleted() const { return m_comboCompleted; }
    
    // Axe combo getters
    bool IsInAxeCombo() const { return m_isInAxeCombo; }
    int GetAxeComboCount() const { return m_axeComboCount; }
    float GetAxeComboTimer() const { return m_axeComboTimer; }
    bool IsAxeComboCompleted() const { return m_axeComboCompleted; }
    
    // Static input configurations
    static const PlayerInputConfig PLAYER1_INPUT;
    static const PlayerInputConfig PLAYER2_INPUT;
}; 