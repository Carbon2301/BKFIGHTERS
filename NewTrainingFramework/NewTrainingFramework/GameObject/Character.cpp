#include "stdafx.h"
#include "Character.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "InputManager.h"
#include "Object.h"
#include "Texture2D.h"
#include <GLES3/gl3.h>
#include <iostream>
#include <cstdlib>

const float Character::JUMP_FORCE = 3.0f;
const float Character::GRAVITY = 8.0f;
const float Character::GROUND_Y = 0.0f;
const float Character::MOVE_SPEED = 0.5f;
const float Character::COMBO_WINDOW = 0.5f;
const float Character::HIT_DURATION = 0.3f;
const float Character::HITBOX_DURATION = 0.2f;

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
      m_isKicking(false), m_isSitting(false), m_isHit(false), m_hitTimer(0.0f),
      m_showHitbox(false), m_hitboxTimer(0.0f), m_hitboxWidth(0.0f), m_hitboxHeight(0.0f),
      m_hitboxOffsetX(0.0f), m_hitboxOffsetY(0.0f),
      m_inputConfig(PLAYER1_INPUT) {
    
    // Initialize hitbox object
    m_hitboxObject = std::make_unique<Object>(-1); // Use -1 as ID for hitbox
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
        m_groundY = m_posY;
        
    } else {
        m_posX = 0.0f;
        m_posY = 0.0f;
        m_groundY = 0.0f;
    }
    
    // Setup hitbox object - use the same model as character but with red color
    if (m_hitboxObject && originalObj) {
        m_hitboxObject->SetModel(originalObj->GetModelId());
        m_hitboxObject->SetShader(originalObj->GetShaderId());
        
        // Create a red texture for hitbox
        auto redTexture = std::make_shared<Texture2D>();
        if (redTexture->CreateColorTexture(64, 64, 255, 0, 0, 180)) { // Red with some transparency
            m_hitboxObject->SetDynamicTexture(redTexture);
        }
    }
    
    if (m_animManager) {
        m_animManager->Play(0, true);
    }
    
}

void Character::ProcessInput(float deltaTime, InputManager* inputManager) {
    if (!inputManager) {
        std::cout << "Warning: InputManager is null in ProcessInput" << std::endl;
        return;
    }
    
    const bool* keyStates = inputManager->GetKeyStates();
    if (!keyStates) {
        std::cout << "Warning: keyStates is null in ProcessInput" << std::endl;
        return;
    }
    
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
    
    // Handle random GetHit animation with 'H' key
    if (inputManager->IsKeyJustPressed('H')) {
        HandleRandomGetHit();
    }
}

void Character::Update(float deltaTime) {
    if (m_animManager) {
        m_animManager->Update(deltaTime);
        UpdateAnimationState();
    }
    
    UpdateComboTimers(deltaTime);
    UpdateHitboxTimer(deltaTime);
    
    // Update hit timer
    if (m_isHit) {
        m_hitTimer -= deltaTime;
        if (m_hitTimer <= 0.0f) {
            m_isHit = false;
            m_hitTimer = 0.0f;
            // Return to idle animation when hit animation finishes
            if (!m_isInCombo && !m_isInAxeCombo && !m_isJumping && !m_isKicking) {
                PlayAnimation(0, true);
            }
        }
    }
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
    
    // Draw hitbox if active
    if (IsHitboxActive()) {
        DrawHitbox(camera);
    }
}

void Character::HandleMovement(float deltaTime, const bool* keyStates) {
    if (!keyStates) {
        std::cout << "Warning: keyStates is null in HandleMovement" << std::endl;
        return;
    }
    
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
        
        if (!m_isInCombo && !m_isInAxeCombo && !m_isJumping && !m_isKicking && !m_isHit) {
            m_state = CharState::Idle;
            PlayAnimation(0, true);
        }
    }
}

void Character::HandleJump(float deltaTime, const bool* keyStates) {
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
            PlayAnimation(16, false);
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
        
        if (m_posY <= m_groundY) {
            m_posY = m_groundY;
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
        }
    }
}

