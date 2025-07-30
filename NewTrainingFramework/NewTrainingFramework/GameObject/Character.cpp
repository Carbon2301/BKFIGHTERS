#include "stdafx.h"
#include "Character.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include <iostream>

// Constants
static const int ANIM_OBJECT_ID = 1000;

// Static constants
const float Character::JUMP_FORCE = 3.0f;
const float Character::GRAVITY = 8.0f;
const float Character::GROUND_Y = 0.0f;
const float Character::MOVE_SPEED = 0.5f;
const float Character::COMBO_WINDOW = 0.5f;

Character::Character() 
    : m_posX(0.0f), m_posY(0.0f), m_facingLeft(false), m_state(CharState::Idle),
      m_isJumping(false), m_jumpVelocity(0.0f), m_jumpStartY(0.0f),
      m_lastAnimation(-1), m_comboCount(0), m_comboTimer(0.0f),
      m_isInCombo(false), m_comboCompleted(false),
      m_axeComboCount(0), m_axeComboTimer(0.0f),
      m_isInAxeCombo(false), m_axeComboCompleted(false) {
}

Character::~Character() {
}

void Character::Initialize(std::shared_ptr<AnimationManager> animManager) {
    m_animManager = animManager;
    m_posX = 0.0f;
    m_posY = 0.0f;
    m_state = CharState::Idle;
    m_facingLeft = false;
    
    if (m_animManager) {
        m_animManager->Play(0, true); // Start with idle animation
    }
}

void Character::Update(float deltaTime) {
    // Update animation
    if (m_animManager) {
        m_animManager->Update(deltaTime);
        
        // Check if combo animation finished
        if (m_isInCombo && !m_animManager->IsPlaying()) {
            if (m_comboCompleted) {
                m_isInCombo = false;
                m_comboCount = 0;
                m_comboCompleted = false;
                m_animManager->Play(0, true);
                std::cout << "Combo completed - returning to idle" << std::endl;
            } else if (m_comboTimer <= 0.0f) {
                m_isInCombo = false;
                m_comboCount = 0;
                m_comboCompleted = false;
                m_animManager->Play(0, true);
                std::cout << "Combo timeout - returning to idle" << std::endl;
            }
        }
        
        // Check if axe combo animation finished
        if (m_isInAxeCombo && !m_animManager->IsPlaying()) {
            if (m_axeComboCompleted) {
                m_isInAxeCombo = false;
                m_axeComboCount = 0;
                m_axeComboCompleted = false;
                m_animManager->Play(0, true);
                std::cout << "Axe combo completed - returning to idle" << std::endl;
            } else if (m_axeComboTimer <= 0.0f) {
                m_isInAxeCombo = false;
                m_axeComboCount = 0;
                m_axeComboCompleted = false;
                m_animManager->Play(0, true);
                std::cout << "Axe combo timeout - returning to idle" << std::endl;
            }
        }
        
        // Check if kick animation finished
        if (!m_isInCombo && !m_isInAxeCombo && m_animManager->GetCurrentAnimation() == 18 && !m_animManager->IsPlaying()) {
            m_animManager->Play(0, true);
            std::cout << "Animation đá kết thúc - trở về trạng thái idle" << std::endl;
        }
    }
    
    // Update combo timers
    if (m_isInCombo) {
        m_comboTimer -= deltaTime;
        if (m_comboTimer <= 0.0f) {
            m_isInCombo = false;
            m_comboCount = 0;
            m_comboCompleted = false;
            if (m_animManager) {
                m_animManager->Play(0, true);
            }
            std::cout << "Combo timeout - returning to idle" << std::endl;
        }
    }
    
    if (m_isInAxeCombo) {
        m_axeComboTimer -= deltaTime;
        if (m_axeComboTimer <= 0.0f) {
            m_isInAxeCombo = false;
            m_axeComboCount = 0;
            m_axeComboCompleted = false;
            if (m_animManager) {
                m_animManager->Play(0, true);
            }
            std::cout << "Axe combo timeout - returning to idle" << std::endl;
        }
    }
}

void Character::Draw(Camera* camera) {
    Object* animObj = SceneManager::GetInstance()->GetObject(ANIM_OBJECT_ID);
    if (animObj && m_animManager && animObj->GetModelId() >= 0 && animObj->GetModelPtr()) {
        float u0, v0, u1, v1;
        m_animManager->GetUV(u0, v0, u1, v1);
        
        // Đảo ngược UV coordinates khi nhìn sang trái
        if (m_facingLeft) {
            float temp = u0;
            u0 = u1;
            u1 = temp;
        }
        
        animObj->SetCustomUV(u0, v0, u1, v1);
        animObj->SetPosition(Vector3(m_posX, m_posY, 0.0f));
        
        if (camera) {
            animObj->Draw(camera->GetViewMatrix(), camera->GetProjectionMatrix());
        }
    }
}

