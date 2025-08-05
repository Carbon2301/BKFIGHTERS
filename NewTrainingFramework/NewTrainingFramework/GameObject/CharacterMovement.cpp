#include "stdafx.h"
#include "CharacterMovement.h"
#include <iostream>
#include <SDL.h>

const float CharacterMovement::JUMP_FORCE = 3.0f;
const float CharacterMovement::GRAVITY = 8.0f;
const float CharacterMovement::GROUND_Y = 0.0f;
const float CharacterMovement::MOVE_SPEED = 0.5f;

// Static input configurations
const PlayerInputConfig CharacterMovement::PLAYER1_INPUT('A', 'D', 'W', 'S', ' ', 'J', 'L', 'K', 0, 'A', 'S', 'S', 'D');
const PlayerInputConfig CharacterMovement::PLAYER2_INPUT(0x25, 0x27, 0x26, 0x28, '0', '1', '3', '2', 0, 0x25, 0x28, 0x27, 0x28);

CharacterMovement::CharacterMovement() 
    : m_posX(0.0f), m_posY(0.0f), m_facingLeft(false), m_state(CharState::Idle),
      m_isJumping(false), m_jumpVelocity(0.0f), m_jumpStartY(0.0f), m_wasJumping(false),
      m_isSitting(false), m_isDying(false), m_isDead(false), m_dieTimer(0.0f), 
      m_knockdownTimer(0.0f), m_knockdownComplete(false), m_attackerFacingLeft(false), m_inputConfig(PLAYER1_INPUT) {
}

CharacterMovement::CharacterMovement(const PlayerInputConfig& inputConfig)
    : m_posX(0.0f), m_posY(0.0f), m_facingLeft(false), m_state(CharState::Idle),
      m_isJumping(false), m_jumpVelocity(0.0f), m_jumpStartY(0.0f), m_wasJumping(false),
      m_isSitting(false), m_isDying(false), m_isDead(false), m_dieTimer(0.0f), 
      m_knockdownTimer(0.0f), m_knockdownComplete(false), m_attackerFacingLeft(false), m_inputConfig(inputConfig) {
}

CharacterMovement::~CharacterMovement() {
}

void CharacterMovement::Initialize(float startX, float startY, float groundY) {
    m_posX = startX;
    m_posY = startY;
    m_groundY = groundY;
    m_state = CharState::Idle;
    m_facingLeft = false;
    m_isJumping = false;
    m_wasJumping = false;
    m_isSitting = false;
    m_isDying = false;
    m_isDead = false;
    m_dieTimer = 0.0f;
    m_knockdownTimer = 0.0f;
    m_knockdownComplete = false;
    m_attackerFacingLeft = false;
}

void CharacterMovement::Update(float deltaTime, const bool* keyStates) {
    if (!keyStates) {
        std::cout << "Warning: keyStates is null in CharacterMovement::Update" << std::endl;
        return;
    }
    
    m_wasJumping = m_isJumping;
    
    // Handle die state
    if (m_isDying) {
        HandleDie(deltaTime);
    } else {
        HandleMovement(deltaTime, keyStates);
        HandleJump(deltaTime, keyStates);
        
        if (m_wasJumping && !m_isJumping) {
            HandleLanding(keyStates);
        }
    }
}

void CharacterMovement::HandleMovement(float deltaTime, const bool* keyStates) {
    if (!keyStates) {
        std::cout << "Warning: keyStates is null in HandleMovement" << std::endl;
        return;
    }
    
    bool isRollingLeft = (keyStates[m_inputConfig.rollLeftKey1] && keyStates[m_inputConfig.rollLeftKey2]);
    bool isRollingRight = (keyStates[m_inputConfig.rollRightKey1] && keyStates[m_inputConfig.rollRightKey2]);
    
    bool isOtherAction = (keyStates[m_inputConfig.sitKey] || keyStates[m_inputConfig.jumpKey] || isRollingLeft || isRollingRight);
    
    float currentTime = SDL_GetTicks() / 1000.0f;

    if (keyStates[m_inputConfig.moveLeftKey] && !m_prevLeftKey) {
        if (currentTime - m_lastLeftPressTime < DOUBLE_TAP_THRESHOLD) {
            m_isRunningLeft = true;
        }
        m_lastLeftPressTime = currentTime;
    }
    if (!keyStates[m_inputConfig.moveLeftKey]) {
        m_isRunningLeft = false;
    }
    m_prevLeftKey = keyStates[m_inputConfig.moveLeftKey];

    if (keyStates[m_inputConfig.moveRightKey] && !m_prevRightKey) {
        if (currentTime - m_lastRightPressTime < DOUBLE_TAP_THRESHOLD) {
            m_isRunningRight = true;
        }
        m_lastRightPressTime = currentTime;
    }
    if (!keyStates[m_inputConfig.moveRightKey]) {
        m_isRunningRight = false;
    }
    m_prevRightKey = keyStates[m_inputConfig.moveRightKey];

    if (isRollingLeft) {
        m_state = CharState::MoveLeft;
        m_facingLeft = true;
        m_posX -= MOVE_SPEED * 1.5f * deltaTime;
    } else if (isRollingRight) {
        m_state = CharState::MoveRight;
        m_facingLeft = false;
        m_posX += MOVE_SPEED * 1.5f * deltaTime;
    } else if (keyStates[m_inputConfig.moveRightKey]) {
        m_state = CharState::MoveRight;
        m_facingLeft = false;
        if (m_isRunningRight) {
            m_posX += MOVE_SPEED * 2.0f * deltaTime;
        } else {
            m_posX += MOVE_SPEED * deltaTime;
        }
    } else if (keyStates[m_inputConfig.moveLeftKey]) {
        m_state = CharState::MoveLeft;
        m_facingLeft = true;
        if (m_isRunningLeft) {
            m_posX -= MOVE_SPEED * 2.0f * deltaTime;
        } else {
            m_posX -= MOVE_SPEED * deltaTime;
        }
    } else if (keyStates[m_inputConfig.sitKey]) {
        m_isSitting = true;
        m_state = CharState::Idle;
    } else {
        m_isSitting = false;
        if (!m_isJumping && !keyStates[m_inputConfig.moveLeftKey] && !keyStates[m_inputConfig.moveRightKey]) {
            m_state = CharState::Idle;
        }
    }
}