void Character::CancelCombosOnOtherAction(const bool* keyStates) {
    if (!keyStates) {
        std::cout << "Warning: keyStates is null in CancelCombosOnOtherAction" << std::endl;
        return;
    }
    
    bool isOtherAction = (keyStates[m_inputConfig.sitKey] || keyStates[m_inputConfig.rollKey] || keyStates[m_inputConfig.jumpKey]);
    
    if (isOtherAction && m_isInCombo) {
        m_isInCombo = false;
        m_comboCount = 0;
        m_comboTimer = 0.0f;
        m_comboCompleted = false;
    }
    
    if (isOtherAction && m_isInAxeCombo) {
        m_isInAxeCombo = false;
        m_axeComboCount = 0;
        m_axeComboTimer = 0.0f;
        m_axeComboCompleted = false;
    }
    
    if (isOtherAction && m_isKicking) {
        m_isKicking = false;
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
        } else if (m_axeComboTimer <= 0.0f) {
            m_isInAxeCombo = false;
            m_axeComboCount = 0;
            m_axeComboCompleted = false;
            m_animManager->Play(0, true);
            std::cout << "Axe combo timeout - returning to idle" << std::endl;
        }
    }
    
    if (m_isKicking && m_animManager->GetCurrentAnimation() == 19 && !m_animManager->IsPlaying()) {
        m_isKicking = false;
        m_animManager->Play(0, true);
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
        PlayAnimation(10, false);
        
        // Show hitbox for punch 1
        float hitboxWidth = 0.08f;
        float hitboxHeight = 0.08f;
        float hitboxOffsetX = m_facingLeft ? -0.1f : 0.1f;
        float hitboxOffsetY = -0.05f;
        ShowHitbox(hitboxWidth, hitboxHeight, hitboxOffsetX, hitboxOffsetY);
        
        std::cout << "=== COMBO START ===" << std::endl;
        std::cout << "Combo " << m_comboCount << ": Punch1!" << std::endl;
        std::cout << "Press J again within " << COMBO_WINDOW << " seconds for next punch!" << std::endl;
    } else if (m_comboTimer > 0.0f) {
        m_comboCount++;
        m_comboTimer = COMBO_WINDOW;
        
        if (m_comboCount == 2) {
            PlayAnimation(11, false);
            
            // Show hitbox for punch 2
            float hitboxWidth = 0.08f;
            float hitboxHeight = 0.08f;
            float hitboxOffsetX = m_facingLeft ? -0.1f : 0.1f;
            float hitboxOffsetY = -0.05f;
            ShowHitbox(hitboxWidth, hitboxHeight, hitboxOffsetX, hitboxOffsetY);
        } else if (m_comboCount == 3) {
            PlayAnimation(12, false);
            
            // Show hitbox for punch 3
            float hitboxWidth = 0.08f;
            float hitboxHeight = 0.08f;
            float hitboxOffsetX = m_facingLeft ? -0.1f : 0.1f;
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
        PlayAnimation(10, false);
        
        // Show hitbox for punch 1
        float hitboxWidth = 0.08f;
        float hitboxHeight = 0.08f;
        float hitboxOffsetX = m_facingLeft ? -0.1f : 0.1f;
        float hitboxOffsetY = -0.05f;
        ShowHitbox(hitboxWidth, hitboxHeight, hitboxOffsetX, hitboxOffsetY);
    }
}

void Character::HandleAxeCombo() {
    if (!m_isInAxeCombo) {
        m_axeComboCount = 1;
        m_isInAxeCombo = true;
        m_axeComboTimer = COMBO_WINDOW;
        PlayAnimation(20, false);
    } else if (m_axeComboTimer > 0.0f) {
        m_axeComboCount++;
        m_axeComboTimer = COMBO_WINDOW;
        
        if (m_axeComboCount == 2) {
            PlayAnimation(21, false);
        } else if (m_axeComboCount == 3) {
            PlayAnimation(22, false);
            m_axeComboCompleted = true;
        } else if (m_axeComboCount > 3) {
            m_axeComboCount = 3;
            m_axeComboCompleted = true;
        }
    } else {
        m_axeComboCount = 1;
        m_isInAxeCombo = true;
        m_axeComboTimer = COMBO_WINDOW;
        PlayAnimation(20, false);
    }
}

void Character::HandleKick() {
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
    PlayAnimation(19, false);
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
    m_isHit = false;
    m_hitTimer = 0.0f;
}

void Character::PlayAnimation(int animIndex, bool loop) {
    if (m_animManager) {
        bool allowReplay = (animIndex == 19) ||
                          (animIndex >= 10 && animIndex <= 12) ||
                          (animIndex >= 20 && animIndex <= 22) ||
                          (animIndex == 8 || animIndex == 9); // Allow replay for GetHit animations
        
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

void Character::HandleRandomGetHit() {
    CancelAllCombos();
    
    int randomHitAnimation = (rand() % 2) + 8; // 8 or 9
    
    m_isHit = true;
    m_hitTimer = HIT_DURATION;
    
    PlayAnimation(randomHitAnimation, false);
}

// Hitbox management methods
void Character::ShowHitbox(float width, float height, float offsetX, float offsetY) {
    m_showHitbox = true;
    m_hitboxTimer = HITBOX_DURATION;
    m_hitboxWidth = width;
    m_hitboxHeight = height;
    m_hitboxOffsetX = offsetX;
    m_hitboxOffsetY = offsetY;
}

void Character::UpdateHitboxTimer(float deltaTime) {
    if (m_showHitbox && m_hitboxTimer > 0.0f) {
        m_hitboxTimer -= deltaTime;
        if (m_hitboxTimer <= 0.0f) {
            m_showHitbox = false;
            m_hitboxTimer = 0.0f;
        }
    }
}

void Character::DrawHitbox(Camera* camera) {
    if (!camera || !m_showHitbox || m_hitboxTimer <= 0.0f || !m_hitboxObject) {
        return;
    }
    
    // Calculate hitbox position based on character position and facing direction
    float hitboxX = m_posX + m_hitboxOffsetX;
    float hitboxY = m_posY + m_hitboxOffsetY;
    
    // Set hitbox object position and scale
    m_hitboxObject->SetPosition(hitboxX, hitboxY, 0.0f);
    m_hitboxObject->SetScale(m_hitboxWidth, m_hitboxHeight, 1.0f);
    
    // Draw hitbox object
    if (camera) {
        m_hitboxObject->Draw(camera->GetViewMatrix(), camera->GetProjectionMatrix());
    }
} 