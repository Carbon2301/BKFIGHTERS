#include "stdafx.h"
#include "CharacterAnimation.h"
#include "CharacterMovement.h"
#include "CharacterMovement.h" // ensure PlayerInputConfig is visible
#include "CharacterCombat.h"
#include "AnimationManager.h"
#include "SceneManager.h"
#include "../GameManager/ResourceManager.h"
#include "Object.h"
#include <SDL.h>
#include <iostream>
#include <cmath>

static inline float ClampFloat(float v, float mn, float mx) {
    return v < mn ? mn : (v > mx ? mx : v);
}

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

    int headTexId = (m_objectId == 1000) ? 8 : 9;
    const TextureData* headTex = ResourceManager::GetInstance()->GetTextureData(headTexId);
    if (headTex && headTex->spriteWidth > 0 && headTex->spriteHeight > 0) {
        m_topAnimManager = std::make_shared<AnimationManager>();
        std::vector<AnimationData> topAnims;
        topAnims.reserve(headTex->animations.size());
        for (const auto& a : headTex->animations) {
            topAnims.push_back({a.startFrame, a.numFrames, a.duration, 0.0f});
        }
        m_topAnimManager->Initialize(headTex->spriteWidth, headTex->spriteHeight, topAnims);
        m_topAnimManager->Play(0, true);

        m_topObject = std::make_unique<Object>(objectId + 10000); // unique id for overlay
        if (originalObj) {
            m_topObject->SetModel(originalObj->GetModelId());
            m_topObject->SetShader(originalObj->GetShaderId());
            m_topObject->SetScale(originalObj->GetScale());
        }
        m_topObject->SetTexture(headTexId, 0);
    }
}

void CharacterAnimation::Update(float deltaTime, CharacterMovement* movement, CharacterCombat* combat) {
    if (m_animManager) {
        m_animManager->Update(deltaTime);
        UpdateAnimationState(movement, combat);
    }
    if (m_topAnimManager) {
        m_topAnimManager->Update(deltaTime);
    }

    if (m_isBatDemon) {
        if (m_batSlashCooldownTimer > 0.0f) {
            m_batSlashCooldownTimer -= deltaTime;
            if (m_batSlashCooldownTimer < 0.0f) m_batSlashCooldownTimer = 0.0f;
        }
    }

    if (m_isWerewolf && movement) {
        if (movement->IsJumping()) {
            m_werewolfAirTimer += deltaTime;
        } else {
            m_werewolfAirTimer = 0.0f;
        }
        if (m_werewolfPounceActive) {
            float dir = movement->IsFacingLeft() ? -1.0f : 1.0f;
            Vector3 pos = movement->GetPosition();
            movement->SetPosition(pos.x + dir * m_werewolfPounceSpeed * deltaTime, pos.y);
            if (combat && m_werewolfPounceHitWindowTimer > 0.0f) {
                float w = 0.15f;
                float h = 0.2f;
                float ox = (dir < 0.0f) ? -0.12f : 0.12f;
                float oy = -0.1f;
                combat->ShowHitbox(w, h, ox, oy);
            }
        }
        if (m_werewolfComboCooldownTimer > 0.0f) {
            m_werewolfComboCooldownTimer -= deltaTime;
            if (m_werewolfComboCooldownTimer < 0.0f) m_werewolfComboCooldownTimer = 0.0f;
        }
        if (m_werewolfPounceCooldownTimer > 0.0f) {
            m_werewolfPounceCooldownTimer -= deltaTime;
            if (m_werewolfPounceCooldownTimer < 0.0f) m_werewolfPounceCooldownTimer = 0.0f;
        }
        if (m_werewolfComboHitWindowTimer > 0.0f) {
            m_werewolfComboHitWindowTimer -= deltaTime;
            if (m_werewolfComboHitWindowTimer < 0.0f) m_werewolfComboHitWindowTimer = 0.0f;
        }
    }

    // Handle turn timing
    if (m_gunMode && movement && m_isTurning) {
        m_turnTimer += deltaTime;
        if (m_turnTimer >= TURN_DURATION) {
            m_isTurning = false;
            m_prevFacingLeft = m_turnTargetLeft;
            movement->SetFacingLeft(m_turnTargetLeft);
            PlayTopAnimation(1, true);
        }
    }
    
    if (m_recoilActive) {
        m_recoilTimer += deltaTime;
        if (m_recoilTimer >= RECOIL_DURATION) {
            m_recoilActive = false;
            if (m_gunTopAnimReload >= 0) {
                m_reloadActive = true;
                m_reloadTimer = 0.0f;
            }
            m_recoilOffsetX = 0.0f;
            m_recoilOffsetY = 0.0f;
        } else {
            float progress = m_recoilTimer / RECOIL_DURATION;
            float easingFactor = 1.0f - powf(1.0f - progress, 3.0f);
            float currentStrength = (1.0f - easingFactor);
            
            float aimRad = m_lastShotAimDeg * 3.14159265f / 180.0f;
            float angleWorld = m_recoilFaceSign * aimRad;
            float muzzleX = cosf(angleWorld);
            float muzzleY = sinf(angleWorld);
            m_recoilOffsetX = muzzleX * RECOIL_STRENGTH * currentStrength;
            m_recoilOffsetY = muzzleY * RECOIL_STRENGTH * currentStrength;
        }
    }

    if (m_reloadActive) {
        m_reloadTimer += deltaTime;
        if (m_reloadTimer >= RELOAD_DURATION) {
            m_reloadActive = false;
            m_reloadTimer = 0.0f;
        }
    }
}

