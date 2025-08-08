#pragma once
#include "../../Utilities/Math.h"
#include <vector>

// Struct đại diện cho một platform
struct Platform {
    float x, y, width, height;
    Platform(float px, float py, float w, float h) : x(px), y(py), width(w), height(h) {}
};

class PlatformCollision {
private:
    std::vector<Platform> m_platforms;
    std::vector<int> m_movingPlatformObjectIds;
    int m_lastCollidedMovingPlatformId = -1;
public:
    PlatformCollision();
    ~PlatformCollision();

    void AddPlatform(float x, float y, float width, float height);
    void ClearPlatforms();
    void AddMovingPlatform(int objectId);
    void ClearMovingPlatforms();
    int GetLastCollidedMovingPlatformId() const { return m_lastCollidedMovingPlatformId; }
    bool CheckPlatformCollision(float& newY, float posX, float posY, float jumpVelocity, float characterWidth, float characterHeight);
    bool CheckPlatformCollisionWithHurtbox(float& newY, float posX, float posY, float jumpVelocity, float hurtboxWidth, float hurtboxHeight, float hurtboxOffsetX, float hurtboxOffsetY);
    const std::vector<Platform>& GetPlatforms() const { return m_platforms; }
};
