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

    // Leo thang: điều khiển frame step-by-step
    float m_climbHoldTimer = 0.0f;
    static constexpr float CLIMB_HOLD_STEP_INTERVAL = 0.12f; // giữ phím: tốc độ chuyển frame liên tục
    int m_lastClimbDir = 0; // 1: lên, -1: xuống, 0: đứng yên
    bool m_prevClimbUpPressed = false;
    bool m_prevClimbDownPressed = false;
    // Phân biệt nhấn-nhả vs giữ khi đi xuống
    float m_downPressStartTime = -1.0f; // giây
    static constexpr float CLIMB_DOWN_HOLD_THRESHOLD = 0.15f; // > 150ms coi như giữ

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