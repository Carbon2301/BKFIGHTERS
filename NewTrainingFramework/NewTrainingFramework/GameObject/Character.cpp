#include "stdafx.h"
#include "Character.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "InputManager.h"
#include "Object.h"
#include <iostream>

const float Character::JUMP_FORCE = 3.0f;
const float Character::GRAVITY = 8.0f;
const float Character::GROUND_Y = 0.0f;
const float Character::MOVE_SPEED = 0.5f;
const float Character::COMBO_WINDOW = 0.5f;

// Static input configurations
const PlayerInputConfig Character::PLAYER1_INPUT('A', 'D', 'W', 'S', ' ', 'J', 'L', 'K');
const PlayerInputConfig Character::PLAYER2_INPUT(0x25, 0x27, 0x26, 0x28, '0', '1', '3', '2');

Character::Character() 
    : m_posX(0.0f), m_posY(0.0f), m_facingLeft(false), m_state(CharState::Idle),
      m_isJumping(false), m_jumpVelocity(0.0f), m_jumpStartY(0.0f),
      m_lastAnimation(-1), m_objectId(1000), m_comboCount(0), m_comboTimer(0.0f),
      m_isInCombo(false), m_comboCompleted(false),
      m_axeComboCount(0), m_axeComboTimer(0.0f),
      m_isInAxeCombo(false), m_axeComboCompleted(false),
      m_isKicking(false), m_isSitting(false),
      m_inputConfig(PLAYER1_INPUT) {
}

Character::~Character() {
}

void Character::Initialize(std::shared_ptr<AnimationManager> animManager, int objectId) {
    m_animManager = animManager;
    m_objectId = objectId;
    m_state = CharState::Idle;
    m_facingLeft = false;
    
    m_characterObject = std::make_unique<Object>(objectId);
    
    Object* originalObj = SceneManager::GetInstance()->GetObject(objectId);
    if (originalObj) {
        m_characterObject->SetModel(originalObj->GetModelId());
        const std::vector<int>& textureIds = originalObj->GetTextureIds();
        if (!textureIds.empty()) {
            m_characterObject->SetTexture(textureIds[0], 0);
        }
        m_characterObject->SetShader(originalObj->GetShaderId());
        m_characterObject->SetScale(originalObj->GetScale());
        
        const Vector3& originalPos = originalObj->GetPosition();
        m_posX = originalPos.x;
        m_posY = originalPos.y;
    } else {
        m_posX = 0.0f;
        m_posY = 0.0f;
    }
    
    if (m_animManager) {
        m_animManager->Play(0, true);
    }
}

void Character::ProcessInput(float deltaTime, InputManager* inputManager) {
    if (!inputManager) return;
    
    const bool* keyStates = inputManager->GetKeyStates();
    
    HandleMovement(deltaTime, keyStates);
    HandleJump(deltaTime, keyStates);
    
    if (inputManager->IsKeyJustPressed(m_inputConfig.punchKey)) {
        HandlePunchCombo();
    }
    
    if (inputManager->IsKeyJustPressed(m_inputConfig.axeKey)) {
        HandleAxeCombo();
    }
    
    if (inputManager->IsKeyJustPressed(m_inputConfig.kickKey)) {
        HandleKick();
    }
}

void Character::Update(float deltaTime) {
    if (m_animManager) {
        m_animManager->Update(deltaTime);
        UpdateAnimationState();
    }
    
    UpdateComboTimers(deltaTime);
}

