#pragma once
#include <memory>
#include "../../Utilities/Math.h"

class AnimationManager;

class Camera;
class Object;
class CharacterMovement;
class CharacterCombat;

class CharacterAnimation {
private:
    std::shared_ptr<AnimationManager> m_animManager;
    std::unique_ptr<class Object> m_characterObject;
    int m_lastAnimation;
    int m_objectId;

    // Helper methods
    void UpdateAnimationState(CharacterMovement* movement, CharacterCombat* combat);

public:
    // Movement animation handling
    void HandleMovementAnimations(const bool* keyStates, CharacterMovement* movement, CharacterCombat* combat);
    CharacterAnimation();
    ~CharacterAnimation();
    
    // Initialization
    void Initialize(std::shared_ptr<AnimationManager> animManager, int objectId);
    
    // Core update
    void Update(float deltaTime, CharacterMovement* movement, CharacterCombat* combat);
    void Draw(Camera* camera, CharacterMovement* movement);
    
    // Animation control
    void PlayAnimation(int animIndex, bool loop);
    int GetCurrentAnimation() const;
    bool IsAnimationPlaying() const;
    
    // Getters
    int GetObjectId() const { return m_objectId; }
    Object* GetCharacterObject() const { return m_characterObject.get(); }
    
    // Facing direction (needed for hitbox calculations)
    bool IsFacingLeft(CharacterMovement* movement) const;
}; 