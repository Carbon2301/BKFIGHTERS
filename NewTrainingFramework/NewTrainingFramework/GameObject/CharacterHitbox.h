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
    
    struct Hurtbox {
        float width = 0.0f;
        float height = 0.0f;
        float offsetX = 0.0f;
        float offsetY = 0.0f;
        bool  isSet() const { return width > 0.0f && height > 0.0f; }
    };
    Hurtbox m_defaultHurtbox;
    Hurtbox m_facingLeftHurtbox;
    Hurtbox m_facingRightHurtbox;
    Hurtbox m_crouchRollHurtbox;
    
    Character* m_character;

    void GetActiveHurtbox(Hurtbox& out) const;

public:
    CharacterHitbox();
    ~CharacterHitbox();
    
    void Initialize(Character* character, int originalObjectId);
    
    void DrawHitbox(Camera* camera, bool forceShow = false);
    void DrawHitboxAndHurtbox(Camera* camera);
    bool IsHitboxActive() const;
    
    void SetHurtbox(float width, float height, float offsetX, float offsetY);
    void SetHurtboxDefault(float width, float height, float offsetX, float offsetY);
    void SetHurtboxFacingLeft(float width, float height, float offsetX, float offsetY);
    void SetHurtboxFacingRight(float width, float height, float offsetX, float offsetY);
    void SetHurtboxCrouchRoll(float width, float height, float offsetX, float offsetY);
    void DrawHurtbox(Camera* camera, bool forceShow = false);
    
    float GetHurtboxWidth() const;
    float GetHurtboxHeight() const;
    float GetHurtboxOffsetX() const;
    float GetHurtboxOffsetY() const;
    
    bool CheckHitboxCollision(const Character& other) const;
    void TriggerGetHit(const Character& attacker);
    bool IsHit() const;
}; 