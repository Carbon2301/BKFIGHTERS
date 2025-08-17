#include "stdafx.h"
#include "Character.h"
#include "CharacterAnimation.h"
#include "CharacterHitbox.h"
#include "../GameManager/SoundManager.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "InputManager.h"
#include "Object.h"
#include "Texture2D.h"
#include "../GameManager/GSPlay.h"
#include "InputManager.h"
#include <GLES3/gl3.h>
#include <iostream>
#include <cstdlib>
#include <memory>

Character::Character() 
    : m_movement(std::make_unique<CharacterMovement>(CharacterMovement::PLAYER1_INPUT)),
      m_combat(std::make_unique<CharacterCombat>()),
      m_hitbox(std::make_unique<CharacterHitbox>()),
      m_animation(std::make_unique<CharacterAnimation>()),
      m_health(100.0f), m_isDead(false),
      m_stamina(100.0f) {
}

Character::~Character() {
}
void Character::SetGunMode(bool enabled) {
    if (m_animation) {
        m_animation->SetGunMode(enabled);
    }
}

bool Character::IsGunMode() const {
    return m_animation ? m_animation->IsGunMode() : false;
}

void Character::SetGrenadeMode(bool enabled) {
    if (m_animation) {
        m_animation->SetGrenadeMode(enabled);
    }
}

bool Character::IsGrenadeMode() const {
    return m_animation ? m_animation->IsGrenadeMode() : false;
}

void Character::SetBatDemonMode(bool enabled) {
    if (m_animation) {
        m_animation->SetBatDemonMode(enabled);
    }
    if (enabled) {
        m_isDead = false;
        if (m_movement) {
            m_movement->ResetDieState();
        }
        
        SetGunMode(false);
        SetGrenadeMode(false);
        if (m_movement) {
            m_movement->SetNoClipNoGravity(true);
            m_movement->SetMoveSpeedMultiplier(1.5f);
        }
    }
    else {
        if (m_movement) {
            m_movement->SetNoClipNoGravity(false);
            m_movement->SetMoveSpeedMultiplier(1.0f);
        }
        
        Vector3 pos = GetPosition();
        if (pos.x < -3.7f || pos.x > 3.4f || pos.y < -1.2f || pos.y > 1.7f) {
            if (m_selfDeathCallback) {
                m_selfDeathCallback(*this);
            } else {
                TriggerDie();
            }
        }
    }
}

bool Character::IsBatDemon() const {
    return m_animation ? m_animation->IsBatDemon() : false;
}

void Character::TriggerBatDemonSlash() {
    if (m_animation) {
        m_animation->TriggerBatDemonSlash();
    }
}

void Character::SetWerewolfMode(bool enabled) {
    if (m_animation) {
        m_animation->SetWerewolfMode(enabled);
    }
    if (enabled) {
        m_isDead = false;
        if (m_movement) {
            m_movement->ResetDieState();
        }
        
        SetGunMode(false);
        SetGrenadeMode(false);
        if (m_movement) {
            m_movement->SetMoveSpeedMultiplier(1.5f);
            m_movement->SetRunSpeedMultiplier(2.0f);
            m_movement->SetJumpForceMultiplier(1.5f);
            m_movement->SetLadderDoubleTapEnabled(false);
            m_movement->SetLadderEnabled(false);
        }
    } else {
        if (m_movement) {
            m_movement->SetMoveSpeedMultiplier(1.0f);
            m_movement->SetRunSpeedMultiplier(1.0f);
            m_movement->SetJumpForceMultiplier(1.0f);
            m_movement->SetLadderDoubleTapEnabled(true);
            m_movement->SetLadderEnabled(true);
        }
    }
}

bool Character::IsWerewolf() const {
    return m_animation ? m_animation->IsWerewolf() : false;
}

