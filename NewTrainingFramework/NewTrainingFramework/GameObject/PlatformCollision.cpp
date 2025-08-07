#include "stdafx.h"
#include "PlatformCollision.h"

PlatformCollision::PlatformCollision() {}
PlatformCollision::~PlatformCollision() {}

void PlatformCollision::AddPlatform(float x, float y, float width, float height) {
    m_platforms.emplace_back(x, y, width, height);
}

void PlatformCollision::ClearPlatforms() {
    m_platforms.clear();
}

bool PlatformCollision::CheckPlatformCollision(float& newY, float posX, float posY, float jumpVelocity, float characterWidth, float characterHeight) {
    if (jumpVelocity > 0) {
        return false;
    }
    float characterLeft = posX - characterWidth * 0.5f;
    float characterRight = posX + characterWidth * 0.5f;
    float characterBottom = posY;
    float characterTop = posY + characterHeight;
    const float epsilon = 0.005f;
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
    const float epsilon = 0.005f;
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
    return false;
}
