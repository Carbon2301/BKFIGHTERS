#pragma once
#include <memory>
#include "AnimationManager.h"
#include "Object.h"

// Forward declarations
class Camera;

// Character states
enum class CharState { Idle, MoveLeft, MoveRight, MoveBack, MoveFront };

class Character {
private:
    // Basic properties
    float m_posX;
    float m_posY;
    bool m_facingLeft;
    CharState m_state;
    
    // Movement properties
    bool m_isJumping;
    float m_jumpVelocity;
    float m_jumpStartY;
    static const float JUMP_FORCE;
    static const float GRAVITY;
    static const float GROUND_Y;
    static const float MOVE_SPEED;
    
    // Animation
    std::shared_ptr<AnimationManager> m_animManager;
    int m_lastAnimation;
    
    // Combo system variables
    int m_comboCount;
    float m_comboTimer;
    static const float COMBO_WINDOW;
    bool m_isInCombo;
    bool m_comboCompleted;
    
    // Axe combo system variables
    int m_axeComboCount;
    float m_axeComboTimer;
    bool m_isInAxeCombo;
    bool m_axeComboCompleted;
    
    // Kick system variables
    bool m_isKicking;

public:
    Character();
    ~Character();
    
    // Initialization
    void Initialize(std::shared_ptr<AnimationManager> animManager);
    
    // Core update
    void Update(float deltaTime);
    void Draw(Camera* camera);
    
    // Movement
    void HandleMovement(float deltaTime, const bool* keyStates);
    void HandleJump(float deltaTime, const bool* keyStates);
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
    bool IsInCombo() const { return m_isInCombo || m_isInAxeCombo; }
    bool IsKicking() const { return m_isKicking; }
    
    // Animation
    void PlayAnimation(int animIndex, bool loop = true);
    int GetCurrentAnimation() const;
    bool IsAnimationPlaying() const;
    
    // Getters for combo system
    int GetComboCount() const { return m_comboCount; }
    float GetComboTimer() const { return m_comboTimer; }
    bool IsComboCompleted() const { return m_comboCompleted; }
    int GetAxeComboCount() const { return m_axeComboCount; }
    float GetAxeComboTimer() const { return m_axeComboTimer; }
    bool IsAxeComboCompleted() const { return m_axeComboCompleted; }
}; 