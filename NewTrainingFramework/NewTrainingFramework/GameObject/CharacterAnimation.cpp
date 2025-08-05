#include "stdafx.h"
#include "CharacterAnimation.h"
#include "CharacterMovement.h"
#include "CharacterCombat.h"
#include "AnimationManager.h"
#include "SceneManager.h"
#include "Object.h"
#include <iostream>

CharacterAnimation::CharacterAnimation() 
    : m_lastAnimation(-1), m_objectId(1000) {
}

CharacterAnimation::~CharacterAnimation() {
}

void CharacterAnimation::Initialize(std::shared_ptr<AnimationManager> animManager, int objectId) {
    m_animManager = animManager;
    m_objectId = objectId;
    
    m_characterObject = std::make_unique<Object>(objectId);
    
    Object* originalObj = SceneManager::GetInstance()->GetObject(objectId);
    if (originalObj) {
        m_characterObject->SetModel(originalObj->GetModelId());
        const std::vector<int>& textureIds = originalObj->GetTextureIds();
        if (!textureIds.empty()) {
            m_characterObject->SetTexture(textureIds[0], 0);
        }
        m_characterObject->SetShader(originalObj->GetShaderId());
        m_characterObject->SetScale(originalObj->GetScale());
    }
    
    if (m_animManager) {
        m_animManager->Play(0, true);
    }
}

void CharacterAnimation::Update(float deltaTime, CharacterMovement* movement, CharacterCombat* combat) {
    if (m_animManager) {
        m_animManager->Update(deltaTime);
        UpdateAnimationState(movement, combat);
    }
}

void CharacterAnimation::Draw(Camera* camera, CharacterMovement* movement) {
    if (m_characterObject && m_animManager && m_characterObject->GetModelId() >= 0 && m_characterObject->GetModelPtr()) {
        float u0, v0, u1, v1;
        m_animManager->GetUV(u0, v0, u1, v1);
        
        if (movement && movement->IsFacingLeft()) {
            float temp = u0;
            u0 = u1;
            u1 = temp;
        }
        
        m_characterObject->SetCustomUV(u0, v0, u1, v1);
        Vector3 position = movement ? movement->GetPosition() : Vector3(0, 0, 0);
        m_characterObject->SetPosition(position);
        
        if (camera) {
            m_characterObject->Draw(camera->GetViewMatrix(), camera->GetProjectionMatrix());
        }
    }
}

void CharacterAnimation::UpdateAnimationState(CharacterMovement* movement, CharacterCombat* combat) {
    if (!movement || !combat) return;
    
    if (movement->IsSitting()) {
        return;
    }
    
    // Don't interrupt die animations
    if (movement->IsDying()) {
        return;
    }
    
    if (combat->IsInCombo() && !m_animManager->IsPlaying()) {
        if (combat->IsComboCompleted()) {
            combat->CancelAllCombos();
            m_animManager->Play(0, true);
        } else if (combat->GetComboTimer() <= 0.0f) {
            combat->CancelAllCombos();
            m_animManager->Play(0, true);
            std::cout << "Combo timeout - returning to idle" << std::endl;
        }
    }
    
    if (combat->IsInAxeCombo() && !m_animManager->IsPlaying()) {
        if (combat->IsAxeComboCompleted()) {
            combat->CancelAllCombos();
            m_animManager->Play(0, true);
        } else if (combat->GetAxeComboTimer() <= 0.0f) {
            combat->CancelAllCombos();
            m_animManager->Play(0, true);
            std::cout << "Axe combo timeout - returning to idle" << std::endl;
        }
    }
    
    if (combat->IsKicking() && m_animManager->GetCurrentAnimation() == 19 && !m_animManager->IsPlaying()) {
        combat->CancelAllCombos();
        m_animManager->Play(0, true);
    }
    
    if (!m_animManager->IsPlaying() && 
        !combat->IsInCombo() && 
        !combat->IsInAxeCombo() && 
        !combat->IsKicking() && 
        !movement->IsJumping() && 
        !movement->IsSitting() && 
        !combat->IsHit() &&
        !movement->IsDying()) {
        m_animManager->Play(0, true);
    }
}

void CharacterAnimation::HandleMovementAnimations(const bool* keyStates, CharacterMovement* movement, CharacterCombat* combat) {
    if (!keyStates || !movement || !combat) {
        return;
    }
    
    const PlayerInputConfig& inputConfig = movement->GetInputConfig();
    bool isShiftPressed = keyStates[16];
    
    if (movement->IsDying()) {
        float dieTimer = movement->GetDieTimer();
        
        if (dieTimer < 0.8f) {
            PlayAnimation(13, false);
        }
        else if (dieTimer < 2.8f) {
            PlayAnimation(15, false);
        }
        else {
            PlayAnimation(0, true);
        }
        return;
    }
    
    // Only play movement animations if not in combat states
    if (!combat->IsInCombo() && !combat->IsInAxeCombo() && !combat->IsKicking() && !combat->IsHit()) {
        // Priority order: Jump > Roll > Movement > Sit > Idle
        
        if (movement->IsJumping()) {
            PlayAnimation(16, false);
        } else if ((keyStates[inputConfig.rollLeftKey1] && keyStates[inputConfig.rollLeftKey2]) ||
                   (keyStates[inputConfig.rollRightKey1] && keyStates[inputConfig.rollRightKey2])) {
            PlayAnimation(4, true);
        } else if (keyStates[inputConfig.moveRightKey]) {
            if (isShiftPressed) {
                PlayAnimation(2, true);
            } else {
                PlayAnimation(1, true);
            }
        } else if (keyStates[inputConfig.moveLeftKey]) {
            if (isShiftPressed) {
                PlayAnimation(2, true);
            } else {
                PlayAnimation(1, true);
            }
        } else if (keyStates[inputConfig.sitKey] || movement->IsSitting()) {
            PlayAnimation(3, true);
        } else {
            PlayAnimation(0, true);
        }
    }
}

void CharacterAnimation::PlayAnimation(int animIndex, bool loop) {
    if (m_animManager) {
        bool allowReplay = (animIndex == 19) ||
                          (animIndex >= 10 && animIndex <= 12) ||
                          (animIndex >= 20 && animIndex <= 22) ||
                          (animIndex == 8 || animIndex == 9) ||
                          (animIndex == 3);
        
        if (m_lastAnimation != animIndex || allowReplay) {
            m_animManager->Play(animIndex, loop);
            m_lastAnimation = animIndex;
        }
    }
}

int CharacterAnimation::GetCurrentAnimation() const {
    return m_animManager ? m_animManager->GetCurrentAnimation() : -1;
}

bool CharacterAnimation::IsAnimationPlaying() const {
    return m_animManager ? m_animManager->IsPlaying() : false;
}

bool CharacterAnimation::IsFacingLeft(CharacterMovement* movement) const {
    return movement ? movement->IsFacingLeft() : false;
} 