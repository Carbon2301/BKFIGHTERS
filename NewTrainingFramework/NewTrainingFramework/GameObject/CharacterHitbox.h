#pragma once
#include <memory>
#include "../../Utilities/Math.h"

class Camera;
class Object;
class Character;
class GSPlay;

class CharacterHitbox {
private:
    // Hitbox objects for visualization
    std::unique_ptr<Object> m_hitboxObject;
    std::unique_ptr<Object> m_hurtboxObject;
    
    // Hurtbox dimensions and offset
    float m_hurtboxWidth;
    float m_hurtboxHeight;
    float m_hurtboxOffsetX;
    float m_hurtboxOffsetY;
    
    // Character reference for position and combat state
    Character* m_character;

public:
    CharacterHitbox();
    ~CharacterHitbox();
    
    // Initialization
    void Initialize(Character* character, int originalObjectId);
    
    // Hitbox management
    void DrawHitbox(Camera* camera, bool forceShow = false);
    void DrawHitboxAndHurtbox(Camera* camera);
    bool IsHitboxActive() const;
    
    // Hurtbox management
    void SetHurtbox(float width, float height, float offsetX, float offsetY);
    void DrawHurtbox(Camera* camera, bool forceShow = false);
    
    // Hurtbox getters for collision detection
    float GetHurtboxWidth() const { return m_hurtboxWidth; }
    float GetHurtboxHeight() const { return m_hurtboxHeight; }
    float GetHurtboxOffsetX() const { return m_hurtboxOffsetX; }
    float GetHurtboxOffsetY() const { return m_hurtboxOffsetY; }
    
    // Collision detection
    bool CheckHitboxCollision(const Character& other) const;
    void TriggerGetHit(const Character& attacker);
    bool IsHit() const;
}; 