#include "stdafx.h"
#include "Character.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "InputManager.h"
#include "Object.h"
#include "Texture2D.h"
#include "../GameManager/GSPlay.h"
#include <GLES3/gl3.h>
#include <iostream>
#include <cstdlib>
#include <memory>



Character::Character() 
    : m_movement(std::make_unique<CharacterMovement>(CharacterMovement::PLAYER1_INPUT)),
      m_combat(std::make_unique<CharacterCombat>()),
      m_lastAnimation(-1), m_objectId(1000),
      m_hurtboxWidth(0.0f), m_hurtboxHeight(0.0f), m_hurtboxOffsetX(0.0f), m_hurtboxOffsetY(0.0f) {
    
    // Initialize hitbox object
    m_hitboxObject = std::make_unique<Object>(-1); // Use -1 as ID for hitbox
    
    // Initialize hurtbox object
    m_hurtboxObject = std::make_unique<Object>(-2); // Use -2 as ID for hurtbox
}

Character::~Character() {
}

void Character::Initialize(std::shared_ptr<AnimationManager> animManager, int objectId) {
    m_animManager = animManager;
    m_objectId = objectId;
    
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
        m_movement->Initialize(originalPos.x, originalPos.y, originalPos.y);
        
    } else {
        m_movement->Initialize(0.0f, 0.0f, 0.0f);
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
    
    // Setup hurtbox object
    if (m_hurtboxObject && originalObj) {
        m_hurtboxObject->SetModel(originalObj->GetModelId());
        m_hurtboxObject->SetShader(originalObj->GetShaderId());
        
        // Create a blue texture for hurtbox
        auto blueTexture = std::make_shared<Texture2D>();
        if (blueTexture->CreateColorTexture(64, 64, 0, 0, 255, 180)) {
            m_hurtboxObject->SetDynamicTexture(blueTexture);
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
    
    CancelCombosOnOtherAction(keyStates);
    
    m_movement->Update(deltaTime, keyStates);
    
    // Handle movement animations
    HandleMovementAnimations(keyStates);
    
    const PlayerInputConfig& inputConfig = m_movement->GetInputConfig();
    
    if (inputManager->IsKeyJustPressed(inputConfig.punchKey)) {
        HandlePunchCombo();
    }
    
    if (inputManager->IsKeyJustPressed(inputConfig.axeKey)) {
        HandleAxeCombo();
    }
    
    if (inputManager->IsKeyJustPressed(inputConfig.kickKey)) {
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
    
    if (m_combat) {
        m_combat->Update(deltaTime);
    }
    
    if (!m_combat->IsInCombo() && 
        !m_combat->IsInAxeCombo() && 
        !m_movement->IsJumping() && 
        !m_combat->IsKicking() && 
        !m_combat->IsHit() && 
        !m_movement->IsSitting()) {
        if (m_animManager && !m_animManager->IsPlaying()) {
            PlayAnimation(0, true);
        }
    }
    
    if (m_movement->IsSitting() && m_animManager && !m_animManager->IsPlaying()) {
        PlayAnimation(3, true);
    }
    

}

void Character::Draw(Camera* camera) {
    if (m_characterObject && m_animManager && m_characterObject->GetModelId() >= 0 && m_characterObject->GetModelPtr()) {
        float u0, v0, u1, v1;
        m_animManager->GetUV(u0, v0, u1, v1);
        
        if (m_movement->IsFacingLeft()) {
            float temp = u0;
            u0 = u1;
            u1 = temp;
        }
        
        m_characterObject->SetCustomUV(u0, v0, u1, v1);
        Vector3 position = m_movement->GetPosition();
        m_characterObject->SetPosition(position);
        
        if (camera) {
            m_characterObject->Draw(camera->GetViewMatrix(), camera->GetProjectionMatrix());
        }
    }
    
    bool showHitboxHurtbox = GSPlay::IsShowHitboxHurtbox();
    
    DrawHitbox(camera, showHitboxHurtbox);
    
    DrawHurtbox(camera, showHitboxHurtbox);
}



void Character::CancelCombosOnOtherAction(const bool* keyStates) {
    if (!keyStates) {
        std::cout << "Warning: keyStates is null in CancelCombosOnOtherAction" << std::endl;
        return;
    }
    
    const PlayerInputConfig& inputConfig = m_movement->GetInputConfig();
    bool isOtherAction = (keyStates[inputConfig.sitKey] || keyStates[inputConfig.rollKey] || keyStates[inputConfig.jumpKey]);
    
    if (isOtherAction && m_combat->IsInCombo()) {
        m_combat->CancelAllCombos();
    }
    
    if (isOtherAction && m_combat->IsInAxeCombo()) {
        m_combat->CancelAllCombos();
    }
    
    if (isOtherAction && m_combat->IsKicking()) {
        m_combat->CancelAllCombos();
    }
}



void Character::UpdateAnimationState() {
    if (m_movement->IsSitting()) {
        return;
    }
    
    if (m_combat->IsInCombo() && !m_animManager->IsPlaying()) {
        if (m_combat->IsComboCompleted()) {
            m_combat->CancelAllCombos();
            m_animManager->Play(0, true);
        } else if (m_combat->GetComboTimer() <= 0.0f) {
            m_combat->CancelAllCombos();
            m_animManager->Play(0, true);
            std::cout << "Combo timeout - returning to idle" << std::endl;
        }
    }
    
    if (m_combat->IsInAxeCombo() && !m_animManager->IsPlaying()) {
        if (m_combat->IsAxeComboCompleted()) {
            m_combat->CancelAllCombos();
            m_animManager->Play(0, true);
        } else if (m_combat->GetAxeComboTimer() <= 0.0f) {
            m_combat->CancelAllCombos();
            m_animManager->Play(0, true);
            std::cout << "Axe combo timeout - returning to idle" << std::endl;
        }
    }
    
    if (m_combat->IsKicking() && m_animManager->GetCurrentAnimation() == 19 && !m_animManager->IsPlaying()) {
        m_combat->CancelAllCombos();
        m_animManager->Play(0, true);
    }
    
    if (m_movement->JustLanded() && !m_combat->IsInCombo() && !m_combat->IsInAxeCombo() && !m_combat->IsKicking() && !m_combat->IsHit()) {

    }
    
    if (!m_animManager->IsPlaying() && 
        !m_combat->IsInCombo() && 
        !m_combat->IsInAxeCombo() && 
        !m_combat->IsKicking() && 
        !m_movement->IsJumping() && 
        !m_movement->IsSitting() && 
        !m_combat->IsHit()) {
        m_animManager->Play(0, true);
    }
}

void Character::SetPosition(float x, float y) {
    m_movement->SetPosition(x, y);
}

Vector3 Character::GetPosition() const {
    return m_movement->GetPosition();
}

bool Character::IsFacingLeft() const {
    return m_movement->IsFacingLeft();
}

void Character::SetFacingLeft(bool facingLeft) {
    m_movement->SetFacingLeft(facingLeft);
}

CharState Character::GetState() const {
    return m_movement->GetState();
}

bool Character::IsJumping() const {
    return m_movement->IsJumping();
}

bool Character::IsSitting() const {
    return m_movement->IsSitting();
}

void Character::SetInputConfig(const PlayerInputConfig& config) {
    m_movement->SetInputConfig(config);
}

void Character::HandleMovementAnimations(const bool* keyStates) {
    if (!keyStates) {
        return;
    }
    
    const PlayerInputConfig& inputConfig = m_movement->GetInputConfig();
    bool isShiftPressed = keyStates[16];
    
    // Only play movement animations if not in combat states
    if (!m_combat->IsInCombo() && !m_combat->IsInAxeCombo() && !m_combat->IsKicking() && !m_combat->IsHit()) {
        // Priority order: Jump > Roll > Movement > Sit > Idle
        
        if (m_movement->IsJumping()) {
            PlayAnimation(16, false);
        } else if (keyStates[inputConfig.moveLeftKey] && keyStates[inputConfig.rollKey]) {
            PlayAnimation(4, true);
        } else if (keyStates[inputConfig.moveRightKey] && keyStates[inputConfig.rollKey]) {
            PlayAnimation(4, true);
        } else if (keyStates[inputConfig.rollKey]) {
            PlayAnimation(4, true);
        } else if (keyStates[inputConfig.moveRightKey]) {
            if (isShiftPressed) {
                PlayAnimation(2, true);
            } else {
                PlayAnimation(1, true);
            }
        } else if (keyStates[inputConfig.moveLeftKey]) {
            if (isShiftPressed) {
                PlayAnimation(2, true);
            } else {
                PlayAnimation(1, true);
            }
        } else if (keyStates[inputConfig.sitKey] || m_movement->IsSitting()) {
            PlayAnimation(3, true);
        } else {
            PlayAnimation(0, true);
        }
    }
}

void Character::HandlePunchCombo() {
    if (m_combat) {
        m_combat->HandlePunchCombo(this);
    }
}

void Character::HandleAxeCombo() {
    if (m_combat) {
        m_combat->HandleAxeCombo(this);
    }
}

void Character::HandleKick() {
    if (m_combat) {
        m_combat->HandleKick(this);
    }
}

void Character::CancelAllCombos() {
    if (m_combat) {
        m_combat->CancelAllCombos();
    }
}

void Character::PlayAnimation(int animIndex, bool loop) {
    if (m_animManager) {
        bool allowReplay = (animIndex == 19) ||
                          (animIndex >= 10 && animIndex <= 12) ||
                          (animIndex >= 20 && animIndex <= 22) ||
                          (animIndex == 8 || animIndex == 9) || // Allow replay for GetHit animations
                          (animIndex == 3); // Allow replay for sit animation
        
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

// Combat getters
bool Character::IsInCombo() const {
    return m_combat ? m_combat->IsInCombo() : false;
}

int Character::GetComboCount() const {
    return m_combat ? m_combat->GetComboCount() : 0;
}

float Character::GetComboTimer() const {
    return m_combat ? m_combat->GetComboTimer() : 0.0f;
}

bool Character::IsComboCompleted() const {
    return m_combat ? m_combat->IsComboCompleted() : false;
}

// Axe combo getters
bool Character::IsInAxeCombo() const {
    return m_combat ? m_combat->IsInAxeCombo() : false;
}

int Character::GetAxeComboCount() const {
    return m_combat ? m_combat->GetAxeComboCount() : 0;
}

float Character::GetAxeComboTimer() const {
    return m_combat ? m_combat->GetAxeComboTimer() : 0.0f;
}

bool Character::IsAxeComboCompleted() const {
    return m_combat ? m_combat->IsAxeComboCompleted() : false;
}

// Kick getter
bool Character::IsKicking() const {
    return m_combat ? m_combat->IsKicking() : false;
}

// Hit getter
bool Character::IsHit() const {
    return m_combat ? m_combat->IsHit() : false;
}

// Hitbox getter
bool Character::IsHitboxActive() const {
    return m_combat ? m_combat->IsHitboxActive() : false;
}

void Character::HandleRandomGetHit() {
    if (m_combat) {
        m_combat->HandleRandomGetHit(this);
    }
}

// Hitbox management methods


void Character::DrawHitbox(Camera* camera, bool forceShow) {
    if (!camera || !m_hitboxObject || !forceShow || !m_combat) {
        return;
    }
    
    if (!m_combat->IsHitboxActive()) {
        return;
    }
    
    // Calculate hitbox position based on character position and facing direction
    Vector3 position = m_movement->GetPosition();
    float hitboxX = position.x + m_combat->GetHitboxOffsetX();
    float hitboxY = position.y + m_combat->GetHitboxOffsetY();
    
    // Set hitbox object position and scale
    m_hitboxObject->SetPosition(hitboxX, hitboxY, 0.0f);
    m_hitboxObject->SetScale(m_combat->GetHitboxWidth(), m_combat->GetHitboxHeight(), 1.0f);
    
    // Draw hitbox object
    if (camera) {
        m_hitboxObject->Draw(camera->GetViewMatrix(), camera->GetProjectionMatrix());
    }
}

void Character::SetHurtbox(float width, float height, float offsetX, float offsetY) {
    m_hurtboxWidth = width;
    m_hurtboxHeight = height;
    m_hurtboxOffsetX = offsetX;
    m_hurtboxOffsetY = offsetY;
}

void Character::DrawHurtbox(Camera* camera, bool forceShow) {
    if (!camera || !m_hurtboxObject || !forceShow) {
        return;
    }
    
    // Calculate hurtbox position based on character position
    Vector3 position = m_movement->GetPosition();
    float hurtboxX = position.x + m_hurtboxOffsetX;
    float hurtboxY = position.y + m_hurtboxOffsetY;
    
    // Set hurtbox object position and scale
    m_hurtboxObject->SetPosition(hurtboxX, hurtboxY, 0.0f);
    m_hurtboxObject->SetScale(m_hurtboxWidth, m_hurtboxHeight, 1.0f);
    
    // Draw hurtbox object
    if (camera) {
        m_hurtboxObject->Draw(camera->GetViewMatrix(), camera->GetProjectionMatrix());
    }
}

// Collision detection methods
bool Character::CheckHitboxCollision(const Character& other) const {
    if (!m_combat) {
        return false;
    }
    return m_combat->CheckHitboxCollision(*this, other);
}

void Character::TriggerGetHit(const Character& attacker) {
    if (m_combat) {
        m_combat->TriggerGetHit(this, attacker);
    }
} 