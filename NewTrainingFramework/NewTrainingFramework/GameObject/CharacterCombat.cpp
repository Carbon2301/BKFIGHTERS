#include "stdafx.h"
#include "CharacterCombat.h"
#include "Character.h"
#include "CharacterAnimation.h"
#include "CharacterMovement.h"
#include "AnimationManager.h"
#include <iostream>
#include <cstdlib>

const float CharacterCombat::COMBO_WINDOW = 0.8f;
const float CharacterCombat::HIT_DURATION = 0.3f;
const float CharacterCombat::HITBOX_DURATION = 0.2f;
const float CharacterCombat::WEAPON_COMBO_MIN_INTERVAL = 0.4f;
const float CharacterCombat::WEAPON_COMBO_WINDOW = 0.6f;
const float CharacterCombat::PUNCH_COMBO_MIN_INTERVAL = 0.4f;
const float CharacterCombat::PUNCH_COMBO_WINDOW = 0.6f;

CharacterCombat::CharacterCombat() 
    : m_comboCount(0), m_comboTimer(0.0f),
      m_isInCombo(false), m_comboCompleted(false),
      m_axeComboCount(0), m_axeComboTimer(0.0f),
      m_isInAxeCombo(false), m_axeComboCompleted(false),
      m_isKicking(false), m_isHit(false), m_hitTimer(0.0f),
      m_showHitbox(false), m_hitboxTimer(0.0f), m_hitboxWidth(0.0f), m_hitboxHeight(0.0f),
      m_hitboxOffsetX(0.0f), m_hitboxOffsetY(0.0f),
      m_nextGetHitAnimation(8),
      m_weaponComboCooldown(0.0f),
      m_punchComboCooldown(0.0f) {
}

CharacterCombat::~CharacterCombat() {
}

void CharacterCombat::Update(float deltaTime) {
    UpdateComboTimers(deltaTime);
    UpdateHitboxTimer(deltaTime);
    if (m_weaponComboCooldown > 0.0f) {
        m_weaponComboCooldown -= deltaTime;
        if (m_weaponComboCooldown < 0.0f) m_weaponComboCooldown = 0.0f;
    }
    if (m_punchComboCooldown > 0.0f) {
        m_punchComboCooldown -= deltaTime;
        if (m_punchComboCooldown < 0.0f) m_punchComboCooldown = 0.0f;
    }
    m_lungeDeltaThisFrame = 0.0f;
    if (m_isLunging) {
        float delta = m_lungeSpeed * deltaTime;
        if (delta > m_lungeRemainingDistance) delta = m_lungeRemainingDistance;
        m_lungeRemainingDistance -= delta;
        m_lungeDeltaThisFrame = delta * m_lungeDirection;
        if (m_lungeRemainingDistance <= 0.0f) {
            m_isLunging = false;
            m_lungeSpeed = 0.0f;
            m_lungeDirection = 0.0f;
        }
    }
    if (m_isInAxeCombo && m_weaponComboQueued && m_weaponComboCooldown <= 0.0f && m_axeComboTimer > 0.0f) {
    }
    
    if (m_isHit) {
        m_hitTimer -= deltaTime;
        if (m_hitTimer <= 0.0f) {
            m_isHit = false;
            m_hitTimer = 0.0f;
        }
    }
}

void CharacterCombat::UpdateComboTimers(float deltaTime) {
    if (m_isInCombo) {
        m_comboTimer -= deltaTime;
        if (m_comboTimer <= 0.0f) {
            m_isInCombo = false;
            m_comboCount = 0;
            m_comboCompleted = false;
        }
    }
    
    if (m_isInAxeCombo) {
        m_axeComboTimer -= deltaTime;
        if (m_axeComboTimer <= 0.0f) {
            m_isInAxeCombo = false;
            m_axeComboCount = 0;
            m_axeComboCompleted = false;
        }
    }
}