void Character::TriggerWerewolfCombo() {
    if (m_animation) {
        m_animation->TriggerWerewolfCombo();
    }
    if (m_animation && m_movement && m_combat && m_animation->IsWerewolf()) {
        bool facingLeft = m_animation->IsFacingLeft(m_movement.get());
        float w = 0.3f;
        float h = 0.17f;
        float ox = facingLeft ? -0.03f : 0.03f;
        float oy = -0.02f;
        m_combat->ShowHitbox(w, h, ox, oy);
    }
}

void Character::TriggerWerewolfPounce() {
    if (m_animation) {
        m_animation->TriggerWerewolfPounce();
    }
}

void Character::SetKitsuneMode(bool enabled) {
    if (m_animation) {
        m_animation->SetKitsuneMode(enabled);
    }
    if (enabled) {
        m_isDead = false;
        if (m_movement) {
            m_movement->ResetDieState();
        }
        
        SetGunMode(false);
        SetGrenadeMode(false);
        if (m_movement) {
            m_movement->SetNoClipNoGravity(false);
            m_movement->SetInputLocked(false);
            m_movement->SetMoveSpeedMultiplier(1.2f);
            m_movement->SetRunSpeedMultiplier(1.2f);
            m_movement->SetJumpForceMultiplier(1.2f);
            m_movement->SetLadderDoubleTapEnabled(false);
            m_movement->SetLadderEnabled(false);
        }
    } else {
        if (m_movement) {
            m_movement->SetMoveSpeedMultiplier(1.0f);
            m_movement->SetRunSpeedMultiplier(1.0f);
            m_movement->SetJumpForceMultiplier(1.0f);
            m_movement->SetLadderDoubleTapEnabled(true);
            m_movement->SetLadderEnabled(true);
        }
    }
}

bool Character::IsKitsune() const {
    return m_animation ? m_animation->IsKitsune() : false;
}

void Character::TriggerKitsuneEnergyOrb() {
    if (m_animation) {
        m_animation->TriggerKitsuneEnergyOrb();
    }
}

bool Character::IsKitsuneEnergyOrbAnimationComplete() const {
    return m_animation ? m_animation->IsKitsuneEnergyOrbAnimationComplete() : false;
}

void Character::ResetKitsuneEnergyOrbAnimationComplete() {
    if (m_animation) {
        m_animation->ResetKitsuneEnergyOrbAnimationComplete();
    }
}

void Character::SetOrcMode(bool enabled) {
    if (m_animation) {
        m_animation->SetOrcMode(enabled);
    }
    if (enabled) {
        m_isDead = false;
        if (m_movement) {
            m_movement->ResetDieState();
        }
        
        SetGunMode(false);
        SetGrenadeMode(false);
        if (m_movement) {
            m_movement->SetNoClipNoGravity(false);
            m_movement->SetInputLocked(false);
            m_movement->SetLadderDoubleTapEnabled(false);
            m_movement->SetLadderEnabled(false);
        }
        if (m_animation) {
            Vector3 pos = GetPosition();
            m_animation->TriggerOrcAppearEffectAt(pos.x, pos.y);
        }
    } else {
        if (m_movement) {
            m_movement->SetLadderDoubleTapEnabled(true);
            m_movement->SetLadderEnabled(true);
        }
    }
}

bool Character::IsOrc() const {
    return m_animation ? m_animation->IsOrc() : false;
}

void Character::TriggerOrcMeteorStrike() {
    if (m_animation) {
        m_animation->TriggerOrcMeteorStrike();
    }
}

void Character::TriggerOrcFlameBurst() {
    if (m_animation) {
        m_animation->TriggerOrcFlameBurst();
    }
}

