#include "stdafx.h"
#include "Character.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include <iostream>



// Static constants
const float Character::JUMP_FORCE = 3.0f;
const float Character::GRAVITY = 8.0f;
const float Character::GROUND_Y = 0.0f;
const float Character::MOVE_SPEED = 0.5f;
const float Character::COMBO_WINDOW = 0.5f;

Character::Character() 
    : m_posX(0.0f), m_posY(0.0f), m_facingLeft(false), m_state(CharState::Idle),
      m_isJumping(false), m_jumpVelocity(0.0f), m_jumpStartY(0.0f),
      m_lastAnimation(-1), m_objectId(1000), m_comboCount(0), m_comboTimer(0.0f),
      m_isInCombo(false), m_comboCompleted(false),
      m_axeComboCount(0), m_axeComboTimer(0.0f),
      m_isInAxeCombo(false), m_axeComboCompleted(false),
      m_isKicking(false),
      m_isSitting(false) {
}

Character::~Character() {
}

void Character::Initialize(std::shared_ptr<AnimationManager> animManager, int objectId) {
    m_animManager = animManager;
    m_objectId = objectId;
    m_state = CharState::Idle;
    m_facingLeft = false;
    
    // Tạo Object riêng cho character này
    m_characterObject = std::make_unique<Object>(objectId);
    
    // Copy settings từ Object gốc trong SceneManager
    Object* originalObj = SceneManager::GetInstance()->GetObject(objectId);
    if (originalObj) {
        m_characterObject->SetModel(originalObj->GetModelId());
        const std::vector<int>& textureIds = originalObj->GetTextureIds();
        if (!textureIds.empty()) {
            m_characterObject->SetTexture(textureIds[0], 0);
        }
        m_characterObject->SetShader(originalObj->GetShaderId());
        m_characterObject->SetScale(originalObj->GetScale());
        
        // Đọc vị trí từ Object gốc thay vì hardcode
        const Vector3& originalPos = originalObj->GetPosition();
        m_posX = originalPos.x;
        m_posY = originalPos.y;
    } else {
        // Fallback nếu không tìm thấy Object gốc
        m_posX = 0.0f;
        m_posY = 0.0f;
    }
    
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
        if (m_isKicking && m_animManager->GetCurrentAnimation() == 18 && !m_animManager->IsPlaying()) {
            m_isKicking = false;
            m_animManager->Play(0, true);
            std::cout << "Animation đá kết thúc - trở về trạng thái idle" << std::endl;
        }
        
        // Force return to idle if no animation is playing and not in any action
        if (!m_animManager->IsPlaying() && !m_isInCombo && !m_isInAxeCombo && !m_isKicking && !m_isJumping && !m_isSitting) {
            m_animManager->Play(0, true);
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
    if (m_characterObject && m_animManager && m_characterObject->GetModelId() >= 0 && m_characterObject->GetModelPtr()) {
        float u0, v0, u1, v1;
        m_animManager->GetUV(u0, v0, u1, v1);
        
        // Đảo ngược UV coordinates khi nhìn sang trái
        if (m_facingLeft) {
            float temp = u0;
            u0 = u1;
            u1 = temp;
        }
        
        m_characterObject->SetCustomUV(u0, v0, u1, v1);
        m_characterObject->SetPosition(Vector3(m_posX, m_posY, 0.0f));
        
        if (camera) {
            m_characterObject->Draw(camera->GetViewMatrix(), camera->GetProjectionMatrix());
        }
    }
}

void Character::HandleMovement(float deltaTime, const bool* keyStates) {
    bool isShiftPressed = keyStates[16];
    bool isMoving = (keyStates['A'] || keyStates['D']); // Only uppercase letters
    bool isOtherAction = (keyStates['S'] || keyStates[' '] || keyStates['W']); // Only uppercase letters
    
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
    
    if (isOtherAction && m_isKicking) {
        m_isKicking = false;
        std::cout << "Kick cancelled due to other action (Sit/Roll/Jump)" << std::endl;
    }
    
    // Handle Roll
    if (keyStates['A'] && keyStates[' ']) { // Only uppercase A
        m_state = CharState::MoveLeft;
        m_facingLeft = true;
        m_posX -= MOVE_SPEED * 1.5f * deltaTime;
        PlayAnimation(4, true);
    } else if (keyStates['D'] && keyStates[' ']) { // Only uppercase D
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
    } else if (keyStates['D']) { // Only uppercase D
        m_state = CharState::MoveRight;
        m_facingLeft = false;
        
        if (isShiftPressed) {
            m_posX += MOVE_SPEED * 2.0f * deltaTime;
        } else {
            m_posX += MOVE_SPEED * deltaTime;
        }
        
        // Chỉ thay đổi animation nếu không đang nhảy, không đang trong combo, và không đang kick
        if (!m_isInCombo && !m_isInAxeCombo && !m_isJumping && !m_isKicking) {
            if (isShiftPressed) {
                PlayAnimation(2, true);
            } else {
                PlayAnimation(1, true);
            }
        }
    } else if (keyStates['A']) { // Only uppercase A
        m_state = CharState::MoveLeft;
        m_facingLeft = true;
        
        if (isShiftPressed) {
            m_posX -= MOVE_SPEED * 2.0f * deltaTime;
        } else {
            m_posX -= MOVE_SPEED * deltaTime;
        }
        
        // Chỉ thay đổi animation nếu không đang nhảy, không đang trong combo, và không đang kick
        if (!m_isInCombo && !m_isInAxeCombo && !m_isJumping && !m_isKicking) {
            if (isShiftPressed) {
                PlayAnimation(2, true);
            } else {
                PlayAnimation(1, true);
            }
        }
    } else if (keyStates['S']) { // Only uppercase S
        m_isSitting = true;
        m_state = CharState::Idle;
        PlayAnimation(3, true);
    } else {
        // Reset sitting state when not pressing S
        m_isSitting = false;
        
        // Chỉ thay đổi animation nếu không đang nhảy, không đang trong combo, và không đang kick
        if (!m_isInCombo && !m_isInAxeCombo && !m_isJumping && !m_isKicking) {
            m_state = CharState::Idle;
            PlayAnimation(0, true);
        }
    }
}

void Character::HandleJump(float deltaTime, const bool* keyStates) {
    if (keyStates['W']) { // Only uppercase W
        if (!m_isJumping) {
            // Reset sitting state when jumping
            m_isSitting = false;
            
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
        
        bool isMovingLeft = keyStates['A']; // Only uppercase A
        bool isMovingRight = keyStates['D']; // Only uppercase D
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
            
            bool isMovingLeft = keyStates['A']; // Only uppercase A
            bool isMovingRight = keyStates['D']; // Only uppercase D
            bool isShiftPressed = keyStates[16];
            
            // Chỉ thay đổi animation nếu không đang trong combo và không đang kick
            if (!m_isInCombo && !m_isInAxeCombo && !m_isKicking) {
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

void Character::HandleMovementPlayer2(float deltaTime, const bool* keyStates) {
    bool isShiftPressed = keyStates[16];
    // Try different key codes for arrow keys
    bool isMoving = (keyStates[0x25] || keyStates[0x27]); // VK_LEFT=0x25, VK_RIGHT=0x27
    bool isOtherAction = (keyStates[0x28] || keyStates[0x26] || keyStates['0']); // VK_DOWN=0x28, VK_UP=0x26, or '0'
    
    // Reset combo chỉ khi thực hiện hành động khác
    if (isOtherAction && m_isInCombo) {
        m_isInCombo = false;
        m_comboCount = 0;
        m_comboTimer = 0.0f;
        m_comboCompleted = false;
        std::cout << "Player 2: Combo cancelled due to other action (Sit/Roll/Jump)" << std::endl;
    }
    
    if (isOtherAction && m_isInAxeCombo) {
        m_isInAxeCombo = false;
        m_axeComboCount = 0;
        m_axeComboTimer = 0.0f;
        m_axeComboCompleted = false;
        std::cout << "Player 2: Axe combo cancelled due to other action (Sit/Roll/Jump)" << std::endl;
    }
    
    if (isOtherAction && m_isKicking) {
        m_isKicking = false;
        std::cout << "Player 2: Kick cancelled due to other action (Sit/Roll/Jump)" << std::endl;
    }
    
    // Handle Roll (using '0' key)
    if (keyStates[0x25] && keyStates['0']) { // Left arrow + '0'
        m_state = CharState::MoveLeft;
        m_facingLeft = true;
        m_posX -= MOVE_SPEED * 1.5f * deltaTime;
        PlayAnimation(4, true);
    } else if (keyStates[0x27] && keyStates['0']) { // Right arrow + '0'
        m_state = CharState::MoveRight;
        m_facingLeft = false;
        m_posX += MOVE_SPEED * 1.5f * deltaTime;
        PlayAnimation(4, true);
    } else if (keyStates['0']) { // Just '0' key
        if (m_facingLeft) {
            m_posX -= MOVE_SPEED * 1.5f * deltaTime;
        } else {
            m_posX += MOVE_SPEED * 1.5f * deltaTime;
        }
        PlayAnimation(4, true);
    } else if (keyStates[0x27]) { // Right arrow
        m_state = CharState::MoveRight;
        m_facingLeft = false;
        
        if (isShiftPressed) {
            m_posX += MOVE_SPEED * 2.0f * deltaTime;
        } else {
            m_posX += MOVE_SPEED * deltaTime;
        }
        
        // Chỉ thay đổi animation nếu không đang nhảy, không đang trong combo, và không đang kick
        if (!m_isInCombo && !m_isInAxeCombo && !m_isJumping && !m_isKicking) {
            if (isShiftPressed) {
                PlayAnimation(2, true);
            } else {
                PlayAnimation(1, true);
            }
        }
    } else if (keyStates[0x25]) { // Left arrow
        m_state = CharState::MoveLeft;
        m_facingLeft = true;
        
        if (isShiftPressed) {
            m_posX -= MOVE_SPEED * 2.0f * deltaTime;
        } else {
            m_posX -= MOVE_SPEED * deltaTime;
        }
        
        // Chỉ thay đổi animation nếu không đang nhảy, không đang trong combo, và không đang kick
        if (!m_isInCombo && !m_isInAxeCombo && !m_isJumping && !m_isKicking) {
            if (isShiftPressed) {
                PlayAnimation(2, true);
            } else {
                PlayAnimation(1, true);
            }
        }
    } else if (keyStates[0x28]) { // Down arrow (sit)
        m_isSitting = true;
        m_state = CharState::Idle;
        PlayAnimation(3, true);
    } else {
        // Reset sitting state when not pressing down arrow
        m_isSitting = false;
        
        // Chỉ thay đổi animation nếu không đang nhảy, không đang trong combo, và không đang kick
        if (!m_isInCombo && !m_isInAxeCombo && !m_isJumping && !m_isKicking) {
            m_state = CharState::Idle;
            PlayAnimation(0, true);
        }
    }
}

void Character::HandleJumpPlayer2(float deltaTime, const bool* keyStates) {
    if (keyStates[0x26]) { // Up arrow
        if (!m_isJumping) {
            // Reset sitting state when jumping
            m_isSitting = false;
            
            // KHÔNG cancel combo khi nhảy - combo có thể tiếp tục trong không trung
            m_isJumping = true;
            m_jumpVelocity = JUMP_FORCE;
            m_jumpStartY = m_posY;
            PlayAnimation(15, false);
            std::cout << "=== PLAYER 2 JUMP START ===" << std::endl;
        }
    }
    
    if (m_isJumping) {
        m_jumpVelocity -= GRAVITY * deltaTime;
        m_posY += m_jumpVelocity * deltaTime;
        
        bool isMovingLeft = keyStates[0x25]; // Left arrow
        bool isMovingRight = keyStates[0x27]; // Right arrow
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
            
            bool isMovingLeft = keyStates[0x25]; // Left arrow
            bool isMovingRight = keyStates[0x27]; // Right arrow
            bool isShiftPressed = keyStates[16];
            
            // Chỉ thay đổi animation nếu không đang trong combo và không đang kick
            if (!m_isInCombo && !m_isInAxeCombo && !m_isKicking) {
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
            std::cout << "=== PLAYER 2 JUMP END ===" << std::endl;
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
        } else if (m_comboCount > 3) {
            // Prevent infinite combo by resetting
            m_comboCount = 3;
            m_comboCompleted = true;
            std::cout << "=== COMBO OVERFLOW PREVENTED ===" << std::endl;
        }
    } else {
        // Combo timer expired, start new combo
        m_comboCount = 1;
        m_isInCombo = true;
        m_comboTimer = COMBO_WINDOW;
        PlayAnimation(9, false);
        std::cout << "=== NEW COMBO START (timer expired) ===" << std::endl;
        std::cout << "Combo " << m_comboCount << ": Punch1!" << std::endl;
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
        } else if (m_axeComboCount > 3) {
            // Prevent infinite combo by resetting
            m_axeComboCount = 3;
            m_axeComboCompleted = true;
            std::cout << "=== AXE COMBO OVERFLOW PREVENTED ===" << std::endl;
        }
    } else {
        // Combo timer expired, start new combo
        m_axeComboCount = 1;
        m_isInAxeCombo = true;
        m_axeComboTimer = COMBO_WINDOW;
        PlayAnimation(19, false);
        std::cout << "=== NEW AXE COMBO START (timer expired) ===" << std::endl;
        std::cout << "Axe Combo " << m_axeComboCount << ": Axe1!" << std::endl;
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
    
    m_isKicking = true;
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
    
    m_isKicking = false;
}

void Character::PlayAnimation(int animIndex, bool loop) {
    if (m_animManager) {
        // Cho phép kick animation (18) và combo animations được phát lại ngay cả khi đang chạy
        bool allowReplay = (animIndex == 18) || // Kick
                          (animIndex >= 9 && animIndex <= 11) || // Punch combo
                          (animIndex >= 19 && animIndex <= 21);  // Axe combo
        
        if (m_lastAnimation != animIndex || allowReplay) {
            m_animManager->Play(animIndex, loop);
            m_lastAnimation = animIndex;
        }
    }
}

int Character::GetCurrentAnimation() const {
    return m_animManager ? m_animManager->GetCurrentAnimation() : -1;
}

bool Character::IsAnimationPlaying() const {
    return m_animManager ? m_animManager->IsPlaying() : false;
} 