void CharacterAnimation::StartHardLanding(CharacterMovement* movement) {
    if (m_hardLandingActive) return;
    unsigned int nowMs = SDL_GetTicks();
    if (m_blockHardLandingUntilMs != 0 && nowMs < m_blockHardLandingUntilMs) {
        return;
    }
    m_hardLandingActive = true;
    m_hardLandingPhase = 0;
    if (movement) {
        m_restoreInputAfterHardLanding = !movement->IsInputLocked();
        movement->SetInputLocked(true);
    } else {
        m_restoreInputAfterHardLanding = false;
    }
    m_gunMode = false;
    m_grenadeMode = false;
    m_isTurning = false;
    m_gunEntering = false;
    m_reloadActive = false;
    m_recoilActive = false;
    PlayAnimation(15, false);
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
        if (m_isWerewolf) {
            m_characterObject->SetPosition(position.x, position.y + m_werewolfBodyOffsetY, position.z);
        } else {
            m_characterObject->SetPosition(position);
        }
        
        if (camera) {
            m_characterObject->Draw(camera->GetViewMatrix(), camera->GetProjectionMatrix());
        }
    }

    if (!m_isBatDemon && (m_gunMode || m_recoilActive) && m_topObject && m_topAnimManager && m_topObject->GetModelId() >= 0 && m_topObject->GetModelPtr()) {
        float u0, v0, u1, v1;
        m_topAnimManager->GetUV(u0, v0, u1, v1);
        
        bool shouldFlipUV = m_gunMode ? 
                           (movement && movement->IsFacingLeft()) : 
                           (m_recoilFaceSign < 0.0f);
        if (shouldFlipUV) {
            std::swap(u0, u1);
        }
        m_topObject->SetCustomUV(u0, v0, u1, v1);
        Vector3 position = movement ? movement->GetPosition() : Vector3(0, 0, 0);
        
        bool isLeftFacing = m_gunMode ? 
                           (movement && movement->IsFacingLeft()) : 
                           (m_recoilFaceSign < 0.0f);
        float offsetX = isLeftFacing ? -m_topOffsetX : m_topOffsetX;
        
        float finalOffsetX = offsetX + m_recoilOffsetX;
        float finalOffsetY = m_topOffsetY + m_recoilOffsetY;
        float bodyY = m_isWerewolf ? (position.y + m_werewolfBodyOffsetY) : position.y;
        m_topObject->SetPosition(position.x + finalOffsetX, bodyY + finalOffsetY, position.z);
        
        float faceSign = m_gunMode ? 
                        ((movement && movement->IsFacingLeft()) ? -1.0f : 1.0f) : 
                        m_recoilFaceSign;
        float aimAngle = m_gunMode ? m_aimAngleDeg : m_lastShotAimDeg;
        m_topObject->SetRotation(0.0f, 0.0f, faceSign * aimAngle * 3.14159265f / 180.0f);
        if (camera) {
            m_topObject->Draw(camera->GetViewMatrix(), camera->GetProjectionMatrix());
        }
    }

    if (!m_isBatDemon && m_grenadeMode && m_topObject && m_topAnimManager && m_topObject->GetModelId() >= 0 && m_topObject->GetModelPtr()) {
        float u0, v0, u1, v1;
        m_topAnimManager->GetUV(u0, v0, u1, v1);
        if (movement && movement->IsFacingLeft()) {
            std::swap(u0, u1);
        }
        m_topObject->SetCustomUV(u0, v0, u1, v1);
        Vector3 position = movement ? movement->GetPosition() : Vector3(0, 0, 0);
        float offsetX = (movement && movement->IsFacingLeft()) ? -m_topOffsetX : m_topOffsetX;
        float bodyY2 = m_isWerewolf ? (position.y + m_werewolfBodyOffsetY) : position.y;
        m_topObject->SetPosition(position.x + offsetX, bodyY2 + m_topOffsetY, position.z);
        float faceSign = (movement && movement->IsFacingLeft()) ? -1.0f : 1.0f;
        m_topObject->SetRotation(0.0f, 0.0f, faceSign * m_aimAngleDeg * 3.14159265f / 180.0f);
        if (camera) {
            m_topObject->Draw(camera->GetViewMatrix(), camera->GetProjectionMatrix());
        }
    }
}