void Character::Initialize(std::shared_ptr<AnimationManager> animManager, int objectId) {
    if (m_animation) {
        m_animation->Initialize(animManager, objectId);
    }
    
    Object* originalObj = SceneManager::GetInstance()->GetObject(objectId);
    if (originalObj) {
        const Vector3& originalPos = originalObj->GetPosition();
        m_movement->Initialize(originalPos.x, originalPos.y, -1000.0f);
    } else {
        m_movement->Initialize(0.0f, 0.0f, 0.0f);
    }
    
    if (m_hitbox) {
        m_hitbox->Initialize(this, objectId);
    }
    
    if (m_movement) {
        m_movement->InitializeWallCollision();
        m_movement->InitializeLadderCollision();
        m_movement->InitializeTeleportCollision();
    }
}

void Character::ProcessInput(float deltaTime, InputManager* inputManager) {
    if (!inputManager) {
        return;
    }
    
    const bool* keyStates = inputManager->GetKeyStates();
    if (!keyStates) {
        return;
    }
    
    if (m_movement && m_combat) {
		bool lock = m_combat->IsKicking();
		
		if (ShouldBlockInput()) {
			lock = true;
			SetGunMode(false);
			SetGrenadeMode(false);
		}
		
		if (m_animation && m_animation->IsGunMode()) lock = true;
		if (m_animation && m_animation->IsGrenadeMode()) lock = true;
		if (m_animation && m_animation->IsOrcMeteorStrikeActive()) lock = true;
		if (m_animation && m_animation->IsGunMode() && m_movement->IsSitting()) {
			m_movement->ForceSit(false);
		}
		if (m_animation) {
			int cur = m_animation->GetCurrentAnimation();
			bool isAttackAnim = (cur >= 10 && cur <= 12) || (cur >= 17 && cur <= 22);
			if (m_animation->IsAnimationPlaying() && isAttackAnim) {
				lock = true;
			}
		}
		m_movement->SetInputLocked(lock);
    }
    
    bool allowPlatformFallThrough = !IsGunMode() && !IsGrenadeMode();
    
    m_movement->UpdateWithHurtbox(deltaTime, keyStates, 
                                  GetHurtboxWidth(), GetHurtboxHeight(), 
                                  GetHurtboxOffsetX(), GetHurtboxOffsetY(), 
                                  allowPlatformFallThrough);
    
    if (IsSpecialForm() && m_movement) {
        (void)m_movement->ConsumePendingFallDamage();
        (void)m_movement->ConsumeHardLandingRequested();
    }
    
    if (m_animation) {
        m_animation->HandleMovementAnimations(keyStates, m_movement.get(), m_combat.get());
    }

    if (m_movement) {
        bool hardLandingNow = (m_animation && m_animation->IsHardLandingActive());
        if (hardLandingNow && !m_prevHardLandingActive) {
            float pendingDamage = m_movement->ConsumePendingFallDamage();
            if (pendingDamage > 0.0f) {
                SoundManager::Instance().PlaySFXByID(9, 0);
                float prevHealth = GetHealth();
                TakeDamage(pendingDamage, false);
                if (prevHealth > 0.0f && GetHealth() <= 0.0f) {
                    if (m_selfDeathCallback) {
                        m_selfDeathCallback(*this);
                    } else {
                        TriggerDie();
                    }
                }
            }
        }
        m_prevHardLandingActive = hardLandingNow;
    }
    
    const PlayerInputConfig& inputConfig = m_movement->GetInputConfig();
    
    bool punchDown = inputManager->IsKeyJustPressed(inputConfig.punchKey);
    if (m_combat) m_combat->SetAttackPressed(punchDown);
    if (m_animation && (m_animation->IsBatDemon() || m_animation->IsKitsune())) {
        if (punchDown) {
            m_animation->TriggerBatDemonSlash();
        }
    } else if (!m_movement->IsInputLocked() && !IsGrenadeMode() && !ShouldBlockInput() && punchDown) {
        if (m_suppressNextPunch) {
            m_suppressNextPunch = false;
        } else {
        if (m_movement->IsJumping()) {
            HandleAirKick();
        } else {
        switch (m_weapon) {
            case WeaponType::Axe:   HandleAxeCombo(); break;
            case WeaponType::Sword: HandleSwordCombo(); break;
            case WeaponType::Pipe:  HandlePipeCombo(); break;
            case WeaponType::None:
            default: HandlePunchCombo(); break;
        }
        }
        }
    }
    
    if (!(m_animation && (m_animation->IsBatDemon() || m_animation->IsKitsune())) && !m_movement->IsInputLocked() && !IsGrenadeMode() && !ShouldBlockInput() && inputManager->IsKeyJustPressed(inputConfig.kickKey)) {
        if (!m_movement->IsJumping()) {
        HandleKick();
        }
    }

    if (!ShouldBlockInput() && keyStates[inputConfig.sitKey]) {
        if (m_combat && (m_combat->IsKicking() || m_combat->IsInCombo() || m_combat->IsInAxeCombo())) {
            m_combat->CancelAllCombos();
        }
    }
    
    if (!ShouldBlockInput() && inputManager->IsKeyJustPressed(inputConfig.dieKey)) {
        HandleDie();
    }
	if (m_movement && m_combat) {
		bool lock = m_combat->IsKicking();
		
		if (ShouldBlockInput()) {
			lock = true;
		}
		
		if (m_animation && m_animation->IsGunMode()) lock = true;
		if (m_animation && m_animation->IsGrenadeMode()) lock = true;
		if (m_animation && m_animation->IsOrcMeteorStrikeActive()) lock = true;
		if (m_animation) {
			int cur = m_animation->GetCurrentAnimation();
			bool isAttackAnim = (cur >= 10 && cur <= 12) || (cur >= 17 && cur <= 22);
			if (m_animation->IsAnimationPlaying() && isAttackAnim) {
				lock = true;
			}
		}
		m_movement->SetInputLocked(lock);
	}
}

