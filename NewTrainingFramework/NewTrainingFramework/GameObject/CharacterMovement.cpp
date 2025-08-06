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
      m_knockdownTimer(0.0f), m_knockdownComplete(false), m_attackerFacingLeft(false), m_inputConfig(PLAYER1_INPUT),
      m_characterWidth(0.1f), m_characterHeight(0.2f), m_isOnPlatform(false), m_currentPlatformY(0.0f) {
}

CharacterMovement::CharacterMovement(const PlayerInputConfig& inputConfig)
    : m_posX(0.0f), m_posY(0.0f), m_facingLeft(false), m_state(CharState::Idle),
      m_isJumping(false), m_jumpVelocity(0.0f), m_jumpStartY(0.0f), m_wasJumping(false),
      m_isSitting(false), m_isDying(false), m_isDead(false), m_dieTimer(0.0f), 
      m_knockdownTimer(0.0f), m_knockdownComplete(false), m_attackerFacingLeft(false), m_inputConfig(inputConfig),
      m_characterWidth(0.1f), m_characterHeight(0.2f), m_isOnPlatform(false), m_currentPlatformY(0.0f) {
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
        std::cout << "Warning: keyStates is null in HandleJump" << std::endl;
        return;
    }
    
    // Handle jump
    if (keyStates[m_inputConfig.jumpKey]) {
        if (!m_isJumping && (m_posY <= m_groundY + 0.01f || m_isOnPlatform)) {
            m_isSitting = false;
            
            m_isJumping = true;
            m_jumpVelocity = JUMP_FORCE;
            m_jumpStartY = m_posY;
            m_isOnPlatform = false;
            std::cout << "Character jumped from " << (m_posY <= m_groundY + 0.01f ? "ground" : "platform") << std::endl;
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
        
        // Check platform collision
        float newY = m_posY;
        if (CheckPlatformCollision(newY)) {
            m_posY = newY;
            m_isJumping = false;
            m_jumpVelocity = 0.0f;
            m_isOnPlatform = true;
            m_currentPlatformY = newY;
            std::cout << "Character landed on platform at Y: " << newY << std::endl;
        }
        // Check ground collision
        else if (m_posY <= m_groundY) {
            m_posY = m_groundY;
            m_isJumping = false;
            m_jumpVelocity = 0.0f;
            m_isOnPlatform = false;
            m_currentPlatformY = m_groundY;
            std::cout << "Character landed on ground" << std::endl;
        }
    }
    
    // Check if character fell off platform when not jumping
    if (!m_isJumping && m_isOnPlatform) {
        float newY = m_posY;
        if (!CheckPlatformCollision(newY)) {
            // Character fell off platform, start falling
            m_isOnPlatform = false;
            m_isJumping = true;
            m_jumpVelocity = 0.0f; // Start falling
            std::cout << "Character fell off platform, starting to fall" << std::endl;
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

// Platform collision methods
void CharacterMovement::AddPlatform(float x, float y, float width, float height) {
    m_platforms.emplace_back(x, y, width, height);
}

void CharacterMovement::ClearPlatforms() {
    m_platforms.clear();
}

void CharacterMovement::SetCharacterSize(float width, float height) {
    m_characterWidth = width;
    m_characterHeight = height;
}

bool CharacterMovement::CheckPlatformCollisionWithHurtbox(float& newY, float hurtboxWidth, float hurtboxHeight, float hurtboxOffsetX, float hurtboxOffsetY) {
    // Chỉ kiểm tra collision khi đang rơi xuống
    if (m_jumpVelocity > 0) {
        return false; // Đang nhảy lên, không cần check platform
    }
    
    float hurtboxLeft = m_posX + hurtboxOffsetX - hurtboxWidth * 0.5f;
    float hurtboxRight = m_posX + hurtboxOffsetX + hurtboxWidth * 0.5f;
    float hurtboxBottom = m_posY + hurtboxOffsetY - hurtboxHeight * 0.5f;
    float hurtboxTop = m_posY + hurtboxOffsetY + hurtboxHeight * 0.5f;
    const float epsilon = 0.05f;
    
    for (const auto& platform : m_platforms) {
        float platformLeft = platform.x - platform.width * 0.5f;
        float platformRight = platform.x + platform.width * 0.5f;
        float platformBottom = platform.y - platform.height * 0.5f;
        float platformTop = platform.y + platform.height * 0.5f;
        
        // Chỉ cho phép va chạm khi đang rơi xuống và đáy hurtbox nằm trong khoảng nhỏ quanh đỉnh bục
        if (m_jumpVelocity <= 0 &&
            hurtboxBottom >= platformTop - epsilon &&
            hurtboxBottom <= platformTop + epsilon &&
            hurtboxRight > platformLeft && hurtboxLeft < platformRight) {
            newY = platformTop - hurtboxOffsetY + hurtboxHeight * 0.5f;
            m_isOnPlatform = true;
            m_currentPlatformY = platformTop;
            return true;
        }
    }
    m_isOnPlatform = false;
    return false;
}

void CharacterMovement::UpdateWithHurtbox(float deltaTime, const bool* keyStates, float hurtboxWidth, float hurtboxHeight, float hurtboxOffsetX, float hurtboxOffsetY) {
    if (!keyStates) return;
    
    if (m_isDying || m_isDead) {
        HandleDie(deltaTime);
        return;
    }
    
    HandleMovement(deltaTime, keyStates);
    HandleJumpWithHurtbox(deltaTime, keyStates, hurtboxWidth, hurtboxHeight, hurtboxOffsetX, hurtboxOffsetY);
    HandleLandingWithHurtbox(keyStates, hurtboxWidth, hurtboxHeight, hurtboxOffsetX, hurtboxOffsetY);
}

void CharacterMovement::HandleJumpWithHurtbox(float deltaTime, const bool* keyStates, float hurtboxWidth, float hurtboxHeight, float hurtboxOffsetX, float hurtboxOffsetY) {
    const PlayerInputConfig& inputConfig = GetInputConfig();
    
    if (!m_isJumping && (m_posY <= m_groundY + 0.01f || m_isOnPlatform)) {
        if (keyStates[inputConfig.jumpKey]) {
            m_isJumping = true;
            m_jumpVelocity = JUMP_FORCE;
            m_jumpStartY = m_posY;
            m_wasJumping = false;
            m_state = CharState::Idle;
            
            std::cout << "Character jumped from " << (m_posY <= m_groundY + 0.01f ? "ground" : "platform") << std::endl;
        }
    }
    
    if (m_isJumping) {
        m_jumpVelocity -= GRAVITY * deltaTime;
        m_posY += m_jumpVelocity * deltaTime;
        
        if (keyStates[inputConfig.moveLeftKey]) {
            m_posX -= MOVE_SPEED * 1.5f * deltaTime;
            m_facingLeft = true;
        }
        
        if (keyStates[inputConfig.moveRightKey]) {
            m_posX += MOVE_SPEED * 1.5f * deltaTime;
            m_facingLeft = false;
        }
        
        float newY = m_posY;
        if (CheckPlatformCollisionWithHurtbox(newY, hurtboxWidth, hurtboxHeight, hurtboxOffsetX, hurtboxOffsetY)) {
            m_posY = newY;
            m_isJumping = false;
            m_jumpVelocity = 0.0f;
            m_wasJumping = true;
            
            std::cout << "Character landed on platform at Y: " << m_posY << std::endl;
        } else if (m_posY <= m_groundY) {
            m_posY = m_groundY;
            m_isJumping = false;
            m_jumpVelocity = 0.0f;
            m_wasJumping = true;
            
            std::cout << "Character landed on ground" << std::endl;
        }
    }
}

void CharacterMovement::HandleLandingWithHurtbox(const bool* keyStates, float hurtboxWidth, float hurtboxHeight, float hurtboxOffsetX, float hurtboxOffsetY) {
    if (!m_isJumping) {
        float newY = m_posY;
        if (!CheckPlatformCollisionWithHurtbox(newY, hurtboxWidth, hurtboxHeight, hurtboxOffsetX, hurtboxOffsetY)) {
            if (m_posY > m_groundY + 0.01f && !m_isOnPlatform) {
                m_isJumping = true;
                m_jumpVelocity = 0.0f;
                std::cout << "Character started falling" << std::endl;
            }
        } else {
            m_posY = newY;
        }
    }
}

bool CharacterMovement::CheckPlatformCollision(float& newY) {
    // Chỉ kiểm tra collision khi đang rơi xuống
    if (m_jumpVelocity > 0) {
        return false; // Đang nhảy lên, không cần check platform
    }
    
    float characterLeft = m_posX - m_characterWidth * 0.5f;
    float characterRight = m_posX + m_characterWidth * 0.5f;
    float characterBottom = m_posY;
    float characterTop = m_posY + m_characterHeight;
    const float epsilon = 0.05f; // cho phép một khoảng nhỏ
    
    for (const auto& platform : m_platforms) {
        float platformLeft = platform.x - platform.width * 0.5f;
        float platformRight = platform.x + platform.width * 0.5f;
        float platformBottom = platform.y - platform.height * 0.5f;
        float platformTop = platform.y + platform.height * 0.5f;
        
        // Chỉ cho phép va chạm khi đang rơi xuống và đáy nhân vật nằm trong khoảng nhỏ quanh đỉnh bục
        if (m_jumpVelocity <= 0 &&
            characterBottom >= platformTop - epsilon &&
            characterBottom <= platformTop + epsilon &&
            characterRight > platformLeft && characterLeft < platformRight) {
            newY = platformTop;
            return true;
        }
    }
    
    return false;
} 