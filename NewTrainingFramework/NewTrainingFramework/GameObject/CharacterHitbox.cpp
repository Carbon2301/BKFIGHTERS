#include "stdafx.h"
#include "CharacterHitbox.h"
#include "Character.h"
#include "SceneManager.h"
#include "Texture2D.h"
#include "../GameManager/GSPlay.h"
#include <GLES3/gl3.h>

CharacterHitbox::CharacterHitbox() 
    : m_character(nullptr) {
    
    m_hitboxObject = std::make_unique<Object>(-1); // Use -1 as ID for hitbox
    
    m_hurtboxObject = std::make_unique<Object>(-2); // Use -2 as ID for hurtbox
}

CharacterHitbox::~CharacterHitbox() {
}

void CharacterHitbox::Initialize(Character* character, int originalObjectId) {
    m_character = character;
    
    Object* originalObj = SceneManager::GetInstance()->GetObject(originalObjectId);
    if (!originalObj) {
        return;
    }
    
    if (m_hitboxObject) {
        m_hitboxObject->SetModel(originalObj->GetModelId());
        m_hitboxObject->SetShader(originalObj->GetShaderId());
        
        // Create a red texture for hitbox
        auto redTexture = std::make_shared<Texture2D>();
        if (redTexture->CreateColorTexture(64, 64, 255, 0, 0, 180)) { // Red
            m_hitboxObject->SetDynamicTexture(redTexture);
        }
    }
    
    if (m_hurtboxObject) {
        m_hurtboxObject->SetModel(originalObj->GetModelId());
        m_hurtboxObject->SetShader(originalObj->GetShaderId());
        
        // Create a blue texture
        auto blueTexture = std::make_shared<Texture2D>();
        if (blueTexture->CreateColorTexture(64, 64, 0, 0, 255, 180)) {
            m_hurtboxObject->SetDynamicTexture(blueTexture);
        }
    }
}

void CharacterHitbox::DrawHitbox(Camera* camera, bool forceShow) {
    if (!camera || !m_hitboxObject || !forceShow || !m_character) {
        return;
    }
    
    if (!m_character->IsHitboxActive()) {
        return;
    }
    
    // Calculate hitbox position based
    Vector3 position = m_character->GetPosition();
    float hitboxX = position.x + m_character->GetHitboxOffsetX();
    float hitboxY = position.y + m_character->GetHitboxOffsetY();
    
    // Set hitbox object position and scale
    m_hitboxObject->SetPosition(hitboxX, hitboxY, 0.0f);
    m_hitboxObject->SetScale(m_character->GetHitboxWidth(), m_character->GetHitboxHeight(), 1.0f);
    
    if (camera) {
        m_hitboxObject->Draw(camera->GetViewMatrix(), camera->GetProjectionMatrix());
    }
}

bool CharacterHitbox::IsHitboxActive() const {
    if (!m_character) {
        return false;
    }
    return m_character->IsHitboxActive();
}

void CharacterHitbox::DrawHitboxAndHurtbox(Camera* camera) {
    bool showHitboxHurtbox = GSPlay::IsShowHitboxHurtbox();
    DrawHitbox(camera, showHitboxHurtbox);
    DrawHurtbox(camera, showHitboxHurtbox);
}

void CharacterHitbox::SetHurtbox(float width, float height, float offsetX, float offsetY) {
    // Backwards compatibility: sets default box
    m_defaultHurtbox = {width, height, offsetX, offsetY};
}

void CharacterHitbox::SetHurtboxDefault(float width, float height, float offsetX, float offsetY) {
    m_defaultHurtbox = {width, height, offsetX, offsetY};
}

void CharacterHitbox::SetHurtboxFacingLeft(float width, float height, float offsetX, float offsetY) {
    m_facingLeftHurtbox = {width, height, offsetX, offsetY};
}

void CharacterHitbox::SetHurtboxFacingRight(float width, float height, float offsetX, float offsetY) {
    m_facingRightHurtbox = {width, height, offsetX, offsetY};
}

void CharacterHitbox::SetHurtboxCrouchRoll(float width, float height, float offsetX, float offsetY) {
    m_crouchRollHurtbox = {width, height, offsetX, offsetY};
}

void CharacterHitbox::SetWerewolfHurtboxIdle(float width, float height, float offsetX, float offsetY)   { m_werewolfHurtboxes.idle   = {width, height, offsetX, offsetY}; }
void CharacterHitbox::SetWerewolfHurtboxWalk(float width, float height, float offsetX, float offsetY)   { m_werewolfHurtboxes.walk   = {width, height, offsetX, offsetY}; }
void CharacterHitbox::SetWerewolfHurtboxRun(float width, float height, float offsetX, float offsetY)    { m_werewolfHurtboxes.run    = {width, height, offsetX, offsetY}; }
void CharacterHitbox::SetWerewolfHurtboxJump(float width, float height, float offsetX, float offsetY)   { m_werewolfHurtboxes.jump   = {width, height, offsetX, offsetY}; }
void CharacterHitbox::SetWerewolfHurtboxCombo(float width, float height, float offsetX, float offsetY)  { m_werewolfHurtboxes.combo  = {width, height, offsetX, offsetY}; }
void CharacterHitbox::SetWerewolfHurtboxPounce(float width, float height, float offsetX, float offsetY) { m_werewolfHurtboxes.pounce = {width, height, offsetX, offsetY}; }

