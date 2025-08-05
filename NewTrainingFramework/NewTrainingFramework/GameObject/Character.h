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

    // Helper methods
    void CancelCombosOnOtherAction(const bool* keyStates);

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
    
    // Combat
    void HandlePunchCombo();
    void HandleAxeCombo();
    void HandleKick();
    void HandleDie();
    void TriggerDie();
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
    

}; 