void Character::Update(float deltaTime) {
    if (m_animation) {
        m_animation->Update(deltaTime, m_movement.get(), m_combat.get());
    }
    
    if (m_combat) {
        m_combat->Update(deltaTime);
        float dx = m_combat->ConsumeLungeDelta();
        if (dx != 0.0f) {
            Vector3 p = m_movement->GetPosition();
            m_movement->SetPosition(p.x + dx, p.y);
        }
    }

	if (m_movement && m_combat) {
		bool lock = m_combat->IsKicking();
		if (m_animation && m_animation->IsOrcMeteorStrikeActive()) lock = true;
		if (m_animation) {
			int cur = m_animation->GetCurrentAnimation();
			bool isAttackAnim = (cur >= 10 && cur <= 12) || (cur >= 17 && cur <= 22);
			if (m_animation->IsAnimationPlaying() && isAttackAnim) {
				lock = true;
			}
		}
		m_movement->SetInputLocked(lock);
	}

    UpdateStamina(deltaTime);
    UpdateAutoHeal(deltaTime);

    if (m_movement) {
        bool allowRun = true;
        if (!IsSpecialForm()) {
            allowRun = (m_stamina > 0.0f);
        }
        m_movement->SetAllowRun(allowRun);
    }
}

void Character::Draw(Camera* camera) {
    if (m_animation) {
        m_animation->Draw(camera, m_movement.get());
    }
    
    if (m_hitbox) {
        m_hitbox->DrawHitboxAndHurtbox(camera);
    }
}



void Character::CancelCombosOnOtherAction(const bool* keyStates) {
}

void Character::SetPosition(float x, float y) {
    m_movement->SetPosition(x, y);
}

Vector3 Character::GetPosition() const {
    return m_movement->GetPosition();
}

bool Character::IsFacingLeft() const {
    return m_movement->IsFacingLeft();
}

void Character::SetFacingLeft(bool facingLeft) {
    m_movement->SetFacingLeft(facingLeft);
}

CharState Character::GetState() const {
    return m_movement->GetState();
}