void CharacterHitbox::SetKitsuneHurtboxIdle(float width, float height, float offsetX, float offsetY)      { m_kitsuneHurtboxes.idle      = {width, height, offsetX, offsetY}; }
void CharacterHitbox::SetKitsuneHurtboxWalk(float width, float height, float offsetX, float offsetY)      { m_kitsuneHurtboxes.walk      = {width, height, offsetX, offsetY}; }
void CharacterHitbox::SetKitsuneHurtboxRun(float width, float height, float offsetX, float offsetY)       { m_kitsuneHurtboxes.run       = {width, height, offsetX, offsetY}; }
void CharacterHitbox::SetKitsuneHurtboxEnergyOrb(float width, float height, float offsetX, float offsetY) { m_kitsuneHurtboxes.energyOrb = {width, height, offsetX, offsetY}; }

void CharacterHitbox::GetActiveHurtbox(Hurtbox& out) const {
    out = m_defaultHurtbox;
    if (!m_character) return;
    if (m_character->IsWerewolf()) {
        if (m_character->IsJumping() && m_werewolfHurtboxes.jump.isSet()) {
            out = m_werewolfHurtboxes.jump;
        } else if (m_character->IsWerewolfPounceActive() && m_werewolfHurtboxes.pounce.isSet()) {
            out = m_werewolfHurtboxes.pounce;
        } else if (m_character->IsWerewolfComboActive() && m_werewolfHurtboxes.combo.isSet()) {
            out = m_werewolfHurtboxes.combo;
        } else {
            CharState st = m_character->GetState();
            bool isMoving = (st == CharState::MoveLeft || st == CharState::MoveRight);
            bool isRunning = m_character->GetMovement() && (m_character->GetMovement()->IsRunningLeft() || m_character->GetMovement()->IsRunningRight());
            if (isMoving && isRunning && m_werewolfHurtboxes.run.isSet()) {
                out = m_werewolfHurtboxes.run;
            } else if (isMoving && m_werewolfHurtboxes.walk.isSet()) {
                out = m_werewolfHurtboxes.walk;
            } else if (m_werewolfHurtboxes.idle.isSet()) {
                out = m_werewolfHurtboxes.idle;
            }
        }
        if (m_character->IsFacingLeft()) {
            out.offsetX = -out.offsetX;
        }
        return;
    }
    if (m_character->IsKitsune()) {
        if (m_kitsuneHurtboxes.idle.isSet()) {
            out = m_kitsuneHurtboxes.idle;
        }
        if (m_character->IsFacingLeft()) {
            out.offsetX = -out.offsetX;
        }
        return;
    }
    const bool sit = m_character->IsSitting();
    const bool roll = m_character->GetMovement() ? m_character->GetMovement()->IsRolling() : false;
    CharState st = m_character->GetState();
    const bool movingHoriz = (st == CharState::MoveLeft || st == CharState::MoveRight);
    if (roll && m_crouchRollHurtbox.isSet()) { out = m_crouchRollHurtbox; return; }
    if (sit && !movingHoriz && m_crouchRollHurtbox.isSet()) { out = m_crouchRollHurtbox; return; }
    const bool facingLeft = m_character->IsFacingLeft();
    if (facingLeft && m_facingLeftHurtbox.isSet()) { out = m_facingLeftHurtbox; return; }
    if (!facingLeft && m_facingRightHurtbox.isSet()) { out = m_facingRightHurtbox; return; }
}

void CharacterHitbox::DrawHurtbox(Camera* camera, bool forceShow) {
    if (!camera || !m_hurtboxObject || !forceShow || !m_character) {
        return;
    }
    
    Hurtbox hb; GetActiveHurtbox(hb);
    Vector3 position = m_character->GetPosition();
    float hurtboxX = position.x + hb.offsetX;
    float hurtboxY = position.y + hb.offsetY;
    
    // Set hurtbox object position and scale
    m_hurtboxObject->SetPosition(hurtboxX, hurtboxY, 0.0f);
    m_hurtboxObject->SetScale(hb.width, hb.height, 1.0f);
    
    if (camera) {
        m_hurtboxObject->Draw(camera->GetViewMatrix(), camera->GetProjectionMatrix());
    }
}

bool CharacterHitbox::CheckHitboxCollision(const Character& other) const {
    if (!m_character) {
        return false;
    }
    
    return m_character->CheckHitboxCollision(other);
}

void CharacterHitbox::TriggerGetHit(const Character& attacker) {
    if (m_character) {
        m_character->TriggerGetHit(attacker);
    }
}

bool CharacterHitbox::IsHit() const {
    if (!m_character) {
        return false;
    }
    return m_character->IsHit();
} 

float CharacterHitbox::GetHurtboxWidth() const {
    Hurtbox hb; GetActiveHurtbox(hb); return hb.width;
}
float CharacterHitbox::GetHurtboxHeight() const {
    Hurtbox hb; GetActiveHurtbox(hb); return hb.height;
}
float CharacterHitbox::GetHurtboxOffsetX() const {
    Hurtbox hb; GetActiveHurtbox(hb); return hb.offsetX;
}
float CharacterHitbox::GetHurtboxOffsetY() const {
    Hurtbox hb; GetActiveHurtbox(hb); return hb.offsetY;
}