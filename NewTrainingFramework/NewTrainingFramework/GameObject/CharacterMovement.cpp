#include "stdafx.h"
#include "CharacterMovement.h"
#include "WallCollision.h"
#include "PlatformCollision.h"
#include <iostream>
#include <SDL.h>

const float CharacterMovement::JUMP_FORCE = 0.9f;
const float CharacterMovement::GRAVITY = 2.5f;
const float CharacterMovement::GROUND_Y = 0.0f;
const float CharacterMovement::MOVE_SPEED = 0.25f;

const PlayerInputConfig CharacterMovement::PLAYER1_INPUT('A', 'D', 'W', 'S', ' ', 'J', 'L', 'K', 0, 'A', 'S', 'S', 'D');
const PlayerInputConfig CharacterMovement::PLAYER2_INPUT(0x25, 0x27, 0x26, 0x28, '0', '1', '3', '2', 0, 0x25, 0x28, 0x27, 0x28);

CharacterMovement::CharacterMovement() 
    : m_posX(0.0f), m_posY(0.0f), m_facingLeft(false), m_state(CharState::Idle),
      m_isJumping(false), m_jumpVelocity(0.0f), m_jumpStartY(0.0f), m_wasJumping(false),
      m_isSitting(false), m_isDying(false), m_isDead(false), m_dieTimer(0.0f), 
      m_knockdownTimer(0.0f), m_knockdownComplete(false), m_attackerFacingLeft(false), m_inputConfig(PLAYER1_INPUT),
      m_characterWidth(0.1f), m_characterHeight(0.2f), m_isOnPlatform(false), m_currentPlatformY(0.0f),
      m_wallCollision(std::make_unique<WallCollision>()),
      m_platformCollision(std::make_unique<PlatformCollision>()) {
}

CharacterMovement::CharacterMovement(const PlayerInputConfig& inputConfig)
    : m_posX(0.0f), m_posY(0.0f), m_facingLeft(false), m_state(CharState::Idle),
      m_isJumping(false), m_jumpVelocity(0.0f), m_jumpStartY(0.0f), m_wasJumping(false),
      m_isSitting(false), m_isDying(false), m_isDead(false), m_dieTimer(0.0f), 
      m_knockdownTimer(0.0f), m_knockdownComplete(false), m_attackerFacingLeft(false), m_inputConfig(inputConfig),
      m_characterWidth(0.1f), m_characterHeight(0.2f), m_isOnPlatform(false), m_currentPlatformY(0.0f),
      m_wallCollision(std::make_unique<WallCollision>()),
      m_platformCollision(std::make_unique<PlatformCollision>()) {
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
    m_isOnPlatform = false;
    m_currentPlatformY = groundY;
}

void CharacterMovement::Update(float deltaTime, const bool* keyStates) {
    if (!keyStates) {
        return;
    }
    
    m_wasJumping = m_isJumping;
    
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
        return;
    }
    
    bool isRollingLeft = (keyStates[m_inputConfig.rollLeftKey1] && keyStates[m_inputConfig.rollLeftKey2]);
    bool isRollingRight = (keyStates[m_inputConfig.rollRightKey1] && keyStates[m_inputConfig.rollRightKey2]);
    
    bool isOtherAction = (keyStates[m_inputConfig.sitKey] || keyStates[m_inputConfig.jumpKey]);
    
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

    if (!m_isJumping) {
        if (isRollingLeft) {
            m_state = CharState::MoveLeft;
            m_facingLeft = true;
            m_posX -= MOVE_SPEED * 1.5f * deltaTime;
        } else if (isRollingRight) {
            m_state = CharState::MoveRight;
            m_facingLeft = false;
            m_posX += MOVE_SPEED * 1.5f * deltaTime;
        } else if (!isOtherAction) {
            if (keyStates[m_inputConfig.moveLeftKey]) {
                m_facingLeft = true;
                m_state = CharState::MoveLeft;
                if (m_isRunningLeft) {
                    m_posX -= MOVE_SPEED * 1.5f * deltaTime;
                } else {
                    m_posX -= MOVE_SPEED * 0.8f * deltaTime;
                }
            } else if (keyStates[m_inputConfig.moveRightKey]) {
                m_facingLeft = false;
                m_state = CharState::MoveRight;
                if (m_isRunningRight) {
                    m_posX += MOVE_SPEED * 1.5f * deltaTime;
                } else {
                    m_posX += MOVE_SPEED * 0.8f * deltaTime;
                }
            } else if (keyStates[m_inputConfig.sitKey]) {
                m_isSitting = true;
                m_state = CharState::Idle;
            } else {
                m_isSitting = false;
                m_state = CharState::Idle;
            }
        }
    }
}