bool Character::IsJumping() const {
    return m_movement->IsJumping();
}

bool Character::IsSitting() const {
    return m_movement->IsSitting();
}

void Character::SetInputConfig(const PlayerInputConfig& config) {
    m_movement->SetInputConfig(config);
}

void Character::HandlePunchCombo() {
    if (m_combat && m_animation) {
        m_combat->HandlePunchCombo(m_animation.get(), m_movement.get());
    }
}

void Character::HandleAxeCombo() {
    if (m_combat && m_animation) {
        m_combat->HandleWeaponCombo(m_animation.get(), m_movement.get(), 20, 21, 22);
    }
}

void Character::HandleSwordCombo() {
    if (m_combat && m_animation) {
        m_combat->HandleWeaponCombo(m_animation.get(), m_movement.get(), 23, 24, 25);
    }
}

void Character::HandlePipeCombo() {
    if (m_combat && m_animation) {
        m_combat->HandleWeaponCombo(m_animation.get(), m_movement.get(), 26, 27, 28);
    }
}

void Character::HandleKick() {
    if (m_combat && m_animation) {
        m_combat->HandleKick(m_animation.get(), m_movement.get());
    }
}

void Character::HandleAirKick() {
    if (m_combat && m_animation && m_movement && m_movement->IsJumping()) {
        m_combat->HandleAirKick(m_animation.get(), m_movement.get());
    }
}

void Character::HandleDie() {
    if (m_movement && !m_movement->IsDying() && !m_movement->IsDead()) {
        m_movement->TriggerDie(false);
    }
}

void Character::TriggerDie() {
    if (m_movement && !m_movement->IsDying() && !m_movement->IsDead()) {
        m_movement->TriggerDie(false);
    }
}

void Character::TriggerDieFromAttack(const Character& attacker) {
    if (m_movement && !m_movement->IsDying() && !m_movement->IsDead()) {
        bool attackerFacingLeft = attacker.IsFacingLeft();
        m_movement->TriggerDie(attackerFacingLeft);
    }
}

void Character::HandleRandomGetHit() {
    if (m_combat && m_animation) {
        m_combat->HandleRandomGetHit(m_animation.get(), m_movement.get());
    }
}

void Character::CancelAllCombos() {
    if (m_combat) {
        m_combat->CancelAllCombos();
    }
}

void Character::PlayAnimation(int animIndex, bool loop) {
    if (m_animation) {
        m_animation->PlayAnimation(animIndex, loop);
    }
}

int Character::GetCurrentAnimation() const {
    return m_animation ? m_animation->GetCurrentAnimation() : -1;
}

bool Character::IsAnimationPlaying() const {
    return m_animation ? m_animation->IsAnimationPlaying() : false;
}

void Character::GetCurrentFrameUV(float& u0, float& v0, float& u1, float& v1) const {
    if (m_animation) {
        m_animation->GetCurrentFrameUV(u0, v0, u1, v1);
    } else {
        u0 = 0.0f; v0 = 0.0f; u1 = 1.0f; v1 = 1.0f;
    }
}

void Character::GetTopFrameUV(float& u0, float& v0, float& u1, float& v1) const {
    if (m_animation) {
        m_animation->GetTopFrameUV(u0, v0, u1, v1);
    } else {
        u0 = 0.0f; v0 = 0.0f; u1 = 1.0f; v1 = 1.0f;
    }
}

int Character::GetHeadTextureId() const {
    if (m_animation) return m_animation->GetHeadTextureId();
    return 8;
}

int Character::GetBodyTextureId() const {
    if (m_animation) return m_animation->GetBodyTextureId();
    return 10;
}

float Character::GetHeadOffsetX() const {
    if (m_animation) return m_animation->GetTopOffsetX();
    return 0.0f;
}

float Character::GetHeadOffsetY() const {
    if (m_animation) return m_animation->GetTopOffsetY();
    return 0.0f;
}