void CharacterCombat::UpdateHitboxTimer(float deltaTime) {
    if (m_showHitbox && m_hitboxTimer > 0.0f) {
        m_hitboxTimer -= deltaTime;
        if (m_hitboxTimer <= 0.0f) {
            m_showHitbox = false;
            m_hitboxTimer = 0.0f;
        }
    }
}

void CharacterCombat::HandlePunchCombo(CharacterAnimation* animation, CharacterMovement* movement) {
    if (!animation || !movement) return;
    if (m_punchComboCooldown > 0.0f) { m_punchComboQueued = true; return; }
    
    if (!m_isInCombo) {
        m_comboCount = 1;
        m_isInCombo = true;
        m_comboTimer = PUNCH_COMBO_WINDOW;
        animation->PlayAnimation(10, false);
        m_punchComboCooldown = PUNCH_COMBO_MIN_INTERVAL;
        
        // Show hitbox for punch 1
        float hitboxWidth = 0.03f;
        float hitboxHeight = 0.03f;
        float hitboxOffsetX = animation->IsFacingLeft(movement) ? -0.04f : 0.04f;
        float hitboxOffsetY = -0.02f;
        ShowHitbox(hitboxWidth, hitboxHeight, hitboxOffsetX, hitboxOffsetY);
        
        std::cout << "=== COMBO START ===" << std::endl;
        std::cout << "Combo " << m_comboCount << ": Punch1!" << std::endl;
        std::cout << "Press J again within " << COMBO_WINDOW << " seconds for next punch!" << std::endl;
    } else if (m_comboTimer > 0.0f) {
        m_comboCount++;
        m_comboTimer = PUNCH_COMBO_WINDOW;
        
        if (m_comboCount == 2) {
            animation->PlayAnimation(11, false);
            m_punchComboCooldown = PUNCH_COMBO_MIN_INTERVAL;
            
            // Show hitbox for punch 2
            float hitboxWidth = 0.03f;
            float hitboxHeight = 0.03f;
            float hitboxOffsetX = animation->IsFacingLeft(movement) ? -0.04f : 0.04f;
            float hitboxOffsetY = -0.02f;
            ShowHitbox(hitboxWidth, hitboxHeight, hitboxOffsetX, hitboxOffsetY);
        } else if (m_comboCount == 3) {
            animation->PlayAnimation(12, false);
            m_punchComboCooldown = PUNCH_COMBO_MIN_INTERVAL;
            
            // Show hitbox for punch 3
            float hitboxWidth = 0.03f;
            float hitboxHeight = 0.03f;
            float hitboxOffsetX = animation->IsFacingLeft(movement) ? -0.05f : 0.05f;
            float hitboxOffsetY = -0.02f;
            ShowHitbox(hitboxWidth, hitboxHeight, hitboxOffsetX, hitboxOffsetY);
            StartLunge(animation->IsFacingLeft(movement), 0.02f, 0.15f);
            
            m_comboCompleted = true;
        } else if (m_comboCount > 3) {
            m_comboCount = 3;
            m_comboCompleted = true;
        }
    } else {
        m_comboCount = 1;
        m_isInCombo = true;
        m_comboTimer = PUNCH_COMBO_WINDOW;
        animation->PlayAnimation(10, false);
        m_punchComboCooldown = PUNCH_COMBO_MIN_INTERVAL;
        
        // Show hitbox for punch 1
        float hitboxWidth = 0.03f;
        float hitboxHeight = 0.03f;
        float hitboxOffsetX = animation->IsFacingLeft(movement) ? -0.04f : 0.04f;
        float hitboxOffsetY = -0.02f;
        ShowHitbox(hitboxWidth, hitboxHeight, hitboxOffsetX, hitboxOffsetY);
    }
}

void CharacterCombat::HandleAxeCombo(CharacterAnimation* animation, CharacterMovement* movement) {
    if (!animation || !movement) return;
    
    HandleWeaponCombo(animation, movement, 20, 21, 22);
}

