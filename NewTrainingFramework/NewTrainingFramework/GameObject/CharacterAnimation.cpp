#include "stdafx.h"
#include "CharacterAnimation.h"
#include "CharacterMovement.h"
#include "CharacterCombat.h"
#include "AnimationManager.h"
#include "SceneManager.h"
#include "Object.h"
#include <SDL.h>
#include <iostream>

CharacterAnimation::CharacterAnimation() 
    : m_lastAnimation(-1), m_objectId(1000),
      m_climbHoldTimer(0.0f), m_lastClimbDir(0),
      m_prevClimbUpPressed(false), m_prevClimbDownPressed(false),
      m_downPressStartTime(-1.0f) {
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
        } else {
            m_animManager->Play(0, true);
        }
    }
    
    if (combat->IsInAxeCombo() && !m_animManager->IsPlaying()) {
        if (combat->IsAxeComboCompleted()) {
            combat->CancelAllCombos();
            m_animManager->Play(0, true);
        } else if (combat->GetAxeComboTimer() <= 0.0f) {
            combat->CancelAllCombos();
            m_animManager->Play(0, true);
        } else {
            m_animManager->Play(0, true);
        }
    }
    
    if (combat->IsKicking()) {
        int cur = m_animManager->GetCurrentAnimation();
        bool isKickAnim = (cur == 17 || cur == 19);
        if (!m_animManager->IsPlaying() || !movement->IsJumping() || !isKickAnim) {
            combat->CancelAllCombos();
            m_animManager->Play(0, true);
        }
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
    
    if (!combat->IsInCombo() && !combat->IsInAxeCombo() && !combat->IsKicking() && !combat->IsHit()) {
        // Ưu tiên animation leo thang khi đang ở trên ladder
        if (movement->IsOnLadder()) {
            if (GetCurrentAnimation() != 6) {
                PlayAnimation(6, true);
            }

            const PlayerInputConfig& input = movement->GetInputConfig();
            const bool upHeld = keyStates[input.jumpKey];
            const bool downHeld = keyStates[input.sitKey];
            const bool upPressed = upHeld;
            const bool downPressed = downHeld;
            const bool upJustPressed = upPressed && !m_prevClimbUpPressed;
            const bool downJustPressed = downPressed && !m_prevClimbDownPressed;

            if (upJustPressed && !upHeld) {
                const AnimationData* anim = m_animManager->GetAnimation(6);
                if (anim) {
                    int frame = m_animManager->GetCurrentFrame();
                    frame = (frame + 1) % anim->numFrames; // 1->2->3->4->1
                    m_animManager->SetCurrentFrame(frame);
                }
            }
            if (downJustPressed) {
                const AnimationData* anim = m_animManager->GetAnimation(6);
                if (anim) {
                    int frame = m_animManager->GetCurrentFrame();
                    frame = (frame - 1);
                    if (frame < 0) frame = anim->numFrames - 1; // 4->3->2->1->4
                    m_animManager->SetCurrentFrame(frame);
                }
            }

            float now = SDL_GetTicks() / 1000.0f;
            if (downJustPressed) {
                m_downPressStartTime = now;
            }
            bool isDownHeldLong = false;
            if (downHeld && m_downPressStartTime >= 0.0f) {
                isDownHeldLong = (now - m_downPressStartTime) > CLIMB_DOWN_HOLD_THRESHOLD;
            }
            if (!downHeld) {
                m_downPressStartTime = -1.0f;
            }

            bool leftHeld = keyStates[input.moveLeftKey];
            bool rightHeld = keyStates[input.moveRightKey];
            if (upHeld || leftHeld || rightHeld) {
                m_animManager->SetPlaying(true);
                m_lastClimbDir = 1;
            } else if (isDownHeldLong) {
                const AnimationData* anim = m_animManager->GetAnimation(6);
                if (anim) {
                    m_animManager->Pause();
                    m_animManager->SetCurrentFrame(0);
                }
                m_lastClimbDir = -1;
            } else if (!upHeld && !downHeld) {
                m_animManager->Pause();
                m_lastClimbDir = 0;
            }

            m_prevClimbUpPressed = upPressed;
            m_prevClimbDownPressed = downPressed;
            return;
        }
        
        if (movement->IsJumping()) {
            if (combat->IsKicking() && GetCurrentAnimation() == 17) {
            } else {
                PlayAnimation(16, false);
            }
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
        bool allowReplay = (animIndex == 19 || animIndex == 17) ||
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

void CharacterAnimation::GetCurrentFrameUV(float& u0, float& v0, float& u1, float& v1) const {
    if (m_animManager) {
        m_animManager->GetUV(u0, v0, u1, v1);
    } else {
        u0 = 0.0f; v0 = 0.0f; u1 = 1.0f; v1 = 1.0f;
    }
}