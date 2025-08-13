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
public:
    enum class WeaponType { None = 0, Axe = 1, Sword = 2, Pipe = 3 };
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
    bool m_prevHardLandingActive = false;

    // Helper methods
    void CancelCombosOnOtherAction(const bool* keyStates);

    // Inventory/state
    WeaponType m_weapon = WeaponType::None;

    bool m_suppressNextPunch = false;

public:
    Character();
    ~Character();
    
    void Initialize(std::shared_ptr<AnimationManager> animManager, int objectId);
    void SetInputConfig(const PlayerInputConfig& config);
    void SetGunMode(bool enabled);
    bool IsGunMode() const;
    void SetGrenadeMode(bool enabled);
    bool IsGrenadeMode() const;
    // Werewolf API
    void SetWerewolfMode(bool enabled);
    bool IsWerewolf() const;
    void TriggerWerewolfCombo();
    void TriggerWerewolfPounce();
    
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
    void HandleSwordCombo();
    void HandlePipeCombo();
    void HandleKick();
    void HandleAirKick();
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
    void SetHurtboxDefault(float width, float height, float offsetX, float offsetY);
    void SetHurtboxFacingLeft(float width, float height, float offsetX, float offsetY);
    void SetHurtboxFacingRight(float width, float height, float offsetX, float offsetY);
    void SetHurtboxCrouchRoll(float width, float height, float offsetX, float offsetY);
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
    void GetTopFrameUV(float& u0, float& v0, float& u1, float& v1) const;
    int GetHeadTextureId() const;
    int GetBodyTextureId() const;
    float GetHeadOffsetX() const;
    float GetHeadOffsetY() const;
    
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
    void SetWeapon(WeaponType weapon) { m_weapon = weapon; }
    WeaponType GetWeapon() const { return m_weapon; }
    
    // Health system
    float GetHealth() const { return m_health; }
    float GetMaxHealth() const { return MAX_HEALTH; }
    bool IsDead() const { return m_isDead; }
    void TakeDamage(float damage, bool playSfx = true);
    void Heal(float amount);
    void ResetHealth();

     Vector3 GetGunTopWorldPosition() const;
     float GetAimAngleDeg() const;
     void MarkGunShotFired();

     void SuppressNextPunch() { m_suppressNextPunch = true; }
     CharacterAnimation* GetAnimation() const { return m_animation.get(); }

     // Werewolf action state queries for hurtbox selection
     bool IsWerewolfComboActive() const;
     bool IsWerewolfPounceActive() const;

	 // Werewolf hurtbox setters (forwarders)
	 void SetWerewolfHurtboxIdle(float w, float h, float ox, float oy);
	 void SetWerewolfHurtboxWalk(float w, float h, float ox, float oy);
	 void SetWerewolfHurtboxRun(float w, float h, float ox, float oy);
	 void SetWerewolfHurtboxJump(float w, float h, float ox, float oy);
	 void SetWerewolfHurtboxCombo(float w, float h, float ox, float oy);
	 void SetWerewolfHurtboxPounce(float w, float h, float ox, float oy);

}; 