void Character::HandleMovement(float deltaTime, const bool* keyStates) {
    bool isShiftPressed = keyStates[16];
    bool isMoving = (keyStates['A'] || keyStates['a'] || keyStates['D'] || keyStates['d']);
    bool isOtherAction = (keyStates['S'] || keyStates['s'] || keyStates[' '] || keyStates['W'] || keyStates['w']);
    
    // Reset combo chỉ khi thực hiện hành động khác
    if (isOtherAction && m_isInCombo) {
        m_isInCombo = false;
        m_comboCount = 0;
        m_comboTimer = 0.0f;
        m_comboCompleted = false;
        std::cout << "Combo cancelled due to other action (Sit/Roll/Jump)" << std::endl;
    }
    
    if (isOtherAction && m_isInAxeCombo) {
        m_isInAxeCombo = false;
        m_axeComboCount = 0;
        m_axeComboTimer = 0.0f;
        m_axeComboCompleted = false;
        std::cout << "Axe combo cancelled due to other action (Sit/Roll/Jump)" << std::endl;
    }
    
    // Handle Roll
    if ((keyStates['A'] || keyStates['a']) && (keyStates[' '])) {
        m_state = CharState::MoveLeft;
        m_facingLeft = true;
        m_posX -= MOVE_SPEED * 1.5f * deltaTime;
        PlayAnimation(4, true);
    } else if ((keyStates['D'] || keyStates['d']) && (keyStates[' '])) {
        m_state = CharState::MoveRight;
        m_facingLeft = false;
        m_posX += MOVE_SPEED * 1.5f * deltaTime;
        PlayAnimation(4, true);
    } else if (keyStates[' ']) {
        if (m_facingLeft) {
            m_posX -= MOVE_SPEED * 1.5f * deltaTime;
        } else {
            m_posX += MOVE_SPEED * 1.5f * deltaTime;
        }
        PlayAnimation(4, true);
    } else if (keyStates['D'] || keyStates['d']) {
        m_state = CharState::MoveRight;
        m_facingLeft = false;
        
        if (isShiftPressed) {
            m_posX += MOVE_SPEED * 2.0f * deltaTime;
        } else {
            m_posX += MOVE_SPEED * deltaTime;
        }
        
        if (!m_isInCombo && !m_isInAxeCombo) {
            if (isShiftPressed) {
                PlayAnimation(2, true);
            } else {
                PlayAnimation(1, true);
            }
        }
    } else if (keyStates['A'] || keyStates['a']) {
        m_state = CharState::MoveLeft;
        m_facingLeft = true;
        
        if (isShiftPressed) {
            m_posX -= MOVE_SPEED * 2.0f * deltaTime;
        } else {
            m_posX -= MOVE_SPEED * deltaTime;
        }
        
        if (!m_isInCombo && !m_isInAxeCombo) {
            if (isShiftPressed) {
                PlayAnimation(2, true);
            } else {
                PlayAnimation(1, true);
            }
        }
    } else if (keyStates['S'] || keyStates['s']) {
        m_state = CharState::Idle;
        PlayAnimation(3, true);
    } else {
        if (!m_isInCombo && !m_isInAxeCombo) {
            m_state = CharState::Idle;
            PlayAnimation(0, true);
        }
    }
}

void Character::HandleJump(float deltaTime, const bool* keyStates) {
    if (keyStates['W'] || keyStates['w']) {
        if (!m_isJumping) {
            // KHÔNG cancel combo khi nhảy - combo có thể tiếp tục trong không trung
            m_isJumping = true;
            m_jumpVelocity = JUMP_FORCE;
            m_jumpStartY = m_posY;
            PlayAnimation(15, false);
            std::cout << "=== JUMP START ===" << std::endl;
        }
    }
    
    if (m_isJumping) {
        m_jumpVelocity -= GRAVITY * deltaTime;
        m_posY += m_jumpVelocity * deltaTime;
        
        bool isMovingLeft = (keyStates['A'] || keyStates['a']);
        bool isMovingRight = (keyStates['D'] || keyStates['d']);
        bool isShiftPressed = keyStates[16];
        
        if (isMovingLeft) {
            m_facingLeft = true;
            m_state = CharState::MoveLeft;
            if (isShiftPressed) {
                m_posX -= MOVE_SPEED * 1.5f * deltaTime;
            } else {
                m_posX -= MOVE_SPEED * 0.8f * deltaTime;
            }
        } else if (isMovingRight) {
            m_facingLeft = false;
            m_state = CharState::MoveRight;
            if (isShiftPressed) {
                m_posX += MOVE_SPEED * 1.5f * deltaTime;
            } else {
                m_posX += MOVE_SPEED * 0.8f * deltaTime;
            }
        }
        
                 if (m_posY <= GROUND_Y) {
             m_posY = GROUND_Y;
             m_isJumping = false;
             m_jumpVelocity = 0.0f;
             
             bool isMovingLeft = (keyStates['A'] || keyStates['a']);
             bool isMovingRight = (keyStates['D'] || keyStates['d']);
             bool isShiftPressed = keyStates[16];
             
             // Chỉ thay đổi animation nếu không đang trong combo
             if (!m_isInCombo && !m_isInAxeCombo) {
                 if (isMovingLeft || isMovingRight) {
                     if (isMovingLeft) {
                         m_facingLeft = true;
                         m_state = CharState::MoveLeft;
                     } else if (isMovingRight) {
                         m_facingLeft = false;
                         m_state = CharState::MoveRight;
                     }
                     
                     if (isShiftPressed) {
                         PlayAnimation(2, true);
                     } else {
                         PlayAnimation(1, true);
                     }
                 } else {
                     m_state = CharState::Idle;
                     PlayAnimation(0, true);
                 }
             }
             std::cout << "=== JUMP END ===" << std::endl;
         }
    }
}