void CharacterCombat::HandleWeaponCombo(CharacterAnimation* animation, CharacterMovement* movement, int anim1, int anim2, int anim3) {
    if (!animation || !movement) return;
    if (m_weaponComboCooldown > 0.0f) { m_weaponComboQueued = true; return; }
    auto computeWeaponHitbox = [&](int weaponAnim1, int step, bool facingLeft,
                                   float& outW, float& outH, float& outOX, float& outOY){
        outW = 0.045f; outH = 0.035f; outOX = facingLeft ? -0.055f : 0.055f; outOY = -0.02f;

        enum class W { Axe, Sword, Pipe, Generic };
        W kind = W::Generic;
        if (weaponAnim1 == 20) kind = W::Axe;
        else if (weaponAnim1 == 23) kind = W::Sword;
        else if (weaponAnim1 == 26) kind = W::Pipe;

        switch (kind) {
            case W::Axe: {
                if (step == 1) { outW = 0.2f;  outH = 0.08f;  outOX = facingLeft ? -0.02f  : 0.02f;  outOY = -0.04f; }
                else if (step == 2) { outW = 0.12f; outH = 0.20f;  outOX = facingLeft ? -0.05f : 0.05f; outOY = -0.02f; }
                else               { outW = 0.10f;  outH = 0.16f; outOX = facingLeft ? -0.07f  : 0.07f;  outOY = -0.02f; }
                break;
            }
            case W::Sword: {
                if (step == 1) { outW = 0.24f;  outH = 0.08f;  outOX = facingLeft ? 0.0f : 0.0f;  outOY = -0.04f; }
                else if (step == 2) { outW = 0.18f; outH = 0.20f;  outOX = facingLeft ? -0.02f : 0.02f; outOY = -0.02f; }
                else { outW = 0.15f;  outH = 0.18f; outOX = facingLeft ? -0.06f : 0.06f;  outOY = -0.02f; }
                break;
            }
            case W::Pipe: {
                if (step == 1) { outW = 0.13f;  outH = 0.06f;  outOX = facingLeft ? -0.02f : 0.02f;  outOY = -0.02f; }
                else if (step == 2) { outW = 0.15f; outH = 0.16f;  outOX = facingLeft ? -0.02f : 0.02f; outOY = -0.02f; }
                else { outW = 0.10f;  outH = 0.14f; outOX = facingLeft ? -0.07f : 0.07f;  outOY = -0.02f; }
                break;
            }
            default: {
                if (step == 1) { outW = 0.045f; outH = 0.035f; outOX = facingLeft ? -0.055f : 0.055f; outOY = -0.02f; }
                else if (step == 2) { outW = 0.05f;  outH = 0.04f;  outOX = facingLeft ? -0.06f  : 0.06f;  outOY = -0.02f; }
                else               { outW = 0.055f; outH = 0.045f; outOX = facingLeft ? -0.065f : 0.065f; outOY = -0.02f; }
                break;
            }
        }
    };

    if (!m_isInAxeCombo) {
        m_axeComboCount = 1;
        m_isInAxeCombo = true;
        m_axeComboTimer = WEAPON_COMBO_WINDOW;
        animation->PlayAnimation(anim1, false);
        m_weaponComboCooldown = WEAPON_COMBO_MIN_INTERVAL;
        float w, h, ox, oy;
        bool facingLeft = animation->IsFacingLeft(movement);
        computeWeaponHitbox(anim1, 1, facingLeft, w, h, ox, oy);
        ShowHitbox(w, h, ox, oy);
    } else if (m_axeComboTimer > 0.0f) {
        m_axeComboCount++;
        m_axeComboTimer = WEAPON_COMBO_WINDOW;
        if (m_axeComboCount == 2) {
            animation->PlayAnimation(anim2, false);
            m_weaponComboCooldown = WEAPON_COMBO_MIN_INTERVAL;
            float w, h, ox, oy;
            bool facingLeft = animation->IsFacingLeft(movement);
            computeWeaponHitbox(anim1, 2, facingLeft, w, h, ox, oy);
            ShowHitbox(w, h, ox, oy);
        } else if (m_axeComboCount == 3) {
            animation->PlayAnimation(anim3, false);
            m_axeComboCompleted = true;
            m_weaponComboCooldown = WEAPON_COMBO_MIN_INTERVAL;
            StartLunge(animation->IsFacingLeft(movement), 0.02f, 0.15f);
            float w, h, ox, oy;
            bool facingLeft = animation->IsFacingLeft(movement);
            computeWeaponHitbox(anim1, 3, facingLeft, w, h, ox, oy);
            ShowHitbox(w, h, ox, oy);
        } else if (m_axeComboCount > 3) {
            m_axeComboCount = 3;
            m_axeComboCompleted = true;
        }
    } else {
        m_axeComboCount = 1;
        m_isInAxeCombo = true;
        m_axeComboTimer = WEAPON_COMBO_WINDOW;
        animation->PlayAnimation(anim1, false);
        m_weaponComboCooldown = WEAPON_COMBO_MIN_INTERVAL;
        float w, h, ox, oy;
        bool facingLeft = animation->IsFacingLeft(movement);
        computeWeaponHitbox(anim1, 1, facingLeft, w, h, ox, oy);
        ShowHitbox(w, h, ox, oy);
    }
    m_weaponComboQueued = false;
}