Vector3 CharacterAnimation::GetTopWorldPosition(CharacterMovement* movement) const {
    Vector3 base = movement ? movement->GetPosition() : Vector3(0, 0, 0);
    float offsetX = (movement && movement->IsFacingLeft()) ? -m_topOffsetX : m_topOffsetX;
    return Vector3(base.x + offsetX, base.y + m_topOffsetY, base.z);
}

void CharacterAnimation::UpdateAnimationState(CharacterMovement* movement, CharacterCombat* combat) {
    if (!movement) return;

    // BatDemon mode
    if (m_isBatDemon) {
        if (m_characterObject) {
            const std::vector<int>& texIds = m_characterObject->GetTextureIds();
            int currentTex = texIds.empty() ? -1 : texIds[0];
            if (currentTex != 61) {
                m_characterObject->SetTexture(61, 0);
                if (auto texData = ResourceManager::GetInstance()->GetTextureData(61)) {
                    if (!m_animManager) {
                        m_animManager = std::make_shared<AnimationManager>();
                    }
                    std::vector<AnimationData> anims;
                    anims.reserve(texData->animations.size());
                    for (const auto& a : texData->animations) {
                        anims.push_back({a.startFrame, a.numFrames, a.duration, 0.0f});
                    }
                    m_animManager->Initialize(texData->spriteWidth, texData->spriteHeight, anims);
                    m_lastAnimation = -1;
                }
            }
        }
        if (m_animManager) {
            if (m_batSlashActive) {
                int cur = GetCurrentAnimation();
                if (cur != 1) {
                    m_animManager->Play(1, false);
                    m_lastAnimation = 1;
                }
                if (!m_animManager->IsPlaying()) {
                    m_batSlashActive = false;
                    m_batSlashCooldownTimer = m_batSlashCooldown;
                }
            } else {
                int cur = GetCurrentAnimation();
                if (cur != 0) {
                    m_animManager->Play(0, true);
                    m_lastAnimation = 0;
                } else {
                    m_animManager->Resume();
                }
            }
        }
        return;
    }

    // Werewolf mode
    if (m_isWerewolf) {
        if (m_characterObject) {
            const std::vector<int>& texIds = m_characterObject->GetTextureIds();
            int currentTex = texIds.empty() ? -1 : texIds[0];
            if (currentTex != 60) {
                m_characterObject->SetTexture(60, 0);
                if (auto texData = ResourceManager::GetInstance()->GetTextureData(60)) {
                    if (!m_animManager) {
                        m_animManager = std::make_shared<AnimationManager>();
                    }
                    std::vector<AnimationData> anims;
                    anims.reserve(texData->animations.size());
                    for (const auto& a : texData->animations) {
                        anims.push_back({a.startFrame, a.numFrames, a.duration, 0.0f});
                    }
                    m_animManager->Initialize(texData->spriteWidth, texData->spriteHeight, anims);
                    m_lastAnimation = -1;
                }
            }
        }
        // Drive werewolf anims by movement state
        if (movement && m_animManager) {
            bool physJumping = movement->IsJumping();
            if (!physJumping) { m_werewolfAirTimer = 0.0f; }
            bool considerJumping = (m_werewolfAirTimer >= WEREWOLF_AIR_DEBOUNCE);

            // Pounce overrides combo and movement
            if (m_werewolfPounceActive) {
                int cur = GetCurrentAnimation();
                if (cur != 3) {
                    m_animManager->Play(3, false);
                    m_lastAnimation = 3;
                }
                if (!m_animManager->IsPlaying()) {
                    m_werewolfPounceActive = false;
                    m_werewolfPounceCooldownTimer = m_werewolfPounceCooldown;
                }
                return;
            }
            if (m_werewolfComboActive) {
                int cur = GetCurrentAnimation();
                if (cur != 1) {
                    m_animManager->Play(1, false);
                    m_lastAnimation = 1;
                }
                if (!m_animManager->IsPlaying()) {
                    m_werewolfComboActive = false;
                    m_werewolfComboCooldownTimer = m_werewolfComboCooldown;
                }
                return;
            }

            int desired = 0;
            bool loop = true;
            if (considerJumping) {
                desired = 5; loop = false;
            } else {
                CharState st = movement->GetState();
                bool isMoving = (st == CharState::MoveLeft || st == CharState::MoveRight);
                bool isRunning = movement->IsRunningLeft() || movement->IsRunningRight();
                if (isMoving && isRunning) { desired = 2; loop = true; }
                else if (isMoving) { desired = 4; loop = true; }
                else { desired = 0; loop = true; }
            }
            int cur = GetCurrentAnimation();
            if (cur != desired || (desired == 5 && !m_animManager->IsPlaying())) {
                m_animManager->Play(desired, loop);
                m_lastAnimation = desired;
            } else {
                m_animManager->Resume();
            }
        }
        return;
    }

    if (!m_hardLandingActive && movement->ConsumeHardLandingRequested()) {
        StartHardLanding(movement);
        return;
    }

    if (m_hardLandingActive) {
        if (movement && !movement->IsInputLocked()) {
            movement->SetInputLocked(true);
        }
        int cur = GetCurrentAnimation();
        if (m_hardLandingPhase == 0) {
            if (cur != 15) {
                PlayAnimation(15, false);
            } else if (!m_animManager->IsPlaying()) {
                m_hardLandingPhase = 1;
                PlayAnimation(14, false);
            }
        } else {
            if (cur != 14) {
                PlayAnimation(14, false);
            } else if (!m_animManager->IsPlaying()) {
                m_hardLandingActive = false;
                m_hardLandingPhase = 0;
                if (m_restoreInputAfterHardLanding) {
                    movement->SetInputLocked(false);
                }
                PlayAnimation(0, true);
                unsigned int doneMs = SDL_GetTicks();
                m_blockHardLandingUntilMs = doneMs + 120; // ~0.12s debounce
            }
        }
        return;
    }

    if (!combat) {
        return;
    }

    if (m_grenadeMode) {
        PlayAnimation(31, true);
        PlayTopAnimation(7, true);
        return;
    }

    if (m_gunMode) {
        if (m_isWerewolf) {
            return;
        }
        if (m_isBatDemon) {
            m_gunMode = false;
            m_isTurning = false;
            m_gunEntering = false;
            // fall back to BatDemon render path
            return;
        }
        return;
    }
    
    if (movement->IsSitting()) {
        if (combat->IsKicking() || combat->IsInCombo() || combat->IsInAxeCombo()) {
            combat->CancelAllCombos();
        }
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
    if (m_hardLandingActive) {
        return;
    }
    
    const PlayerInputConfig& inputConfig = movement->GetInputConfig();
    bool isShiftPressed = keyStates[16];

    if (m_isBatDemon) { return; }

    if (m_isWerewolf) { return; }

    // Grenade mode: allow aiming like gun mode
    if (m_grenadeMode) {
        if (m_isWerewolf) {
            m_grenadeMode = false;
        } else {
        if (m_isBatDemon) {
            m_grenadeMode = false;
            return;
        }
        // Allow turning; when turn happens, reset aim like gun mode
        bool currentLeft = movement->IsFacingLeft();
        bool wantLeft = keyStates[inputConfig.moveLeftKey];
        bool wantRight = keyStates[inputConfig.moveRightKey];
        bool turned = false;
        if (wantLeft && !currentLeft) {
            movement->SetFacingLeft(true);
            turned = true;
        } else if (wantRight && currentLeft) {
            movement->SetFacingLeft(false);
            turned = true;
        }
        if (turned) {
            m_aimAngleDeg = 0.0f;
            m_aimHoldTimerUp = m_aimHoldTimerDown = 0.0f;
            m_aimSincePressUp = m_aimSincePressDown = 0.0f;
            m_prevAimUp = m_prevAimDown = false;
        }

        HandleGunAim(keyStates, inputConfig);
        PlayAnimation(31, true);
        PlayTopAnimation(7, true);
        return;
        }
    }

    if (m_gunMode) {
        if (m_syncFacingOnEnter) {
            m_prevFacingLeft = movement->IsFacingLeft();
            m_syncFacingOnEnter = false;
        }
        bool facingLeft = m_prevFacingLeft;
        const PlayerInputConfig& ic = movement->GetInputConfig();
        bool wantLeft = keyStates[ic.moveLeftKey];
        bool wantRight = keyStates[ic.moveRightKey];
        if (!m_isTurning) {
            if (wantLeft && !facingLeft) {
                m_turnTargetLeft = true;
                m_isTurning = true;
                m_turnTimer = 0.0f;
                m_aimAngleDeg = 0.0f;
                m_aimHoldTimerUp = m_aimHoldTimerDown = 0.0f;
                m_aimSincePressUp = m_aimSincePressDown = 0.0f;
                m_prevAimUp = m_prevAimDown = false;
                PlayTopAnimation(0, false);
            } else if (wantRight && facingLeft) {
                m_turnTargetLeft = false;
                m_isTurning = true;
                m_turnTimer = 0.0f;
                m_aimAngleDeg = 0.0f;
                m_aimHoldTimerUp = m_aimHoldTimerDown = 0.0f;
                m_aimSincePressUp = m_aimSincePressDown = 0.0f;
                m_prevAimUp = m_prevAimDown = false;
                PlayTopAnimation(0, false);
            }
        }

        if (m_isTurning || m_gunEntering) {
            if (m_gunEntering) {
                PlayTopAnimation(m_gunTopAnimReverse, false);
            }
            PlayAnimation(29, true);
            if (m_gunEntering) {
                unsigned int nowMs = SDL_GetTicks();
                float elapsed = (nowMs - m_gunEnterStartMs) / 1000.0f;
                if (elapsed >= GUN_ENTER_DURATION) {
                    m_gunEntering = false;
                }
            }
            return;
        }

        HandleGunAim(keyStates, movement->GetInputConfig());

        PlayAnimation(29, true);
        if (m_reloadActive && m_gunTopAnimReload >= 0) {
            PlayTopAnimation(m_gunTopAnimReload, false);
        } else {
            PlayTopAnimation(m_gunTopAnimHold, true);
        }
        return;
    }
    
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
                    frame = (frame + 1) % anim->numFrames;
                    m_animManager->SetCurrentFrame(frame);
                }
            }
            if (downJustPressed) {
                const AnimationData* anim = m_animManager->GetAnimation(6);
                if (anim) {
                    int frame = m_animManager->GetCurrentFrame();
                    frame = (frame - 1);
                    if (frame < 0) frame = anim->numFrames - 1;
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
        } else if (movement->IsRolling()) {
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
    if (m_hardLandingActive) {
        int required = (m_hardLandingPhase == 0) ? 15 : 14;
        if (animIndex != required) {
            return;
        }
        if (m_animManager) {
            int cur = m_animManager->GetCurrentAnimation();
            if (cur == required && m_animManager->IsPlaying()) {
                return;
            }
        }
    }
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

void CharacterAnimation::PlayTopAnimation(int animIndex, bool loop) {
    if (m_hardLandingActive) {
        return;
    }
    if (m_topAnimManager) {
        if (m_lastTopAnimation != animIndex) {
            m_topAnimManager->Play(animIndex, loop);
            m_lastTopAnimation = animIndex;
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

void CharacterAnimation::SetGunMode(bool enabled) {
    if (m_isBatDemon && enabled) {
        return;
    }
    if (enabled && !m_gunMode) {
        unsigned int nowMsEnter = SDL_GetTicks();
        bool isSticky = (m_lastShotTickMs != 0 && (nowMsEnter - m_lastShotTickMs) <= STICKY_AIM_WINDOW_MS);
        if (isSticky) {
            m_aimAngleDeg = m_lastShotAimDeg;
        } else {
            m_aimAngleDeg = 0.0f;
        }
        m_prevAimUp = m_prevAimDown = false;
        m_aimHoldTimerUp = m_aimHoldTimerDown = 0.0f;
        m_aimSincePressUp = m_aimSincePressDown = 0.0f;
        m_gunEntering = !isSticky;
        m_gunEnterStartMs = nowMsEnter;
        m_syncFacingOnEnter = true;
        m_aimHoldBlockUntilMs = m_gunEnterStartMs + (unsigned int)(AIM_HOLD_INITIAL_DELAY * 1000.0f);
        m_lastAimTickMs = m_gunEnterStartMs;
    }
    if (enabled) {
        // Gun mode and grenade mode are mutually exclusive
        m_grenadeMode = false;
    }
    m_gunMode = enabled;
}

void CharacterAnimation::SetGrenadeMode(bool enabled) {
    if (m_isBatDemon && enabled) {
        return;
    }
    if (enabled && !m_grenadeMode) {
        // Initialize aim like entering gun mode
        unsigned int nowMsEnter = SDL_GetTicks();
        bool isSticky = (m_lastShotTickMs != 0 && (nowMsEnter - m_lastShotTickMs) <= STICKY_AIM_WINDOW_MS);
        if (!isSticky) {
            m_aimAngleDeg = 0.0f;
        }
        m_prevAimUp = m_prevAimDown = false;
        m_aimHoldTimerUp = m_aimHoldTimerDown = 0.0f;
        m_aimSincePressUp = m_aimSincePressDown = 0.0f;
        m_aimHoldBlockUntilMs = nowMsEnter + (unsigned int)(AIM_HOLD_INITIAL_DELAY * 1000.0f);
        m_lastAimTickMs = nowMsEnter;
    }
    if (enabled) {
        // Grenade mode and gun mode are mutually exclusive
        m_gunMode = false;
        m_isTurning = false;
        m_gunEntering = false;
    }
    m_grenadeMode = enabled;
}

void CharacterAnimation::SetBatDemonMode(bool enabled) {
    m_isBatDemon = enabled;
    m_batSlashActive = false;
    if (enabled) {
        // Disable overlays
        m_gunMode = false;
        m_grenadeMode = false;
        // Switch texture and start Fly
        if (m_characterObject) {
            m_characterObject->SetTexture(61, 0);
        }
        if (auto texData = ResourceManager::GetInstance()->GetTextureData(61)) {
            if (!m_animManager) {
                m_animManager = std::make_shared<AnimationManager>();
            }
            std::vector<AnimationData> anims;
            anims.reserve(texData->animations.size());
            for (const auto& a : texData->animations) {
                anims.push_back({a.startFrame, a.numFrames, a.duration, 0.0f});
            }
            m_animManager->Initialize(texData->spriteWidth, texData->spriteHeight, anims);
            // Animation 0: Fly
            m_animManager->Play(0, true);
            m_lastAnimation = 0;
        }
    } else {
        // Restore original player body or werewolf depending on state
        if (m_isWerewolf) {
            if (m_characterObject) {
                m_characterObject->SetTexture(60, 0);
            }
            if (auto texData = ResourceManager::GetInstance()->GetTextureData(60)) {
                if (!m_animManager) {
                    m_animManager = std::make_shared<AnimationManager>();
                }
                std::vector<AnimationData> anims;
                anims.reserve(texData->animations.size());
                for (const auto& a : texData->animations) {
                    anims.push_back({a.startFrame, a.numFrames, a.duration, 0.0f});
                }
                m_animManager->Initialize(texData->spriteWidth, texData->spriteHeight, anims);
                m_animManager->Play(0, true);
                m_lastAnimation = 0;
            }
        } else {
            int bodyTexId = (m_objectId == 1000) ? 10 : 11;
            if (m_characterObject) {
                m_characterObject->SetTexture(bodyTexId, 0);
            }
            if (auto texData = ResourceManager::GetInstance()->GetTextureData(bodyTexId)) {
                if (!m_animManager) {
                    m_animManager = std::make_shared<AnimationManager>();
                }
                std::vector<AnimationData> anims;
                anims.reserve(texData->animations.size());
                for (const auto& a : texData->animations) {
                    anims.push_back({a.startFrame, a.numFrames, a.duration, 0.0f});
                }
                m_animManager->Initialize(texData->spriteWidth, texData->spriteHeight, anims);
                m_animManager->Play(0, true);
                m_lastAnimation = 0;
            }
        }
    }
}

void CharacterAnimation::TriggerBatDemonSlash() {
    if (!m_isBatDemon) return;
    if (m_batSlashCooldownTimer > 0.0f) return;
    if (m_batSlashActive) return;
    m_batSlashActive = true;
    if (m_animManager) {
        m_animManager->Play(1, false);
        m_lastAnimation = 1;
    }
}

void CharacterAnimation::GetCurrentFrameUV(float& u0, float& v0, float& u1, float& v1) const {
    if (m_animManager) {
        m_animManager->GetUV(u0, v0, u1, v1);
    } else {
        u0 = 0.0f; v0 = 0.0f; u1 = 1.0f; v1 = 1.0f;
    }
}

void CharacterAnimation::GetTopFrameUV(float& u0, float& v0, float& u1, float& v1) const {
    if (m_topAnimManager) {
        m_topAnimManager->GetUV(u0, v0, u1, v1);
    } else {
        u0 = 0.0f; v0 = 0.0f; u1 = 1.0f; v1 = 1.0f;
    }
}

void CharacterAnimation::StartTurn(bool toLeft, bool initialLeft) {
    m_isTurning = true;
    m_turnTargetLeft = toLeft;
    m_turnTimer = 0.0f;
    m_turnInitialLeft = initialLeft;
    PlayTopAnimation(0, false);
}

void CharacterAnimation::HandleGunAim(const bool* keyStates, const PlayerInputConfig& inputConfig) {
    if (!keyStates) return;
    const int aimUpKey = inputConfig.jumpKey;
    const int aimDownKey = inputConfig.sitKey;

    unsigned int nowMs = SDL_GetTicks();
    float dt = 0.0f;
    if (m_lastAimTickMs == 0) {
        dt = 0.0f;
    } else {
        unsigned int diff = nowMs - m_lastAimTickMs;
        dt = diff / 1000.0f;
        if (dt > 0.05f) dt = 0.05f;
        if (dt < 0.0f) dt = 0.0f;
    }
    m_lastAimTickMs = nowMs;

    bool upHeld = keyStates[aimUpKey];
    bool downHeld = keyStates[aimDownKey];
    bool upJust = upHeld && !this->m_prevAimUp;
    bool downJust = downHeld && !this->m_prevAimDown;

    if (upJust && nowMs >= m_aimHoldBlockUntilMs) {
        this->m_aimAngleDeg = ClampFloat(this->m_aimAngleDeg + 6.0f, -90.0f, 90.0f);
        m_aimSincePressUp = 0.0f;
    }
    if (downJust && nowMs >= m_aimHoldBlockUntilMs) {
        this->m_aimAngleDeg = ClampFloat(this->m_aimAngleDeg - 6.0f, -90.0f, 90.0f);
        m_aimSincePressDown = 0.0f;
    }

    if (upJust && downJust) {
        m_aimSincePressUp = m_aimSincePressDown = 0.0f;
        m_aimHoldTimerUp = m_aimHoldTimerDown = 0.0f;
    }

    if (upHeld && nowMs >= m_aimHoldBlockUntilMs) {
        m_aimSincePressUp += dt;
        if (m_aimSincePressUp >= AIM_HOLD_INITIAL_DELAY) {
            m_aimHoldTimerUp += dt;
            while (m_aimHoldTimerUp >= AIM_HOLD_STEP_INTERVAL) {
                m_aimHoldTimerUp -= AIM_HOLD_STEP_INTERVAL;
                this->m_aimAngleDeg = ClampFloat(this->m_aimAngleDeg + 6.0f, -90.0f, 90.0f);
            }
        }
    } else {
        m_aimHoldTimerUp = 0.0f;
        m_aimSincePressUp = 0.0f;
    }

    if (downHeld && nowMs >= m_aimHoldBlockUntilMs) {
        m_aimSincePressDown += dt;
        if (m_aimSincePressDown >= AIM_HOLD_INITIAL_DELAY) {
            m_aimHoldTimerDown += dt;
            while (m_aimHoldTimerDown >= AIM_HOLD_STEP_INTERVAL) {
                m_aimHoldTimerDown -= AIM_HOLD_STEP_INTERVAL;
                this->m_aimAngleDeg = ClampFloat(this->m_aimAngleDeg - 6.0f, -90.0f, 90.0f);
            }
        }
    } else {
        m_aimHoldTimerDown = 0.0f;
        m_aimSincePressDown = 0.0f;
    }

    this->m_prevAimUp = upHeld;
    this->m_prevAimDown = downHeld;
}

void CharacterAnimation::OnGunShotFired(CharacterMovement* movement) {
    m_lastShotAimDeg = m_aimAngleDeg;
    m_lastShotTickMs = SDL_GetTicks();
    
    m_recoilActive = true;
    m_recoilTimer = 0.0f;
    
    m_recoilFaceSign = (movement && movement->IsFacingLeft()) ? -1.0f : 1.0f;
    
    float aimRad = m_aimAngleDeg * 3.14159265f / 180.0f;
    float angleWorld = m_recoilFaceSign * aimRad;
    
    float muzzleX = cosf(angleWorld);
    float muzzleY = sinf(angleWorld);
    
    float k = RECOIL_STRENGTH * m_recoilStrengthMul;
    m_recoilOffsetX = muzzleX * k;
    m_recoilOffsetY = muzzleY * k;
}

void CharacterAnimation::SetGunByTextureId(int texId) {
    m_gunTopAnimReverse = 0;
    m_gunTopAnimHold    = 1;
    m_gunTopAnimReload  = -1;
    m_recoilStrengthMul = 1.0f;
    m_reloadActive = false;
    m_reloadTimer = 0.0f;
    m_recoilActive = false;
    m_recoilTimer = 0.0f;

    switch (texId) {
        case 41: // M4A1
            m_gunTopAnimReverse = 2;
            m_gunTopAnimHold    = 3;
            break;
        case 42: // Shotgun
            m_gunTopAnimReverse = 4;
            m_gunTopAnimHold    = 5;
            m_gunTopAnimReload  = 6;
            break;
        case 43: // Bazoka
            m_gunTopAnimReverse = 8;
            m_gunTopAnimHold    = 9;
            break;
        case 44: // FlameGun
            m_gunTopAnimReverse = 10;
            m_gunTopAnimHold    = 11;
            break;
        case 45: // Deagle
            m_gunTopAnimReverse = 12;
            m_gunTopAnimHold    = 13;
            break;
        case 46: // Sniper
            m_gunTopAnimReverse = 14;
            m_gunTopAnimHold    = 15;
            break;
        case 47: // Uzi
            m_gunTopAnimReverse = 16;
            m_gunTopAnimHold    = 17;
            m_recoilStrengthMul  = 1.8f;
            break;
        case 40: // Pistol (default)
        default:
            m_gunTopAnimReverse = 0;
            m_gunTopAnimHold    = 1;
            m_gunTopAnimReload  = -1;
            m_recoilStrengthMul  = 1.0f;
            break;
    }
}

void CharacterAnimation::SetWerewolfMode(bool enabled) {
    m_isWerewolf = enabled;
    m_werewolfComboActive = false;
    m_werewolfPounceActive = false;
    if (enabled) {
        // Disable gun/grenade overlays when werewolf
        m_gunMode = false;
        m_grenadeMode = false;
        // Switch texture immediately and start werewolf Idle
        if (m_characterObject) {
            m_characterObject->SetTexture(60, 0);
        }
        if (auto texData = ResourceManager::GetInstance()->GetTextureData(60)) {
            if (!m_animManager) {
                m_animManager = std::make_shared<AnimationManager>();
            }
            std::vector<AnimationData> anims;
            anims.reserve(texData->animations.size());
            for (const auto& a : texData->animations) {
                anims.push_back({a.startFrame, a.numFrames, a.duration, 0.0f});
            }
            m_animManager->Initialize(texData->spriteWidth, texData->spriteHeight, anims);
            m_animManager->Play(0, true);
            m_lastAnimation = 0;
        }
    } else {
        // Restore original player body texture and animations
        int bodyTexId = (m_objectId == 1000) ? 10 : 11;
        if (m_characterObject) {
            m_characterObject->SetTexture(bodyTexId, 0);
        }
        if (auto texData = ResourceManager::GetInstance()->GetTextureData(bodyTexId)) {
            if (!m_animManager) {
                m_animManager = std::make_shared<AnimationManager>();
            }
            std::vector<AnimationData> anims;
            anims.reserve(texData->animations.size());
            for (const auto& a : texData->animations) {
                anims.push_back({a.startFrame, a.numFrames, a.duration, 0.0f});
            }
            m_animManager->Initialize(texData->spriteWidth, texData->spriteHeight, anims);
            m_animManager->Play(0, true);
            m_lastAnimation = 0;
        }
    }
}

void CharacterAnimation::TriggerWerewolfCombo() {
    if (!m_isWerewolf) return;
    if (m_werewolfComboCooldownTimer > 0.0f || m_werewolfComboActive) return;
    m_werewolfComboActive = true;
    m_werewolfComboHitWindowTimer = m_werewolfComboHitWindow;
    if (m_animManager) {
        m_animManager->Play(1, false);
        m_lastAnimation = 1;
    }
}

void CharacterAnimation::TriggerWerewolfPounce() {
    if (!m_isWerewolf) return;
    if (m_werewolfPounceCooldownTimer > 0.0f || m_werewolfPounceActive) return;
    m_werewolfComboActive = false;
    m_werewolfPounceActive = true;
    m_werewolfPounceHitWindowTimer = m_werewolfPounceHitWindow;
    if (m_animManager) {
        m_animManager->Play(3, false);
        m_lastAnimation = 3;
    }
}