void Character::SetPosition(float x, float y) {
    m_posX = x;
    m_posY = y;
}

void Character::HandlePunchCombo() {
    if (!m_isInCombo) {
        m_comboCount = 1;
        m_isInCombo = true;
        m_comboTimer = COMBO_WINDOW;
        PlayAnimation(9, false);
        std::cout << "=== COMBO START ===" << std::endl;
        std::cout << "Combo " << m_comboCount << ": Punch1!" << std::endl;
        std::cout << "Press J again within " << COMBO_WINDOW << " seconds for next punch!" << std::endl;
    } else if (m_comboTimer > 0.0f) {
        m_comboCount++;
        m_comboTimer = COMBO_WINDOW;
        
        if (m_comboCount == 2) {
            PlayAnimation(10, false);
            std::cout << "=== COMBO CONTINUE ===" << std::endl;
            std::cout << "Combo " << m_comboCount << ": Punch2!" << std::endl;
            std::cout << "Press J again within " << COMBO_WINDOW << " seconds for final punch!" << std::endl;
        } else if (m_comboCount == 3) {
            PlayAnimation(11, false);
            std::cout << "=== COMBO FINISH ===" << std::endl;
            std::cout << "Combo " << m_comboCount << ": Punch3! COMBO COMPLETE!" << std::endl;
            m_comboCompleted = true;
        }
    }
}

void Character::HandleAxeCombo() {
    if (!m_isInAxeCombo) {
        m_axeComboCount = 1;
        m_isInAxeCombo = true;
        m_axeComboTimer = COMBO_WINDOW;
        PlayAnimation(19, false);
        std::cout << "=== AXE COMBO START ===" << std::endl;
        std::cout << "Axe Combo " << m_axeComboCount << ": Axe1!" << std::endl;
        std::cout << "Press L again within " << COMBO_WINDOW << " seconds for next axe attack!" << std::endl;
    } else if (m_axeComboTimer > 0.0f) {
        m_axeComboCount++;
        m_axeComboTimer = COMBO_WINDOW;
        
        if (m_axeComboCount == 2) {
            PlayAnimation(20, false);
            std::cout << "=== AXE COMBO CONTINUE ===" << std::endl;
            std::cout << "Axe Combo " << m_axeComboCount << ": Axe2!" << std::endl;
            std::cout << "Press L again within " << COMBO_WINDOW << " seconds for final axe attack!" << std::endl;
        } else if (m_axeComboCount == 3) {
            PlayAnimation(21, false);
            std::cout << "=== AXE COMBO FINISH ===" << std::endl;
            std::cout << "Axe Combo " << m_axeComboCount << ": Axe3! AXE COMBO COMPLETE!" << std::endl;
            m_axeComboCompleted = true;
        }
    }
}

void Character::HandleKick() {
    if (m_isInCombo) {
        m_isInCombo = false;
        m_comboCount = 0;
        m_comboTimer = 0.0f;
        m_comboCompleted = false;
        std::cout << "Combo cancelled due to kick" << std::endl;
    }
    
    if (m_isInAxeCombo) {
        m_isInAxeCombo = false;
        m_axeComboCount = 0;
        m_axeComboTimer = 0.0f;
        m_axeComboCompleted = false;
        std::cout << "Axe combo cancelled due to kick" << std::endl;
    }
    
    PlayAnimation(18, false);
    std::cout << "=== KICK EXECUTED ===" << std::endl;
    std::cout << "Playing Kick animation (Animation 18)" << std::endl;
}

void Character::CancelAllCombos() {
    m_isInCombo = false;
    m_comboCount = 0;
    m_comboTimer = 0.0f;
    m_comboCompleted = false;
    
    m_isInAxeCombo = false;
    m_axeComboCount = 0;
    m_axeComboTimer = 0.0f;
    m_axeComboCompleted = false;
}

void Character::PlayAnimation(int animIndex, bool loop) {
    if (m_animManager && m_lastAnimation != animIndex) {
        m_animManager->Play(animIndex, loop);
        m_lastAnimation = animIndex;
    }
}

int Character::GetCurrentAnimation() const {
    return m_animManager ? m_animManager->GetCurrentAnimation() : -1;
}

bool Character::IsAnimationPlaying() const {
    return m_animManager ? m_animManager->IsPlaying() : false;
} 