#include "stdafx.h"
#include "PlatformCollision.h"
#include "SceneManager.h"
#include "Object.h"

PlatformCollision::PlatformCollision() {}
PlatformCollision::~PlatformCollision() {}

void PlatformCollision::AddPlatform(float x, float y, float width, float height) {
    m_platforms.emplace_back(x, y, width, height);
    std::cout << "[PlatformCollision] Add platform: Pos:(" << x << ", " << y << ") Size:(" << width << ", " << height << ")" << std::endl;
}

void PlatformCollision::ClearPlatforms() {
    m_platforms.clear();
}

void PlatformCollision::AddMovingPlatform(int objectId) {
    for (int id : m_movingPlatformObjectIds) {
        if (id == objectId) return;
    }
    m_movingPlatformObjectIds.push_back(objectId);
}

void PlatformCollision::ClearMovingPlatforms() {
    m_movingPlatformObjectIds.clear();
}

bool PlatformCollision::CheckPlatformCollision(float& newY, float posX, float posY, float jumpVelocity, float characterWidth, float characterHeight) {
    if (jumpVelocity > 0) {
        return false;
    }
    float characterLeft = posX - characterWidth * 0.5f;
    float characterRight = posX + characterWidth * 0.5f;
    float characterBottom = posY;
    float characterTop = posY + characterHeight;
    // Tăng epsilon để dễ hạ cánh hơn khi bệ di chuyển nhanh theo frame
    const float epsilon = 0.015f;
    m_lastCollidedMovingPlatformId = -1;

    // Static platforms
    for (const auto& platform : m_platforms) {
        float platformLeft = platform.x - platform.width * 0.5f;
        float platformRight = platform.x + platform.width * 0.5f;
        float platformBottom = platform.y - platform.height * 0.5f;
        float platformTop = platform.y + platform.height * 0.5f;
        if (jumpVelocity <= 0 &&
            characterBottom >= platformTop - epsilon &&
            characterBottom <= platformTop + epsilon &&
            characterRight > platformLeft && characterLeft < platformRight) {
            newY = platformTop;
            return true;
        }
    }

    for (int objectId : m_movingPlatformObjectIds) {
        Object* obj = SceneManager::GetInstance()->GetObject(objectId);
        if (!obj) continue;
        const Vector3& pos = obj->GetPosition();
        const Vector3& scale = obj->GetScale();
        float platformLeft = pos.x - scale.x * 0.5f;
        float platformRight = pos.x + scale.x * 0.5f;
        float platformBottom = pos.y - scale.y * 0.5f;
        float platformTop = pos.y + scale.y * 0.5f;
        if (jumpVelocity <= 0 &&
            characterBottom >= platformTop - epsilon &&
            characterBottom <= platformTop + epsilon &&
            characterRight > platformLeft && characterLeft < platformRight) {
            newY = platformTop;
            m_lastCollidedMovingPlatformId = objectId;
            return true;
        }
    }
    return false;
}

bool PlatformCollision::CheckPlatformCollisionWithHurtbox(float& newY, float posX, float posY, float jumpVelocity, float hurtboxWidth, float hurtboxHeight, float hurtboxOffsetX, float hurtboxOffsetY) {
    if (jumpVelocity > 0) {
        return false;
    }
    float hurtboxLeft = posX + hurtboxOffsetX - hurtboxWidth * 0.5f;
    float hurtboxRight = posX + hurtboxOffsetX + hurtboxWidth * 0.5f;
    float hurtboxBottom = posY + hurtboxOffsetY - hurtboxHeight * 0.5f;
    float hurtboxTop = posY + hurtboxOffsetY + hurtboxHeight * 0.5f;
    const float epsilon = 0.015f;
    m_lastCollidedMovingPlatformId = -1;

    // Static platforms
    for (const auto& platform : m_platforms) {
        float platformLeft = platform.x - platform.width * 0.5f;
        float platformRight = platform.x + platform.width * 0.5f;
        float platformBottom = platform.y - platform.height * 0.5f;
        float platformTop = platform.y + platform.height * 0.5f;
        if (jumpVelocity <= 0 &&
            hurtboxBottom >= platformTop - epsilon &&
            hurtboxBottom <= platformTop + epsilon &&
            hurtboxRight > platformLeft && hurtboxLeft < platformRight) {
            newY = platformTop - hurtboxOffsetY + hurtboxHeight * 0.5f;
            return true;
        }
    }

    // Moving platforms
    for (int objectId : m_movingPlatformObjectIds) {
        Object* obj = SceneManager::GetInstance()->GetObject(objectId);
        if (!obj) continue;
        const Vector3& pos = obj->GetPosition();
        const Vector3& scale = obj->GetScale();
        float platformLeft = pos.x - scale.x * 0.5f;
        float platformRight = pos.x + scale.x * 0.5f;
        float platformBottom = pos.y - scale.y * 0.5f;
        float platformTop = pos.y + scale.y * 0.5f;
        if (jumpVelocity <= 0 &&
            hurtboxBottom >= platformTop - epsilon &&
            hurtboxBottom <= platformTop + epsilon &&
            hurtboxRight > platformLeft && hurtboxLeft < platformRight) {
            newY = platformTop - hurtboxOffsetY + hurtboxHeight * 0.5f;
            m_lastCollidedMovingPlatformId = objectId;
            return true;
        }
    }
    return false;
}
