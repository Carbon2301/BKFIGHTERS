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

    void UpdateComboTimers(float deltaTime);
    void UpdateHitboxTimer(float deltaTime);

    int m_nextGetHitAnimation;

public:
    CharacterCombat();
    ~CharacterCombat();
    
    void Update(float deltaTime);
    
    void HandlePunchCombo(CharacterAnimation* animation, CharacterMovement* movement);
    void HandleAxeCombo(CharacterAnimation* animation, CharacterMovement* movement);
    void HandleKick(CharacterAnimation* animation, CharacterMovement* movement);
    void CancelAllCombos();
    void HandleRandomGetHit(CharacterAnimation* animation, CharacterMovement* movement);
    
    void ShowHitbox(float width, float height, float offsetX, float offsetY);
    bool IsHitboxActive() const { return m_showHitbox && m_hitboxTimer > 0.0f; }
    
    bool CheckHitboxCollision(const Character& attacker, const Character& target) const;
    void TriggerGetHit(CharacterAnimation* animation, const Character& attacker, Character* target);
    bool IsHit() const { return m_isHit; }
    
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
}; 