#pragma once
#include <memory>
#include "../../Utilities/Math.h"

class Camera;
class Object;
class Character;
class GSPlay;

class CharacterHitbox {
private:
    std::unique_ptr<Object> m_hitboxObject;
    std::unique_ptr<Object> m_hurtboxObject;
    
    float m_hurtboxWidth;
    float m_hurtboxHeight;
    float m_hurtboxOffsetX;
    float m_hurtboxOffsetY;
    
    Character* m_character;

public:
    CharacterHitbox();
    ~CharacterHitbox();
    
    void Initialize(Character* character, int originalObjectId);
    
    void DrawHitbox(Camera* camera, bool forceShow = false);
    void DrawHitboxAndHurtbox(Camera* camera);
    bool IsHitboxActive() const;
    
    void SetHurtbox(float width, float height, float offsetX, float offsetY);
    void DrawHurtbox(Camera* camera, bool forceShow = false);
    
    float GetHurtboxWidth() const { return m_hurtboxWidth; }
    float GetHurtboxHeight() const { return m_hurtboxHeight; }
    float GetHurtboxOffsetX() const { return m_hurtboxOffsetX; }
    float GetHurtboxOffsetY() const { return m_hurtboxOffsetY; }
    
    bool CheckHitboxCollision(const Character& other) const;
    void TriggerGetHit(const Character& attacker);
    bool IsHit() const;
}; 