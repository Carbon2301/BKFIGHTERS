#include "stdafx.h"
#include "Character.h"
#include "CharacterAnimation.h"
#include "CharacterHitbox.h"
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
      m_hitbox(std::make_unique<CharacterHitbox>()),
      m_animation(std::make_unique<CharacterAnimation>()),
      m_health(100.0f), m_isDead(false) {
}

Character::~Character() {
}

void Character::Initialize(std::shared_ptr<AnimationManager> animManager, int objectId) {
    if (m_animation) {
        m_animation->Initialize(animManager, objectId);
    }
    
    // Initialize movement with position from scene object
    Object* originalObj = SceneManager::GetInstance()->GetObject(objectId);
    if (originalObj) {
        const Vector3& originalPos = originalObj->GetPosition();
        m_movement->Initialize(originalPos.x, originalPos.y, originalPos.y);
    } else {
        m_movement->Initialize(0.0f, 0.0f, 0.0f);
    }
    
    // Initialize hitbox system
    if (m_hitbox) {
        m_hitbox->Initialize(this, objectId);
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
    if (m_animation) {
        m_animation->HandleMovementAnimations(keyStates, m_movement.get(), m_combat.get());
    }
    
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
    
    if (inputManager->IsKeyJustPressed(inputConfig.dieKey)) {
        HandleDie();
    }
}

void Character::Update(float deltaTime) {
    // Update animation system
    if (m_animation) {
        m_animation->Update(deltaTime, m_movement.get(), m_combat.get());
    }
    
    if (m_combat) {
        m_combat->Update(deltaTime);
    }
}

void Character::Draw(Camera* camera) {
    // Draw character animation
    if (m_animation) {
        m_animation->Draw(camera, m_movement.get());
    }
    
    // Draw hitbox
    if (m_hitbox) {
        m_hitbox->DrawHitboxAndHurtbox(camera);
    }
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

void Character::HandlePunchCombo() {
    if (m_combat && m_animation) {
        m_combat->HandlePunchCombo(m_animation.get(), m_movement.get());
    }
}

void Character::HandleAxeCombo() {
    if (m_combat && m_animation) {
        m_combat->HandleAxeCombo(m_animation.get(), m_movement.get());
    }
}

void Character::HandleKick() {
    if (m_combat && m_animation) {
        m_combat->HandleKick(m_animation.get(), m_movement.get());
    }
}

void Character::HandleDie() {
    if (m_movement && !m_movement->IsDying() && !m_movement->IsDead()) {
        m_movement->TriggerDie();
        std::cout << "Die triggered via input" << std::endl;
    }
}

void Character::TriggerDie() {
    if (m_movement && !m_movement->IsDying() && !m_movement->IsDead()) {
        m_movement->TriggerDie();
        std::cout << "Die triggered externally" << std::endl;
    }
}

void Character::CancelAllCombos() {
    if (m_combat) {
        m_combat->CancelAllCombos();
    }
}

void Character::PlayAnimation(int animIndex, bool loop) {
    if (m_animation) {
        m_animation->PlayAnimation(animIndex, loop);
    }
}

int Character::GetCurrentAnimation() const {
    return m_animation ? m_animation->GetCurrentAnimation() : -1;
}

bool Character::IsAnimationPlaying() const {
    return m_animation ? m_animation->IsAnimationPlaying() : false;
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



// Hitbox management methods
void Character::DrawHitbox(Camera* camera, bool forceShow) {
    if (m_hitbox) {
        m_hitbox->DrawHitbox(camera, forceShow);
    }
}

void Character::DrawHitboxAndHurtbox(Camera* camera) {
    if (m_hitbox) {
        m_hitbox->DrawHitboxAndHurtbox(camera);
    }
}

void Character::SetHurtbox(float width, float height, float offsetX, float offsetY) {
    if (m_hitbox) {
        m_hitbox->SetHurtbox(width, height, offsetX, offsetY);
    }
}

void Character::DrawHurtbox(Camera* camera, bool forceShow) {
    if (m_hitbox) {
        m_hitbox->DrawHurtbox(camera, forceShow);
    }
}

// Hurtbox getters
float Character::GetHurtboxWidth() const {
    return m_hitbox ? m_hitbox->GetHurtboxWidth() : 0.0f;
}

float Character::GetHurtboxHeight() const {
    return m_hitbox ? m_hitbox->GetHurtboxHeight() : 0.0f;
}

float Character::GetHurtboxOffsetX() const {
    return m_hitbox ? m_hitbox->GetHurtboxOffsetX() : 0.0f;
}

float Character::GetHurtboxOffsetY() const {
    return m_hitbox ? m_hitbox->GetHurtboxOffsetY() : 0.0f;
}

// Hitbox getters
float Character::GetHitboxWidth() const {
    return m_combat ? m_combat->GetHitboxWidth() : 0.0f;
}

float Character::GetHitboxHeight() const {
    return m_combat ? m_combat->GetHitboxHeight() : 0.0f;
}

float Character::GetHitboxOffsetX() const {
    return m_combat ? m_combat->GetHitboxOffsetX() : 0.0f;
}

float Character::GetHitboxOffsetY() const {
    return m_combat ? m_combat->GetHitboxOffsetY() : 0.0f;
}

// Collision detection methods
bool Character::CheckHitboxCollision(const Character& other) const {
    if (!m_combat) {
        return false;
    }
    return m_combat->CheckHitboxCollision(*this, other);
}

void Character::TriggerGetHit(const Character& attacker) {
    if (m_combat && m_animation) {
        // Set facing direction based on attacker
        SetFacingLeft(!attacker.IsFacingLeft());
        m_combat->TriggerGetHit(m_animation.get(), attacker, this);
    }
} 

// Health system methods
void Character::TakeDamage(float damage) {
    if (m_isDead) return;
    
    m_health -= damage;
    if (m_health < 0.0f) {
        m_health = 0.0f;
    }
    
    std::cout << "=== DAMAGE TAKEN ===" << std::endl;
    std::cout << "Health: " << m_health << "/" << MAX_HEALTH << std::endl;
    
    if (m_health <= 0.0f && !m_isDead) {
        m_isDead = true;
        TriggerDie();
        std::cout << "Character died! Health: " << m_health << std::endl;
    }
    std::cout << "===================" << std::endl;
}

void Character::Heal(float amount) {
    if (m_isDead) return;
    
    m_health += amount;
    if (m_health > MAX_HEALTH) {
        m_health = MAX_HEALTH;
    }
}

void Character::ResetHealth() {
    m_health = MAX_HEALTH;
    m_isDead = false;
} 