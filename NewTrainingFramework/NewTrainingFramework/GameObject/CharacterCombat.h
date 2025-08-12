#pragma once
#include <memory>
#include "../../Utilities/Math.h"

class AnimationManager;
class Character;
class CharacterAnimation;
class CharacterMovement;

class CharacterCombat {
private:
    int m_comboCount;
    float m_comboTimer;
    bool m_isInCombo;
    bool m_comboCompleted;
    
    int m_axeComboCount;
    float m_axeComboTimer;
    bool m_isInAxeCombo;
    bool m_axeComboCompleted;
    
    bool m_isKicking;
    
    bool m_isHit;
    float m_hitTimer;
    static const float HIT_DURATION;
    
    bool m_showHitbox;
    float m_hitboxTimer;
    static const float HITBOX_DURATION;
    float m_hitboxWidth;
    float m_hitboxHeight;
    float m_hitboxOffsetX;
    float m_hitboxOffsetY;
    
    static const float COMBO_WINDOW;
    static const float WEAPON_COMBO_MIN_INTERVAL;
    static const float WEAPON_COMBO_WINDOW;
    static const float PUNCH_COMBO_MIN_INTERVAL;
    static const float PUNCH_COMBO_WINDOW;

    void UpdateComboTimers(float deltaTime);
    void UpdateHitboxTimer(float deltaTime);

    int m_nextGetHitAnimation;

    float m_weaponComboCooldown;
    bool m_weaponComboQueued = false;

    float m_punchComboCooldown;
    bool m_punchComboQueued = false;
    int m_currentWeaponAnim1 = -1;
    int m_currentWeaponAnim2 = -1;
    int m_currentWeaponAnim3 = -1;

    class CharacterAnimation* m_lastAnimationCtx = nullptr;
    class CharacterMovement*  m_lastMovementCtx  = nullptr;

  bool m_attackPressed = false;

public:
    CharacterCombat();
    ~CharacterCombat();
    
    void Update(float deltaTime);
    
    void HandlePunchCombo(CharacterAnimation* animation, CharacterMovement* movement);
    void HandleAxeCombo(CharacterAnimation* animation, CharacterMovement* movement);
    void HandleWeaponCombo(CharacterAnimation* animation, CharacterMovement* movement, int anim1, int anim2, int anim3);
    void HandleKick(CharacterAnimation* animation, CharacterMovement* movement);
    void HandleAirKick(CharacterAnimation* animation, CharacterMovement* movement);
    void CancelAllCombos();
    void HandleRandomGetHit(CharacterAnimation* animation, CharacterMovement* movement);
    
    void ShowHitbox(float width, float height, float offsetX, float offsetY);
    bool IsHitboxActive() const { return m_showHitbox && m_hitboxTimer > 0.0f; }
    
    bool CheckHitboxCollision(const Character& attacker, const Character& target) const;
    void TriggerGetHit(CharacterAnimation* animation, const Character& attacker, Character* target);
    bool IsHit() const { return m_isHit; }
  void SetAttackPressed(bool pressed) { m_attackPressed = pressed; }
    
    bool IsInCombo() const { return m_isInCombo; }
    int GetComboCount() const { return m_comboCount; }
    float GetComboTimer() const { return m_comboTimer; }
    bool IsComboCompleted() const { return m_comboCompleted; }
    
    bool IsInAxeCombo() const { return m_isInAxeCombo; }
    int GetAxeComboCount() const { return m_axeComboCount; }
    float GetAxeComboTimer() const { return m_axeComboTimer; }
    bool IsAxeComboCompleted() const { return m_axeComboCompleted; }
    
    bool IsKicking() const { return m_isKicking; }
    
    float GetHitboxWidth() const { return m_hitboxWidth; }
    float GetHitboxHeight() const { return m_hitboxHeight; }
    float GetHitboxOffsetX() const { return m_hitboxOffsetX; }
    float GetHitboxOffsetY() const { return m_hitboxOffsetY; }

    void StartLunge(bool facingLeft, float distance, float duration) {
        if (distance <= 0.0f || duration <= 0.0f) return;
        m_isLunging = true;
        m_lungeRemainingDistance = distance;
        m_lungeSpeed = distance / duration;
        m_lungeDirection = facingLeft ? -1.0f : 1.0f;
    }
    float ConsumeLungeDelta() {
        float d = m_lungeDeltaThisFrame;
        m_lungeDeltaThisFrame = 0.0f;
        return d;
    }

private:
    bool m_isLunging = false;
    float m_lungeRemainingDistance = 0.0f;
    float m_lungeSpeed = 0.0f;
    float m_lungeDirection = 0.0f;
    float m_lungeDeltaThisFrame = 0.0f;
}; 