void Character::Draw(Camera* camera) {
    if (m_characterObject && m_animManager && m_characterObject->GetModelId() >= 0 && m_characterObject->GetModelPtr()) {
        float u0, v0, u1, v1;
        m_animManager->GetUV(u0, v0, u1, v1);
        
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
    bool isMoving = (keyStates[m_inputConfig.moveLeftKey] || keyStates[m_inputConfig.moveRightKey]);
    bool isOtherAction = (keyStates[m_inputConfig.sitKey] || keyStates[m_inputConfig.rollKey] || keyStates[m_inputConfig.jumpKey]);
    
    CancelCombosOnOtherAction(keyStates);
    
    if (keyStates[m_inputConfig.moveLeftKey] && keyStates[m_inputConfig.rollKey]) {
        m_state = CharState::MoveLeft;
        m_facingLeft = true;
        m_posX -= MOVE_SPEED * 1.5f * deltaTime;
        PlayAnimation(4, true);
    } else if (keyStates[m_inputConfig.moveRightKey] && keyStates[m_inputConfig.rollKey]) {
        m_state = CharState::MoveRight;
        m_facingLeft = false;
        m_posX += MOVE_SPEED * 1.5f * deltaTime;
        PlayAnimation(4, true);
    } else if (keyStates[m_inputConfig.rollKey]) {
        if (m_facingLeft) {
            m_posX -= MOVE_SPEED * 1.5f * deltaTime;
        } else {
            m_posX += MOVE_SPEED * 1.5f * deltaTime;
        }
        PlayAnimation(4, true);
    } else if (keyStates[m_inputConfig.moveRightKey]) {
        m_state = CharState::MoveRight;
        m_facingLeft = false;
        
        if (isShiftPressed) {
            m_posX += MOVE_SPEED * 2.0f * deltaTime;
        } else {
            m_posX += MOVE_SPEED * deltaTime;
        }
        
        if (!m_isInCombo && !m_isInAxeCombo && !m_isJumping && !m_isKicking) {
            if (isShiftPressed) {
                PlayAnimation(2, true);
            } else {
                PlayAnimation(1, true);
            }
        }
    } else if (keyStates[m_inputConfig.moveLeftKey]) {
        m_state = CharState::MoveLeft;
        m_facingLeft = true;
        
        if (isShiftPressed) {
            m_posX -= MOVE_SPEED * 2.0f * deltaTime;
        } else {
            m_posX -= MOVE_SPEED * deltaTime;
        }
        
        if (!m_isInCombo && !m_isInAxeCombo && !m_isJumping && !m_isKicking) {
            if (isShiftPressed) {
                PlayAnimation(2, true);
            } else {
                PlayAnimation(1, true);
            }
        }
    } else if (keyStates[m_inputConfig.sitKey]) {
        m_isSitting = true;
        m_state = CharState::Idle;
        PlayAnimation(3, true);
    } else {
        m_isSitting = false;
        
        if (!m_isInCombo && !m_isInAxeCombo && !m_isJumping && !m_isKicking) {
            m_state = CharState::Idle;
            PlayAnimation(0, true);
        }
    }
}

void Character::HandleJump(float deltaTime, const bool* keyStates) {
    if (keyStates[m_inputConfig.jumpKey]) {
        if (!m_isJumping) {
            m_isSitting = false;
            
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
        
        bool isMovingLeft = keyStates[m_inputConfig.moveLeftKey];
        bool isMovingRight = keyStates[m_inputConfig.moveRightKey];
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
            
            bool isMovingLeft = keyStates[m_inputConfig.moveLeftKey];
            bool isMovingRight = keyStates[m_inputConfig.moveRightKey];
            bool isShiftPressed = keyStates[16];
            
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

void Character::CancelCombosOnOtherAction(const bool* keyStates) {
    bool isOtherAction = (keyStates[m_inputConfig.sitKey] || keyStates[m_inputConfig.rollKey] || keyStates[m_inputConfig.jumpKey]);
    
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
}

void Character::UpdateComboTimers(float deltaTime) {
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

void Character::UpdateAnimationState() {
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
    
    if (m_isKicking && m_animManager->GetCurrentAnimation() == 18 && !m_animManager->IsPlaying()) {
        m_isKicking = false;
        m_animManager->Play(0, true);
        std::cout << "Animation đá kết thúc - trở về trạng thái idle" << std::endl;
    }
    
    if (!m_animManager->IsPlaying() && !m_isInCombo && !m_isInAxeCombo && !m_isKicking && !m_isJumping && !m_isSitting) {
        m_animManager->Play(0, true);
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
            m_comboCount = 3;
            m_comboCompleted = true;
            std::cout << "=== COMBO OVERFLOW PREVENTED ===" << std::endl;
        }
    } else {
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
            m_axeComboCount = 3;
            m_axeComboCompleted = true;
            std::cout << "=== AXE COMBO OVERFLOW PREVENTED ===" << std::endl;
        }
    } else {
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
        bool allowReplay = (animIndex == 18) ||
                          (animIndex >= 9 && animIndex <= 11) ||
                          (animIndex >= 19 && animIndex <= 21);
        
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