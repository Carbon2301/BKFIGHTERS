#pragma once
#include <memory>
#include <vector>
#include "AnimationManager.h"
#include "CharacterMovement.h"
#include "CharacterCombat.h"
#include "../../Utilities/Math.h"

class InputManager;
class Camera;

class Character {
private:
    // Movement component
    std::unique_ptr<CharacterMovement> m_movement;
    
    // Combat component
    std::unique_ptr<CharacterCombat> m_combat;
    
    // Animation
    std::shared_ptr<AnimationManager> m_animManager;
    int m_lastAnimation;
    int m_objectId; // Object ID for this character
    std::unique_ptr<class Object> m_characterObject; // Own Object instance for this character
    std::unique_ptr<class Object> m_hitboxObject; // Object for hitbox visualization
    
    // Hurtbox system variables
    std::unique_ptr<class Object> m_hurtboxObject; // Object for hurtbox visualization
    float m_hurtboxWidth;
    float m_hurtboxHeight;
    float m_hurtboxOffsetX;
    float m_hurtboxOffsetY;

    // Helper methods
    void CancelCombosOnOtherAction(const bool* keyStates);
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
    void DrawHitbox(class Camera* camera, bool forceShow = false);
    bool IsHitboxActive() const;
    
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