void CharacterMovement::HandleJump(float deltaTime, const bool* keyStates) {
    if (!keyStates) {
        std::cout << "Warning: keyStates is null in HandleJump" << std::endl;
        return;
    }
    
    if (keyStates[m_inputConfig.jumpKey]) {
        if (!m_isJumping) {
            m_isSitting = false;
            
            m_isJumping = true;
            m_jumpVelocity = JUMP_FORCE;
            m_jumpStartY = m_posY;
        }
    }
    
    if (m_isJumping) {
        m_jumpVelocity -= GRAVITY * deltaTime;
        m_posY += m_jumpVelocity * deltaTime;
        
        bool isMovingLeft = keyStates[m_inputConfig.moveLeftKey];
        bool isMovingRight = keyStates[m_inputConfig.moveRightKey];
        
        if (isMovingLeft) {
            m_facingLeft = true;
            m_state = CharState::MoveLeft;
            if (m_isRunningLeft) {
                m_posX -= MOVE_SPEED * 1.5f * deltaTime;
            } else {
                m_posX -= MOVE_SPEED * 0.8f * deltaTime;
            }
        } else if (isMovingRight) {
            m_facingLeft = false;
            m_state = CharState::MoveRight;
            if (m_isRunningRight) {
                m_posX += MOVE_SPEED * 1.5f * deltaTime;
            } else {
                m_posX += MOVE_SPEED * 0.8f * deltaTime;
            }
        }
        
        if (m_posY <= m_groundY) {
            m_posY = m_groundY;
            m_isJumping = false;
            m_jumpVelocity = 0.0f;
        }
    }
}

void CharacterMovement::SetPosition(float x, float y) {
    m_posX = x;
    m_posY = y;
}

void CharacterMovement::TriggerDie(bool attackerFacingLeft) {
    if (!m_isDying && !m_isDead) {
        m_isDying = true;
        m_isDead = false;
        m_dieTimer = 0.0f;
        m_knockdownTimer = 0.0f;
        m_knockdownComplete = false;
        m_attackerFacingLeft = attackerFacingLeft;
        m_state = CharState::Die;
        std::cout << "Character triggered die animation, attacker facing: " << (attackerFacingLeft ? "LEFT" : "RIGHT") << std::endl;
    }
}

void CharacterMovement::HandleDie(float deltaTime) {
    m_dieTimer += deltaTime;
    
    if (!m_knockdownComplete && m_dieTimer < 0.8f) {
        m_knockdownTimer += deltaTime;
        
        float knockdownProgress = m_knockdownTimer / 0.8f;
        float arcHeight = 0.15f * sin(knockdownProgress * 3.14159f);
        
        // Tính toán hướng bật ngược dựa trên hướng của người tấn công
        float backwardMovement = m_attackerFacingLeft ? -0.8f * knockdownProgress : 0.8f * knockdownProgress;
        
        m_posY = m_groundY + arcHeight;
        m_posX += backwardMovement * deltaTime * 1.0f;
        
        if (m_knockdownTimer >= 0.8f) {
            m_knockdownComplete = true;
            m_posY = m_groundY;
            std::cout << "Knockdown complete, switching to Lie animation" << std::endl;
        }
    }
    else if (m_knockdownComplete && m_dieTimer < 2.8f) {
        m_posY = m_groundY;
    }
    else if (m_dieTimer >= 2.8f) {
        m_isDying = false;
        m_isDead = false;
        m_state = CharState::Idle;
        std::cout << "Die sequence complete, returning to idle" << std::endl;
    }
}

void CharacterMovement::HandleLanding(const bool* keyStates) {
    if (!keyStates) {
        return;
    }
    
    if (keyStates[m_inputConfig.moveLeftKey]) {
        m_state = CharState::MoveLeft;
        m_facingLeft = true;
    } else if (keyStates[m_inputConfig.moveRightKey]) {
        m_state = CharState::MoveRight;
        m_facingLeft = false;
    } else {
        m_state = CharState::Idle;
    }
} 