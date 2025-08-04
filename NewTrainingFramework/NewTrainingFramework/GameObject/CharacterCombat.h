#pragma once
#include <memory>
#include "../../Utilities/Math.h"

class AnimationManager;
class Character;

class CharacterCombat {
private:
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
    
    // Constants
    static const float COMBO_WINDOW;

    // Helper methods
    void UpdateComboTimers(float deltaTime);
    void UpdateHitboxTimer(float deltaTime);

public:
    CharacterCombat();
    ~CharacterCombat();
    
    // Core update
    void Update(float deltaTime);
    
    // Combat actions
    void HandlePunchCombo(Character* character);
    void HandleAxeCombo(Character* character);
    void HandleKick(Character* character);
    void CancelAllCombos();
    void HandleRandomGetHit(Character* character);
    
    // Hitbox management
    void ShowHitbox(float width, float height, float offsetX, float offsetY);
    bool IsHitboxActive() const { return m_showHitbox && m_hitboxTimer > 0.0f; }
    
    // Collision detection
    bool CheckHitboxCollision(const Character& attacker, const Character& target) const;
    void TriggerGetHit(Character* character, const Character& attacker);
    bool IsHit() const { return m_isHit; }
    
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
    
    // Kick getter
    bool IsKicking() const { return m_isKicking; }
    
    // Hitbox getters for collision detection
    float GetHitboxWidth() const { return m_hitboxWidth; }
    float GetHitboxHeight() const { return m_hitboxHeight; }
    float GetHitboxOffsetX() const { return m_hitboxOffsetX; }
    float GetHitboxOffsetY() const { return m_hitboxOffsetY; }
}; 