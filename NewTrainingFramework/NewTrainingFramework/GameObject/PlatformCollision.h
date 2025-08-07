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
public:
    PlatformCollision();
    ~PlatformCollision();

    void AddPlatform(float x, float y, float width, float height);
    void ClearPlatforms();
    bool CheckPlatformCollision(float& newY, float posX, float posY, float jumpVelocity, float characterWidth, float characterHeight);
    bool CheckPlatformCollisionWithHurtbox(float& newY, float posX, float posY, float jumpVelocity, float hurtboxWidth, float hurtboxHeight, float hurtboxOffsetX, float hurtboxOffsetY);
    const std::vector<Platform>& GetPlatforms() const { return m_platforms; }
};
