#include "stdafx.h"
#include "EnergyOrbProjectile.h"
#include "Camera.h"
#include "../GameManager/ResourceManager.h"
#include "../GameManager/SceneManager.h"
#include "WallCollision.h"

EnergyOrbProjectile::EnergyOrbProjectile()
    : m_isActive(false)
    , m_position(0.0f, 0.0f, 0.0f)
    , m_velocity(0.0f, 0.0f, 0.0f)
    , m_speed(2.0f)
    , m_lifetime(0.0f)
    , m_maxLifetime(5.0f)
    , m_isExploding(false)
    , m_explosionTimer(0.0f)
    , m_explosionDuration(0.5f)
    , m_currentAnimation(0)
    , m_animationLoop(true)
    , m_wallCollision(nullptr)
    , m_ownerId(0)
    , m_explosionCallback(nullptr) {
}

EnergyOrbProjectile::~EnergyOrbProjectile() {
}

void EnergyOrbProjectile::Initialize() {
    //(Energy orb from GSPlay.txt)
    m_object = std::make_unique<Object>(1503);
    
    Object* originalObj = SceneManager::GetInstance()->GetObject(1503);
    if (originalObj) {
        m_object->SetModel(originalObj->GetModelId());
        m_object->SetTexture(originalObj->GetTextureIds()[0], 0);
        m_object->SetShader(originalObj->GetShaderId());
        m_object->SetScale(originalObj->GetScale());
        
        m_object->MakeModelInstanceCopy();
    }
    
    if (auto texData = ResourceManager::GetInstance()->GetTextureData(65)) {
        m_animManager = std::make_shared<AnimationManager>();
        std::vector<AnimationData> anims;
        anims.reserve(texData->animations.size());
        for (const auto& a : texData->animations) {
            anims.push_back({a.startFrame, a.numFrames, a.duration, 0.0f});
        }
        m_animManager->Initialize(texData->spriteWidth, texData->spriteHeight, anims);
    }
}

void EnergyOrbProjectile::Spawn(const Vector3& position, const Vector3& direction, float speed, int ownerId) {
    m_position = position;
    Vector3 dir(direction.x, direction.y, direction.z);
    m_velocity = dir * speed;
    m_speed = speed;
    m_lifetime = 0.0f;
    m_isActive = true;
    m_isExploding = false;
    m_explosionTimer = 0.0f;
    m_currentAnimation = 0;
    m_animationLoop = true;
    m_ownerId = ownerId;
    
    if (m_animManager) {
        m_animManager->Play(0, true);
    }
    
    if (m_object) {
        m_object->SetPosition(m_position);
        
        if (m_animManager) {
            float u0, v0, u1, v1;
            m_animManager->GetUV(u0, v0, u1, v1);
            m_object->SetCustomUV(u0, v0, u1, v1);
        }
    }
}

void EnergyOrbProjectile::Update(float deltaTime) {
    if (!m_isActive) return;
    
    m_lifetime += deltaTime;
    
    if (m_lifetime >= m_maxLifetime) {
        TriggerExplosion();
        return;
    }
    
    if (m_isExploding) {
        m_explosionTimer += deltaTime;
        if (m_explosionTimer >= m_explosionDuration) {
            m_isActive = false;
            return;
        }
        
        if (m_animManager && m_currentAnimation != 1) {
            m_animManager->Play(1, false);
            m_currentAnimation = 1;
            m_animationLoop = false;
            
            if (m_object) {
                float u0, v0, u1, v1;
                m_animManager->GetUV(u0, v0, u1, v1);
                m_object->SetCustomUV(u0, v0, u1, v1);
            }
        }
    } else {
        Vector3 vel(m_velocity.x, m_velocity.y, m_velocity.z);
        m_position += vel * deltaTime;
        
        if (m_object) {
            m_object->SetPosition(m_position);
        }
        
        HandleCollision();
        
    }
    
    if (m_animManager) {
        m_animManager->Update(deltaTime);
    }
    
    if (m_animManager && m_object) {
        float u0, v0, u1, v1;
        m_animManager->GetUV(u0, v0, u1, v1);
        m_object->SetCustomUV(u0, v0, u1, v1);
    }
}

void EnergyOrbProjectile::Draw(Camera* camera) {
    if (!m_isActive || !m_object || !camera) return;
    
    m_object->Draw(camera->GetViewMatrix(), camera->GetProjectionMatrix());
}

void EnergyOrbProjectile::TriggerExplosion() {
    if (m_isExploding) return;
    
    m_isExploding = true;
    m_explosionTimer = 0.0f;
    
    m_velocity = Vector3(0.0f, 0.0f, 0.0f);
    
    if (m_explosionCallback) {
        m_explosionCallback(m_position.x);
    }
    
    if (m_animManager) {
        m_animManager->Play(1, false);
        m_currentAnimation = 1;
        m_animationLoop = false;
        
        if (m_object) {
            float u0, v0, u1, v1;
            m_animManager->GetUV(u0, v0, u1, v1);
            m_object->SetCustomUV(u0, v0, u1, v1);
        }
    }
}

void EnergyOrbProjectile::HandleCollision() {
    if (m_position.x < -10.0f || m_position.x > 10.0f || 
        m_position.y < -10.0f || m_position.y > 10.0f) {
        TriggerExplosion();
        return;
    }
    
    if (CheckWallCollision()) {
        TriggerExplosion();
        return;
    }
}

bool EnergyOrbProjectile::CheckWallCollision() const {
    if (!m_wallCollision) {
        return false;
    }
    
    const float PROJECTILE_SIZE = 0.02f;
    
    return m_wallCollision->CheckWallCollision(
        m_position,
        PROJECTILE_SIZE, PROJECTILE_SIZE,
        0.0f, 0.0f                
    );
}
