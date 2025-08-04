#include "stdafx.h"
#include "CharacterHitbox.h"
#include "Character.h"
#include "SceneManager.h"
#include "Texture2D.h"
#include "../GameManager/GSPlay.h"
#include <GLES3/gl3.h>

CharacterHitbox::CharacterHitbox() 
    : m_hurtboxWidth(0.0f), m_hurtboxHeight(0.0f), 
      m_hurtboxOffsetX(0.0f), m_hurtboxOffsetY(0.0f), m_character(nullptr) {
    
    // Initialize hitbox object
    m_hitboxObject = std::make_unique<Object>(-1); // Use -1 as ID for hitbox
    
    // Initialize hurtbox object
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
    
    // Setup hitbox object - use the same model as character but with red color
    if (m_hitboxObject) {
        m_hitboxObject->SetModel(originalObj->GetModelId());
        m_hitboxObject->SetShader(originalObj->GetShaderId());
        
        // Create a red texture for hitbox
        auto redTexture = std::make_shared<Texture2D>();
        if (redTexture->CreateColorTexture(64, 64, 255, 0, 0, 180)) { // Red with some transparency
            m_hitboxObject->SetDynamicTexture(redTexture);
        }
    }
    
    // Setup hurtbox object
    if (m_hurtboxObject) {
        m_hurtboxObject->SetModel(originalObj->GetModelId());
        m_hurtboxObject->SetShader(originalObj->GetShaderId());
        
        // Create a blue texture for hurtbox
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
    
    // Calculate hitbox position based on character position and facing direction
    Vector3 position = m_character->GetPosition();
    float hitboxX = position.x + m_character->GetHitboxOffsetX();
    float hitboxY = position.y + m_character->GetHitboxOffsetY();
    
    // Set hitbox object position and scale
    m_hitboxObject->SetPosition(hitboxX, hitboxY, 0.0f);
    m_hitboxObject->SetScale(m_character->GetHitboxWidth(), m_character->GetHitboxHeight(), 1.0f);
    
    // Draw hitbox object
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
    m_hurtboxWidth = width;
    m_hurtboxHeight = height;
    m_hurtboxOffsetX = offsetX;
    m_hurtboxOffsetY = offsetY;
}

void CharacterHitbox::DrawHurtbox(Camera* camera, bool forceShow) {
    if (!camera || !m_hurtboxObject || !forceShow || !m_character) {
        return;
    }
    
    // Calculate hurtbox position based on character position
    Vector3 position = m_character->GetPosition();
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

bool CharacterHitbox::CheckHitboxCollision(const Character& other) const {
    if (!m_character) {
        return false;
    }
    
    // This method should delegate to the combat component
    // We'll need to add a method in Character class to access combat collision detection
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