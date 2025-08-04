#pragma once
#include <memory>
#include <vector>
#include "AnimationManager.h"
#include "CharacterMovement.h"
#include "../../Utilities/Math.h"

class InputManager;
class Camera;

class Character {
private:
    // Movement component
    std::unique_ptr<CharacterMovement> m_movement;
    
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

    // Constants
    static const float COMBO_WINDOW;

    // Helper methods
    void CancelCombosOnOtherAction(const bool* keyStates);
    void UpdateComboTimers(float deltaTime);
    void UpdateAnimationState();
    void HandleMovementAnimations(const bool* keyStates);

public:
    Character();
    ~Character();
    
    // Initialization
    void Initialize(std::shared_ptr<AnimationManager> animManager, int objectId);
    void SetInputConfig(const PlayerInputConfig& config);
    
    // Core update
    void Update(float deltaTime);
    void Draw(class Camera* camera);
    
    // Input handling - Unified input processing
    void ProcessInput(float deltaTime, InputManager* inputManager);
    
    // Movement
    void SetPosition(float x, float y);
    Vector3 GetPosition() const;
    bool IsFacingLeft() const;
    void SetFacingLeft(bool facingLeft);
    CharState GetState() const;
    bool IsJumping() const;
    bool IsSitting() const;
    
    // Combat
    void HandlePunchCombo();
    void HandleAxeCombo();
    void HandleKick();
    void CancelAllCombos();
    void HandleRandomGetHit();
    
    // Hitbox management
    void ShowHitbox(float width, float height, float offsetX, float offsetY);
    void UpdateHitboxTimer(float deltaTime);
    void DrawHitbox(class Camera* camera, bool forceShow = false);
    bool IsHitboxActive() const { return m_showHitbox && m_hitboxTimer > 0.0f; }
    
    // Hurtbox management
    void SetHurtbox(float width, float height, float offsetX, float offsetY);
    void DrawHurtbox(class Camera* camera, bool forceShow = false);
    
    // Hurtbox getters for collision detection
    float GetHurtboxWidth() const { return m_hurtboxWidth; }
    float GetHurtboxHeight() const { return m_hurtboxHeight; }
    float GetHurtboxOffsetX() const { return m_hurtboxOffsetX; }
    float GetHurtboxOffsetY() const { return m_hurtboxOffsetY; }
    
    // Collision detection
    bool CheckHitboxCollision(const Character& other) const;
    void TriggerGetHit(const Character& attacker);
    bool IsHit() const { return m_isHit; }
    
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
    

}; 