bool Character::IsInCombo() const {
    return m_combat ? m_combat->IsInCombo() : false;
}

int Character::GetComboCount() const {
    return m_combat ? m_combat->GetComboCount() : 0;
}

float Character::GetComboTimer() const {
    return m_combat ? m_combat->GetComboTimer() : 0.0f;
}

bool Character::IsComboCompleted() const {
    return m_combat ? m_combat->IsComboCompleted() : false;
}

bool Character::IsInAxeCombo() const {
    return m_combat ? m_combat->IsInAxeCombo() : false;
}

int Character::GetAxeComboCount() const {
    return m_combat ? m_combat->GetAxeComboCount() : 0;
}

float Character::GetAxeComboTimer() const {
    return m_combat ? m_combat->GetAxeComboTimer() : 0.0f;
}

bool Character::IsAxeComboCompleted() const {
    return m_combat ? m_combat->IsAxeComboCompleted() : false;
}

bool Character::IsKicking() const {
    return m_combat ? m_combat->IsKicking() : false;
}

bool Character::IsHit() const {
    return m_combat ? m_combat->IsHit() : false;
}

bool Character::IsHitboxActive() const {
    return m_combat ? m_combat->IsHitboxActive() : false;
}



void Character::DrawHitbox(Camera* camera, bool forceShow) {
    if (m_hitbox) {
        m_hitbox->DrawHitbox(camera, forceShow);
    }
}

void Character::DrawHitboxAndHurtbox(Camera* camera) {
    if (m_hitbox) {
        m_hitbox->DrawHitboxAndHurtbox(camera);
    }
}

void Character::SetHurtbox(float width, float height, float offsetX, float offsetY) {
    if (m_hitbox) {
        m_hitbox->SetHurtbox(width, height, offsetX, offsetY);
    }
}

void Character::SetHurtboxDefault(float width, float height, float offsetX, float offsetY) {
    if (m_hitbox) {
        m_hitbox->SetHurtboxDefault(width, height, offsetX, offsetY);
    }
}
void Character::SetHurtboxFacingLeft(float width, float height, float offsetX, float offsetY) {
    if (m_hitbox) {
        m_hitbox->SetHurtboxFacingLeft(width, height, offsetX, offsetY);
    }
}
void Character::SetHurtboxFacingRight(float width, float height, float offsetX, float offsetY) {
    if (m_hitbox) {
        m_hitbox->SetHurtboxFacingRight(width, height, offsetX, offsetY);
    }
}
void Character::SetHurtboxCrouchRoll(float width, float height, float offsetX, float offsetY) {
    if (m_hitbox) {
        m_hitbox->SetHurtboxCrouchRoll(width, height, offsetX, offsetY);
    }
}

void Character::DrawHurtbox(Camera* camera, bool forceShow) {
    if (m_hitbox) {
        m_hitbox->DrawHurtbox(camera, forceShow);
    }
}

float Character::GetHurtboxWidth() const {
    return m_hitbox ? m_hitbox->GetHurtboxWidth() : 0.0f;
}

float Character::GetHurtboxHeight() const {
    return m_hitbox ? m_hitbox->GetHurtboxHeight() : 0.0f;
}

float Character::GetHurtboxOffsetX() const {
    return m_hitbox ? m_hitbox->GetHurtboxOffsetX() : 0.0f;
}

float Character::GetHurtboxOffsetY() const {
    return m_hitbox ? m_hitbox->GetHurtboxOffsetY() : 0.0f;
}

bool Character::IsWerewolfComboActive() const {
    if (m_animation) {
        return m_animation->IsWerewolf() && (m_animation->GetCurrentAnimation() == 1) && m_animation->IsAnimationPlaying();
    }
    return false;
}

bool Character::IsWerewolfPounceActive() const {
    if (m_animation) {
        return m_animation->IsWerewolf() && (m_animation->GetCurrentAnimation() == 3) && m_animation->IsAnimationPlaying();
    }
    return false;
}