void CharacterCombat::AdvanceQueuedWeaponCombo(CharacterAnimation* animation, CharacterMovement* movement) {
    (void)animation;
    (void)movement;
}

void CharacterCombat::HandleKick(CharacterAnimation* animation, CharacterMovement* movement) {
    if (!animation || !movement) return;
    
    if (m_isInCombo) {
        m_isInCombo = false;
        m_comboCount = 0;
        m_comboTimer = 0.0f;
        m_comboCompleted = false;
    }
    
    if (m_isInAxeCombo) {
        m_isInAxeCombo = false;
        m_axeComboCount = 0;
        m_axeComboTimer = 0.0f;
        m_axeComboCompleted = false;
    }
    
    m_isKicking = true;
    animation->PlayAnimation(19, false);
}

void CharacterCombat::HandleAirKick(CharacterAnimation* animation, CharacterMovement* movement) {
    if (!animation || !movement) return;
    if (!movement->IsJumping()) return;

    if (m_isInCombo) {
        m_isInCombo = false;
        m_comboCount = 0;
        m_comboTimer = 0.0f;
        m_comboCompleted = false;
    }
    if (m_isInAxeCombo) {
        m_isInAxeCombo = false;
        m_axeComboCount = 0;
        m_axeComboTimer = 0.0f;
        m_axeComboCompleted = false;
    }

    m_isKicking = true;
    animation->PlayAnimation(17, false);

    float hitboxWidth = 0.04f;
    float hitboxHeight = 0.03f;
    float hitboxOffsetX = animation->IsFacingLeft(movement) ? -0.05f : 0.05f;
    float hitboxOffsetY = -0.01f;
    ShowHitbox(hitboxWidth, hitboxHeight, hitboxOffsetX, hitboxOffsetY);
}

void CharacterCombat::CancelAllCombos() {
    m_isInCombo = false;
    m_comboCount = 0;
    m_comboTimer = 0.0f;
    m_comboCompleted = false;
    
    m_isInAxeCombo = false;
    m_axeComboCount = 0;
    m_axeComboTimer = 0.0f;
    m_axeComboCompleted = false;
    
    m_isKicking = false;
    m_isHit = false;
    m_hitTimer = 0.0f;
}

