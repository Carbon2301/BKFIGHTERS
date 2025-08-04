#include "stdafx.h"
#include "CharacterCombat.h"
#include "Character.h"
#include "CharacterAnimation.h"
#include "CharacterMovement.h"
#include "AnimationManager.h"
#include <iostream>
#include <cstdlib>

const float CharacterCombat::COMBO_WINDOW = 0.5f;
const float CharacterCombat::HIT_DURATION = 0.3f;
const float CharacterCombat::HITBOX_DURATION = 0.2f;

CharacterCombat::CharacterCombat() 
    : m_comboCount(0), m_comboTimer(0.0f),
      m_isInCombo(false), m_comboCompleted(false),
      m_axeComboCount(0), m_axeComboTimer(0.0f),
      m_isInAxeCombo(false), m_axeComboCompleted(false),
      m_isKicking(false), m_isHit(false), m_hitTimer(0.0f),
      m_showHitbox(false), m_hitboxTimer(0.0f), m_hitboxWidth(0.0f), m_hitboxHeight(0.0f),
      m_hitboxOffsetX(0.0f), m_hitboxOffsetY(0.0f) {
}

CharacterCombat::~CharacterCombat() {
}

void CharacterCombat::Update(float deltaTime) {
    UpdateComboTimers(deltaTime);
    UpdateHitboxTimer(deltaTime);
    
    // Update hit timer
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
    
    if (!m_isInCombo) {
        m_comboCount = 1;
        m_isInCombo = true;
        m_comboTimer = COMBO_WINDOW;
        animation->PlayAnimation(10, false);
        
        // Show hitbox for punch 1
        float hitboxWidth = 0.07f;
        float hitboxHeight = 0.07f;
        float hitboxOffsetX = animation->IsFacingLeft(movement) ? -0.08f : 0.08f;
        float hitboxOffsetY = -0.05f;
        ShowHitbox(hitboxWidth, hitboxHeight, hitboxOffsetX, hitboxOffsetY);
        
        std::cout << "=== COMBO START ===" << std::endl;
        std::cout << "Combo " << m_comboCount << ": Punch1!" << std::endl;
        std::cout << "Press J again within " << COMBO_WINDOW << " seconds for next punch!" << std::endl;
    } else if (m_comboTimer > 0.0f) {
        m_comboCount++;
        m_comboTimer = COMBO_WINDOW;
        
        if (m_comboCount == 2) {
            animation->PlayAnimation(11, false);
            
            // Show hitbox for punch 2
            float hitboxWidth = 0.07f;
            float hitboxHeight = 0.07f;
            float hitboxOffsetX = animation->IsFacingLeft(movement) ? -0.08f : 0.08f;
            float hitboxOffsetY = -0.05f;
            ShowHitbox(hitboxWidth, hitboxHeight, hitboxOffsetX, hitboxOffsetY);
        } else if (m_comboCount == 3) {
            animation->PlayAnimation(12, false);
            
            // Show hitbox for punch 3
            float hitboxWidth = 0.07f;
            float hitboxHeight = 0.07f;
            float hitboxOffsetX = animation->IsFacingLeft(movement) ? -0.1f : 0.1f;
            float hitboxOffsetY = -0.05f;
            ShowHitbox(hitboxWidth, hitboxHeight, hitboxOffsetX, hitboxOffsetY);
            
            m_comboCompleted = true;
        } else if (m_comboCount > 3) {
            m_comboCount = 3;
            m_comboCompleted = true;
        }
    } else {
        m_comboCount = 1;
        m_isInCombo = true;
        m_comboTimer = COMBO_WINDOW;
        animation->PlayAnimation(10, false);
        
        // Show hitbox for punch 1
        float hitboxWidth = 0.07f;
        float hitboxHeight = 0.07f;
        float hitboxOffsetX = animation->IsFacingLeft(movement) ? -0.13f : 0.13f;
        float hitboxOffsetY = -0.05f;
        ShowHitbox(hitboxWidth, hitboxHeight, hitboxOffsetX, hitboxOffsetY);
    }
}

void CharacterCombat::HandleAxeCombo(CharacterAnimation* animation, CharacterMovement* movement) {
    if (!animation || !movement) return;
    
    if (!m_isInAxeCombo) {
        m_axeComboCount = 1;
        m_isInAxeCombo = true;
        m_axeComboTimer = COMBO_WINDOW;
        animation->PlayAnimation(20, false);
    } else if (m_axeComboTimer > 0.0f) {
        m_axeComboCount++;
        m_axeComboTimer = COMBO_WINDOW;
        
        if (m_axeComboCount == 2) {
            animation->PlayAnimation(21, false);
        } else if (m_axeComboCount == 3) {
            animation->PlayAnimation(22, false);
            m_axeComboCompleted = true;
        } else if (m_axeComboCount > 3) {
            m_axeComboCount = 3;
            m_axeComboCompleted = true;
        }
    } else {
        m_axeComboCount = 1;
        m_isInAxeCombo = true;
        m_axeComboTimer = COMBO_WINDOW;
        animation->PlayAnimation(20, false);
    }
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
    
    bool attackerFacingLeft = rand() % 2 == 0;
    
    CancelAllCombos();
    
    int randomHitAnimation = (rand() % 2) + 8;
    
    m_isHit = true;
    m_hitTimer = HIT_DURATION;
    
    animation->PlayAnimation(randomHitAnimation, false);
    
    std::cout << "=== RANDOM HIT ===" << std::endl;
    std::cout << "Character hit! Playing GetHit animation " << randomHitAnimation << std::endl;
    std::cout << "===================" << std::endl;
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

void CharacterCombat::TriggerGetHit(CharacterAnimation* animation, const Character& attacker) {
    if (!animation || m_isHit) {
        return;
    }
    
    CancelAllCombos();
    
    bool attackerFacingLeft = attacker.IsFacingLeft();
    // Note: Character facing direction should be set by the Character class
    
    int randomHitAnimation = (rand() % 2) + 8;
    
    m_isHit = true;
    m_hitTimer = HIT_DURATION;
    
    animation->PlayAnimation(randomHitAnimation, false);
    
    std::cout << "=== HIT DETECTED ===" << std::endl;
    std::cout << "Character hit! Playing GetHit animation " << randomHitAnimation << std::endl;
    std::cout << "Attacker facing: " << (attackerFacingLeft ? "LEFT" : "RIGHT") << std::endl;
    std::cout << "===================" << std::endl;
} 