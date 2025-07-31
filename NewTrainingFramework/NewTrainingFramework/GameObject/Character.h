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