void CharacterCombat::HandleRandomGetHit(CharacterAnimation* animation, CharacterMovement* movement) {
    if (!animation || !movement) return;
    
    CancelAllCombos();
    
    m_isHit = true;
    m_hitTimer = HIT_DURATION;
    
    animation->PlayAnimation(m_nextGetHitAnimation, false);
    m_nextGetHitAnimation = (m_nextGetHitAnimation == 8) ? 9 : 8;
    
}

void CharacterCombat::ShowHitbox(float width, float height, float offsetX, float offsetY) {
    m_showHitbox = true;
    m_hitboxTimer = HITBOX_DURATION;
    m_hitboxWidth = width;
    m_hitboxHeight = height;
    m_hitboxOffsetX = offsetX;
    m_hitboxOffsetY = offsetY;
}



bool CharacterCombat::CheckHitboxCollision(const Character& attacker, const Character& target) const {
    if (!IsHitboxActive()) {
        return false;
    }
    
    // Calculate attacker's hitbox bounds
    Vector3 attackerPosition = attacker.GetPosition();
    float attackerHitboxX = attackerPosition.x + m_hitboxOffsetX;
    float attackerHitboxY = attackerPosition.y + m_hitboxOffsetY;
    float attackerHitboxLeft = attackerHitboxX - m_hitboxWidth * 0.5f;
    float attackerHitboxRight = attackerHitboxX + m_hitboxWidth * 0.5f;
    float attackerHitboxTop = attackerHitboxY + m_hitboxHeight * 0.5f;
    float attackerHitboxBottom = attackerHitboxY - m_hitboxHeight * 0.5f;
    
    // Calculate target's hurtbox bounds
    Vector3 targetPosition = target.GetPosition();
    float targetHurtboxX = targetPosition.x + target.GetHurtboxOffsetX();
    float targetHurtboxY = targetPosition.y + target.GetHurtboxOffsetY();
    float targetHurtboxLeft = targetHurtboxX - target.GetHurtboxWidth() * 0.5f;
    float targetHurtboxRight = targetHurtboxX + target.GetHurtboxWidth() * 0.5f;
    float targetHurtboxTop = targetHurtboxY + target.GetHurtboxHeight() * 0.5f;
    float targetHurtboxBottom = targetHurtboxY - target.GetHurtboxHeight() * 0.5f;
    
    // Check for collision using AABB
    bool collisionX = attackerHitboxRight >= targetHurtboxLeft && attackerHitboxLeft <= targetHurtboxRight;
    bool collisionY = attackerHitboxTop >= targetHurtboxBottom && attackerHitboxBottom <= targetHurtboxTop;
    
    return collisionX && collisionY;
}

void CharacterCombat::TriggerGetHit(CharacterAnimation* animation, const Character& attacker, Character* target) {
    if (!animation || m_isHit) {
        return;
    }
    
    CancelAllCombos();
    
    bool attackerFacingLeft = attacker.IsFacingLeft();
    
    m_isHit = true;
    m_hitTimer = HIT_DURATION;
    
    animation->PlayAnimation(m_nextGetHitAnimation, false);
    int playedAnim = m_nextGetHitAnimation;
    m_nextGetHitAnimation = (m_nextGetHitAnimation == 8) ? 9 : 8;
    
    // Apply damage to target
    if (target) {
        float currentHealth = target->GetHealth();
        target->TakeDamage(10.0f);
        
        if (currentHealth > 0.0f && target->GetHealth() <= 0.0f) {
            target->TriggerDieFromAttack(attacker);
        }
    }
    
    std::cout << "=== HIT DETECTED ===" << std::endl;
    std::cout << "Character hit! Playing GetHit animation " << playedAnim << std::endl;
    std::cout << "Attacker facing: " << (attackerFacingLeft ? "LEFT" : "RIGHT") << std::endl;
    std::cout << "Damage applied: 10 HP" << std::endl;
    std::cout << "===================" << std::endl;
} 