void Character::SetWerewolfHurtboxIdle(float w, float h, float ox, float oy)   { if (m_hitbox) m_hitbox->SetWerewolfHurtboxIdle(w, h, ox, oy); }
void Character::SetWerewolfHurtboxWalk(float w, float h, float ox, float oy)   { if (m_hitbox) m_hitbox->SetWerewolfHurtboxWalk(w, h, ox, oy); }
void Character::SetWerewolfHurtboxRun(float w, float h, float ox, float oy)    { if (m_hitbox) m_hitbox->SetWerewolfHurtboxRun(w, h, ox, oy); }
void Character::SetWerewolfHurtboxJump(float w, float h, float ox, float oy)   { if (m_hitbox) m_hitbox->SetWerewolfHurtboxJump(w, h, ox, oy); }
void Character::SetWerewolfHurtboxCombo(float w, float h, float ox, float oy)  { if (m_hitbox) m_hitbox->SetWerewolfHurtboxCombo(w, h, ox, oy); }
void Character::SetWerewolfHurtboxPounce(float w, float h, float ox, float oy) { if (m_hitbox) m_hitbox->SetWerewolfHurtboxPounce(w, h, ox, oy); }

void Character::SetKitsuneHurtboxIdle(float w, float h, float ox, float oy)      { if (m_hitbox) m_hitbox->SetKitsuneHurtboxIdle(w, h, ox, oy); }
void Character::SetKitsuneHurtboxWalk(float w, float h, float ox, float oy)      { if (m_hitbox) m_hitbox->SetKitsuneHurtboxWalk(w, h, ox, oy); }
void Character::SetKitsuneHurtboxRun(float w, float h, float ox, float oy)       { if (m_hitbox) m_hitbox->SetKitsuneHurtboxRun(w, h, ox, oy); }
void Character::SetKitsuneHurtboxEnergyOrb(float w, float h, float ox, float oy) { if (m_hitbox) m_hitbox->SetKitsuneHurtboxEnergyOrb(w, h, ox, oy); }

void Character::SetOrcHurtboxIdle  (float w, float h, float ox, float oy) { if (m_hitbox) m_hitbox->SetOrcHurtboxIdle(w, h, ox, oy); }
void Character::SetOrcHurtboxWalk  (float w, float h, float ox, float oy) { if (m_hitbox) m_hitbox->SetOrcHurtboxWalk(w, h, ox, oy); }
void Character::SetOrcHurtboxMeteor(float w, float h, float ox, float oy) { if (m_hitbox) m_hitbox->SetOrcHurtboxMeteor(w, h, ox, oy); }
void Character::SetOrcHurtboxFlame (float w, float h, float ox, float oy) { if (m_hitbox) m_hitbox->SetOrcHurtboxFlame(w, h, ox, oy); }

float Character::GetHitboxWidth() const {
    return m_combat ? m_combat->GetHitboxWidth() : 0.0f;
}

float Character::GetHitboxHeight() const {
    return m_combat ? m_combat->GetHitboxHeight() : 0.0f;
}

float Character::GetHitboxOffsetX() const {
    return m_combat ? m_combat->GetHitboxOffsetX() : 0.0f;
}

float Character::GetHitboxOffsetY() const {
    return m_combat ? m_combat->GetHitboxOffsetY() : 0.0f;
}

bool Character::CheckHitboxCollision(const Character& other) const {
    if (!m_combat) {
        return false;
    }
    return m_combat->CheckHitboxCollision(*this, other);
}

void Character::TriggerGetHit(const Character& attacker) {
    if (m_combat && m_animation) {
        SetFacingLeft(!attacker.IsFacingLeft());
        m_combat->TriggerGetHit(m_animation.get(), attacker, this);
    }
} 

