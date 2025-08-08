#pragma once
#include <memory>
#include <vector>
#include "CharacterMovement.h"
#include "CharacterCombat.h"
#include "CharacterHitbox.h"
#include "../../Utilities/Math.h"

class AnimationManager;
class CharacterAnimation;

class InputManager;
class Camera;

class Character {
private:
    std::unique_ptr<CharacterMovement> m_movement;
    std::unique_ptr<CharacterCombat> m_combat;
    std::unique_ptr<CharacterHitbox> m_hitbox;
    std::unique_ptr<CharacterAnimation> m_animation;

    // Health system
    float m_health;
    const float MAX_HEALTH = 100.0f;
    const float DAMAGE_PER_HIT = 10.0f;
    bool m_isDead;

    // Helper methods
    void CancelCombosOnOtherAction(const bool* keyStates);

    // Inventory/state
    bool m_hasAxe = false;

public:
    Character();
    ~Character();
    
    // Initialization
    void Initialize(std::shared_ptr<AnimationManager> animManager, int objectId);
    void SetInputConfig(const PlayerInputConfig& config);
    
    // Core update
    void Update(float deltaTime);
    void Draw(class Camera* camera);
    
    // Input handling
    void ProcessInput(float deltaTime, InputManager* inputManager);
    
    // Movement
    void SetPosition(float x, float y);
    Vector3 GetPosition() const;
    bool IsFacingLeft() const;
    void SetFacingLeft(bool facingLeft);
    CharState GetState() const;
    bool IsJumping() const;
    bool IsSitting() const;
    CharacterMovement* GetMovement() const { return m_movement.get(); }
    
    // Combat
    void HandlePunchCombo();
    void HandleAxeCombo();
    void HandleKick();
    void HandleDie();
    void TriggerDie();
    void TriggerDieFromAttack(const Character& attacker);
    void CancelAllCombos();
    void HandleRandomGetHit();
    
    // Hitbox management
    void DrawHitbox(class Camera* camera, bool forceShow = false);
    void DrawHitboxAndHurtbox(class Camera* camera);
    bool IsHitboxActive() const;
    
    // Hurtbox management
    void SetHurtbox(float width, float height, float offsetX, float offsetY);
    void DrawHurtbox(class Camera* camera, bool forceShow = false);
    
    // Hurtbox getters for collision detection
    float GetHurtboxWidth() const;
    float GetHurtboxHeight() const;
    float GetHurtboxOffsetX() const;
    float GetHurtboxOffsetY() const;
    
    // Hitbox getters for collision detection
    float GetHitboxWidth() const;
    float GetHitboxHeight() const;
    float GetHitboxOffsetX() const;
    float GetHitboxOffsetY() const;
    
    // Collision detection
    bool CheckHitboxCollision(const Character& other) const;
    void TriggerGetHit(const Character& attacker);
    bool IsHit() const;
    
    // Animation
    void PlayAnimation(int animIndex, bool loop);
    int GetCurrentAnimation() const;
    bool IsAnimationPlaying() const;
    void GetCurrentFrameUV(float& u0, float& v0, float& u1, float& v1) const;
    
    // Combat getters
    bool IsInCombo() const;
    int GetComboCount() const;
    float GetComboTimer() const;
    bool IsComboCompleted() const;
    
    // Axe combo getters
    bool IsInAxeCombo() const;
    int GetAxeComboCount() const;
    float GetAxeComboTimer() const;
    bool IsAxeComboCompleted() const;
    
    // Kick getter
    bool IsKicking() const;
    
    // Inventory/state
    void SetHasAxe(bool hasAxe) { m_hasAxe = hasAxe; }
    bool HasAxe() const { return m_hasAxe; }
    
    // Health system
    float GetHealth() const { return m_health; }
    float GetMaxHealth() const { return MAX_HEALTH; }
    bool IsDead() const { return m_isDead; }
    void TakeDamage(float damage);
    void Heal(float amount);
    void ResetHealth();

}; 