void CharacterMovement::HandleJump(float deltaTime, const bool* keyStates) {
    if (!keyStates) {
        return;
    }
    
    if (keyStates[m_inputConfig.jumpKey]) {
        if (!m_isJumping && (m_posY <= m_groundY + 0.01f || m_isOnPlatform)) {
            m_isSitting = false;
            
            m_isJumping = true;
            m_jumpVelocity = JUMP_FORCE;
            m_jumpStartY = m_posY;
            m_isOnPlatform = false;
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
        
        float newY = m_posY;
        if (CheckPlatformCollision(newY)) {
            m_posY = newY;
            m_isJumping = false;
            m_jumpVelocity = 0.0f;
            m_isOnPlatform = true;
            m_currentPlatformY = newY;
        }
        else if (m_posY <= m_groundY) {
            m_posY = m_groundY;
            m_isJumping = false;
            m_jumpVelocity = 0.0f;
            m_isOnPlatform = false;
            m_currentPlatformY = m_groundY;
        }
    }
    
    if (!m_isJumping && m_isOnPlatform) {
        float newY = m_posY;
        if (!CheckPlatformCollision(newY)) {
            m_isOnPlatform = false;
            m_isJumping = true;
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
    }
}

void CharacterMovement::HandleDie(float deltaTime) {
    m_dieTimer += deltaTime;
    
    if (!m_knockdownComplete && m_dieTimer < 0.8f) {
        m_knockdownTimer += deltaTime;
        
        float knockdownProgress = m_knockdownTimer / 0.8f;
        float arcHeight = 0.15f * sin(knockdownProgress * 3.14159f);
        
        float backwardMovement = m_attackerFacingLeft ? -0.8f * knockdownProgress : 0.8f * knockdownProgress;
        
        m_posY = m_groundY + arcHeight;
        m_posX += backwardMovement * deltaTime * 1.0f;
        
        if (m_knockdownTimer >= 0.8f) {
            m_knockdownComplete = true;
            m_posY = m_groundY;
        }
    }
    else if (m_knockdownComplete && m_dieTimer < 2.8f) {
        m_posY = m_groundY;
    }
    else if (m_dieTimer >= 2.8f) {
        m_isDying = false;
        m_isDead = false;
        m_state = CharState::Idle;
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

void CharacterMovement::AddPlatform(float x, float y, float width, float height) {
    if (m_platformCollision) {
        m_platformCollision->AddPlatform(x, y, width, height);
    }
}

void CharacterMovement::ClearPlatforms() {
    if (m_platformCollision) {
        m_platformCollision->ClearPlatforms();
    }
}

void CharacterMovement::SetCharacterSize(float width, float height) {
    m_characterWidth = width;
    m_characterHeight = height;
}

bool CharacterMovement::CheckPlatformCollision(float& newY) {
    if (!m_platformCollision) return false;
    return m_platformCollision->CheckPlatformCollision(newY, m_posX, m_posY, m_jumpVelocity, m_characterWidth, m_characterHeight);
}

bool CharacterMovement::CheckPlatformCollisionWithHurtbox(float& newY, float hurtboxWidth, float hurtboxHeight, float hurtboxOffsetX, float hurtboxOffsetY) {
    if (!m_platformCollision) return false;
    return m_platformCollision->CheckPlatformCollisionWithHurtbox(newY, m_posX, m_posY, m_jumpVelocity, hurtboxWidth, hurtboxHeight, hurtboxOffsetX, hurtboxOffsetY);
}

void CharacterMovement::UpdateWithHurtbox(float deltaTime, const bool* keyStates, float hurtboxWidth, float hurtboxHeight, float hurtboxOffsetX, float hurtboxOffsetY) {
    if (!keyStates) return;
    
    if (m_isDying || m_isDead) {
        HandleDie(deltaTime);
        return;
    }
    
    Vector3 currentPos(m_posX, m_posY, 0.0f);
    
    HandleMovement(deltaTime, keyStates);
    HandleJumpWithHurtbox(deltaTime, keyStates, hurtboxWidth, hurtboxHeight, hurtboxOffsetX, hurtboxOffsetY);
    HandleLandingWithHurtbox(keyStates, hurtboxWidth, hurtboxHeight, hurtboxOffsetX, hurtboxOffsetY);
    
    if (m_wallCollision) {
        Vector3 newPos(m_posX, m_posY, 0.0f);
        Vector3 resolvedPos = m_wallCollision->ResolveWallCollision(currentPos, newPos,
                                                                   hurtboxWidth, hurtboxHeight,
                                                                   hurtboxOffsetX, hurtboxOffsetY);
        
        if (resolvedPos.x != newPos.x || resolvedPos.y != newPos.y) {
            m_posX = resolvedPos.x;
            m_posY = resolvedPos.y;
            
            if (resolvedPos.y != newPos.y && resolvedPos.y > newPos.y) {
                m_isJumping = false;
                m_jumpVelocity = 0.0f;
                if (!keyStates[m_inputConfig.moveLeftKey] && !keyStates[m_inputConfig.moveRightKey]) {
                    m_state = CharState::Idle;
                }
            }
            
            if (resolvedPos.x != newPos.x) {
                if (!keyStates[m_inputConfig.moveLeftKey] && !keyStates[m_inputConfig.moveRightKey]) {
                    m_state = CharState::Idle;
                }
            }
        }
    }
}

void CharacterMovement::HandleJumpWithHurtbox(float deltaTime, const bool* keyStates, float hurtboxWidth, float hurtboxHeight, float hurtboxOffsetX, float hurtboxOffsetY) {
    const PlayerInputConfig& inputConfig = GetInputConfig();
    
    bool onGround = (m_posY <= m_groundY + 0.01f);
    bool onWallSupport = false;
    
    if (m_wallCollision) {
        Vector3 testPos(m_posX, m_posY - 0.01f, 0.0f);
        onWallSupport = m_wallCollision->CheckWallCollision(testPos, hurtboxWidth, hurtboxHeight, 
                                                          hurtboxOffsetX, hurtboxOffsetY);
    }
    
    if (!m_isJumping && (onGround || m_isOnPlatform || onWallSupport)) {
        if (keyStates[inputConfig.jumpKey]) {
            m_isJumping = true;
            m_jumpVelocity = JUMP_FORCE;
            m_jumpStartY = m_posY;
            m_wasJumping = false;
            m_state = CharState::Idle;
            m_isOnPlatform = false;
        }
    }
    
    if (m_isJumping) {
        m_jumpVelocity -= GRAVITY * deltaTime;
        m_posY += m_jumpVelocity * deltaTime;
        
        if (keyStates[inputConfig.moveLeftKey]) {
            m_posX -= MOVE_SPEED * 1.5f * deltaTime;
            m_facingLeft = true;
            m_state = CharState::MoveLeft;
        }
        
        if (keyStates[inputConfig.moveRightKey]) {
            m_posX += MOVE_SPEED * 1.5f * deltaTime;
            m_facingLeft = false;
            m_state = CharState::MoveRight;
        }
        
        float newY = m_posY;
        if (CheckPlatformCollisionWithHurtbox(newY, hurtboxWidth, hurtboxHeight, hurtboxOffsetX, hurtboxOffsetY)) {
            m_posY = newY;
            m_isJumping = false;
            m_jumpVelocity = 0.0f;
            m_wasJumping = true;
            m_isOnPlatform = true;
            m_currentPlatformY = newY;
            
            if (!keyStates[inputConfig.moveLeftKey] && !keyStates[inputConfig.moveRightKey]) {
                m_state = CharState::Idle;
            }
        } else if (m_posY <= m_groundY) {
            m_posY = m_groundY;
            m_isJumping = false;
            m_jumpVelocity = 0.0f;
            m_wasJumping = true;
            m_isOnPlatform = false;
            m_currentPlatformY = m_groundY;
            
            if (!keyStates[inputConfig.moveLeftKey] && !keyStates[inputConfig.moveRightKey]) {
                m_state = CharState::Idle;
            }
        }
    }
}

void CharacterMovement::HandleLandingWithHurtbox(const bool* keyStates, float hurtboxWidth, float hurtboxHeight, float hurtboxOffsetX, float hurtboxOffsetY) {
    if (!m_isJumping) {
        float newY = m_posY;
        bool onPlatform = CheckPlatformCollisionWithHurtbox(newY, hurtboxWidth, hurtboxHeight, hurtboxOffsetX, hurtboxOffsetY);
        bool onWall = false;
        if (m_wallCollision) {
            Vector3 testPos(m_posX, m_posY - 0.01f, 0.0f);
            onWall = m_wallCollision->CheckWallCollision(testPos, hurtboxWidth, hurtboxHeight, 
                                                        hurtboxOffsetX, hurtboxOffsetY);
        }
        if (!onPlatform && !onWall) {
            m_isOnPlatform = false;
            if (m_posY > m_groundY + 0.01f) {
                m_isJumping = true;
                m_jumpVelocity = 0.0f;
            }
        } else if (onPlatform) {
            m_posY = newY;
            m_isOnPlatform = true;
        }
    }
}

void CharacterMovement::InitializeWallCollision() {
    if (m_wallCollision) {
        m_wallCollision->LoadWallsFromScene();
    }
} 