void Character::TakeDamage(float damage, bool playSfx) {
    if (m_isDead) return;
    if (IsSpecialForm()) return;
    
    m_health -= damage;
    if (m_health < 0.0f) {
        m_health = 0.0f;
    }
    
    m_lastDamageTime = 0.0f;
    m_isHealing = false;
    
    if (playSfx) {
        SoundManager::Instance().PlaySFXByID(6, 0);
    }
    
    if (m_health <= 0.0f && !m_isDead) {
        m_isDead = true;
    }
}

void Character::Heal(float amount) {
    if (m_isDead) return;
    
    m_health += amount;
    if (m_health > MAX_HEALTH) {
        m_health = MAX_HEALTH;
    }
}

void Character::ResetHealth() {
    m_health = MAX_HEALTH;
    m_isDead = false;
    
    m_lastDamageTime = 0.0f;
    m_isHealing = false;
} 

bool Character::IsSpecialForm() const {
    return (IsWerewolf() || IsBatDemon() || IsKitsune() || IsOrc());
}

bool Character::ShouldBlockInput() const {
    if (m_isDead) return true;
    if (m_movement && m_movement->IsDying()) return true;
    if (m_animation && m_animation->IsHardLandingActive()) return true;
    
    if (m_animation) {
        int currentAnim = m_animation->GetCurrentAnimation();
        if (currentAnim == 8 || currentAnim == 9) { // GetHit1, GetHit2
            return true;
        }
    }
    return false;
}

void Character::UpdateStamina(float deltaTime) {
    if (!m_movement || IsSpecialForm()) {
        return;
    }

    const float runDrainPerSecond = MAX_STAMINA / 5.0f;
    const float regenPerSecond    = MAX_STAMINA / 3.0f;
    const float jumpCost = 5.0f;
    const float rollCost = 5.0f;

    bool isRunning = (m_movement->IsRunningLeft() || m_movement->IsRunningRight());
    bool isJumping = m_movement->IsJumping();
    bool isRolling = m_movement->IsRolling();

    if (isRunning) {
        m_stamina -= runDrainPerSecond * deltaTime;
    }

    bool isJumpingNow = m_movement->IsJumping();
    if (isJumpingNow && !m_prevJumpingForStamina) {
        m_stamina -= jumpCost;
    }
    if (isRolling && !m_prevRolling) {
        m_stamina -= rollCost;
    }
    m_prevRolling = isRolling;
    m_prevJumpingForStamina = isJumpingNow;

    if (!isRunning && !isJumping && !isRolling) {
        m_stamina += regenPerSecond * deltaTime;
    }

    if (m_stamina < 0.0f) m_stamina = 0.0f;
    if (m_stamina > MAX_STAMINA) m_stamina = MAX_STAMINA;
}

void Character::UpdateAutoHeal(float deltaTime) {
    if (m_isDead || IsSpecialForm()) {
        return;
    }
    
    float timeSinceLastDamage = m_lastDamageTime;
    if (timeSinceLastDamage >= m_healStartDelay) {
        if (!m_isHealing) {
            m_isHealing = true;
        }
        
        if (m_isHealing && m_health < MAX_HEALTH) {
            m_health += m_healRate * deltaTime;
            if (m_health > MAX_HEALTH) {
                m_health = MAX_HEALTH;
                m_isHealing = false;
            }
        }
    } else {
        m_isHealing = false;
    }
    
    m_lastDamageTime += deltaTime;
}

Vector3 Character::GetGunTopWorldPosition() const {
    if (m_animation && m_movement) {
        return m_animation->GetTopWorldPosition(m_movement.get());
    }
    return GetPosition();
}

float Character::GetAimAngleDeg() const {
    if (m_animation) {
        return m_animation->GetAimAngleDeg();
    }
    return 0.0f;
}

void Character::MarkGunShotFired() {
    if (m_animation) {
        m_animation->OnGunShotFired(m_movement.get());
    }
}