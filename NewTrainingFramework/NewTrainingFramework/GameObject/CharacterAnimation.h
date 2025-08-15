#pragma once
#include <memory>
#include "../../Utilities/Math.h"

class AnimationManager;

class Camera;
class Object;
class CharacterMovement;
struct PlayerInputConfig;
class CharacterCombat;

class CharacterAnimation {
private:
    std::shared_ptr<AnimationManager> m_animManager;
    std::unique_ptr<class Object> m_characterObject;
    int m_lastAnimation;
    int m_objectId;

    std::shared_ptr<AnimationManager> m_topAnimManager;
    std::unique_ptr<class Object> m_topObject;
    int m_lastTopAnimation = -1;
    bool m_gunMode = false;
    bool m_gunEntering = false;
    unsigned int m_gunEnterStartMs = 0;
    static constexpr float GUN_ENTER_DURATION = 0.12f;
    bool m_syncFacingOnEnter = false;
    float m_topOffsetX = -0.019f;
    float m_topOffsetY = -0.025f;

    int m_gunTopAnimReverse = 0; 
    int m_gunTopAnimHold    = 1; 
    int m_gunTopAnimReload  = -1;

    float m_climbHoldTimer = 0.0f;
    static constexpr float CLIMB_HOLD_STEP_INTERVAL = 0.12f;
    int m_lastClimbDir = 0;
    bool m_prevClimbUpPressed = false;
    bool m_prevClimbDownPressed = false;
    float m_downPressStartTime = -1.0f;
    static constexpr float CLIMB_DOWN_HOLD_THRESHOLD = 0.15f;

    // Turning control for gun mode
    bool m_isTurning = false;
    bool m_turnTargetLeft = false;
    float m_turnTimer = 0.0f;
    static constexpr float TURN_DURATION = 0.12f;
    bool m_turnInitialLeft = false;
    bool m_prevFacingLeft = false;

    float m_aimAngleDeg = 0.0f;
    bool m_prevAimUp = false;
    bool m_prevAimDown = false;
    float m_aimHoldTimerUp = 0.0f;
    float m_aimHoldTimerDown = 0.0f;
    static constexpr float AIM_HOLD_STEP_INTERVAL = 0.03f;
    static constexpr float AIM_HOLD_INITIAL_DELAY = 0.10f;
    float m_aimSincePressUp = 0.0f;
    float m_aimSincePressDown = 0.0f;
    unsigned int m_lastAimTickMs = 0;
    unsigned int m_aimHoldBlockUntilMs = 0;

    // Sticky aim after shot
    float m_lastShotAimDeg = 0.0f;
    unsigned int m_lastShotTickMs = 0;
    static constexpr unsigned int STICKY_AIM_WINDOW_MS = 600;

    bool m_recoilActive = false;
    float m_recoilTimer = 0.0f;
    float m_recoilOffsetX = 0.0f;
    float m_recoilOffsetY = 0.0f;
    float m_recoilFaceSign = 1.0f;
    static constexpr float RECOIL_DURATION = 0.09f;
    static constexpr float RECOIL_STRENGTH = 0.03f;
    float m_recoilStrengthMul = 1.0f;
    bool m_reloadActive = false;
    float m_reloadTimer = 0.0f;
    static constexpr float RELOAD_DURATION = 0.30f;

    // Grenade throw visual mode
    bool m_grenadeMode = false;
    bool m_isBatDemon = false;
    bool m_isWerewolf = false;
    bool m_isKitsune = false;
    bool m_isOrc = false;
    bool m_batSlashActive = false;
    float m_batSlashCooldownTimer = 0.0f;
    static constexpr float BAT_SLASH_COOLDOWN = 0.5f;
    bool m_kitsuneEnergyOrbActive = false;
    float m_kitsuneEnergyOrbCooldownTimer = 0.0f;
    static constexpr float KITSUNE_ENERGY_ORB_COOLDOWN = 0.5f;
    bool m_kitsuneEnergyOrbAnimationComplete = false;
    bool m_orcMeteorStrikeActive = false;
    bool m_orcFlameBurstActive = false;
    bool m_orcFireActive = false;
    std::shared_ptr<AnimationManager> m_orcFireAnim;
    std::unique_ptr<class Object> m_orcFireObject;
    bool m_orcAppearActive = false;
    std::shared_ptr<AnimationManager> m_orcAppearAnim;
    std::unique_ptr<class Object> m_orcAppearObject;
    static constexpr float ORC_APPEAR_Y_OFFSET = 0.19f;
    bool m_werewolfComboActive = false;
    bool m_werewolfPounceActive = false;
    float m_werewolfBodyOffsetY = 0.0f;
    float m_werewolfAirTimer = 0.0f;
    static constexpr float WEREWOLF_AIR_DEBOUNCE = 0.06f;
    float m_werewolfPounceSpeed = 1.0f;
    float m_werewolfPounceCooldownTimer = 0.0f;
    float m_werewolfPounceCooldown = 0.5f;
    float m_werewolfPounceHitWindowTimer = 0.0f;
    float m_werewolfPounceHitWindow = 0.12f;
    // Combo cooldown
    float m_werewolfComboCooldownTimer = 0.0f;
    float m_werewolfComboCooldown = 0.5f;
    float m_werewolfComboHitWindowTimer = 0.0f;
    float m_werewolfComboHitWindow = 0.15f;

    // Helper methods
    void UpdateAnimationState(CharacterMovement* movement, CharacterCombat* combat);

    bool m_hardLandingActive = false;
    int  m_hardLandingPhase = 0; // 0 = Lie, 1 = Land
    bool m_restoreInputAfterHardLanding = false;
    unsigned int m_blockHardLandingUntilMs = 0;

public:
    // Movement animation handling
    void HandleMovementAnimations(const bool* keyStates, CharacterMovement* movement, CharacterCombat* combat);
    CharacterAnimation();
    ~CharacterAnimation();
    
    // Initialization
    void Initialize(std::shared_ptr<AnimationManager> animManager, int objectId);
    
    // Core update
    void Update(float deltaTime, CharacterMovement* movement, CharacterCombat* combat);
    void Draw(Camera* camera, CharacterMovement* movement);
    void DrawOrcFire(Camera* camera);
    void DrawOrcAppear(Camera* camera);
    
    // Animation control
    void PlayAnimation(int animIndex, bool loop);
    void PlayTopAnimation(int animIndex, bool loop);
    int GetCurrentAnimation() const;
    bool IsAnimationPlaying() const;
    void GetCurrentFrameUV(float& u0, float& v0, float& u1, float& v1) const;
    // Gun helpers
    Vector3 GetTopWorldPosition(CharacterMovement* movement) const;
    float GetAimAngleDeg() const { return m_aimAngleDeg; }
    
    // Getters
    int GetObjectId() const { return m_objectId; }
    Object* GetCharacterObject() const { return m_characterObject.get(); }
    
    // Facing direction (needed for hitbox calculations)
    bool IsFacingLeft(CharacterMovement* movement) const;

    // Gun/overlay control
    void SetGunMode(bool enabled);
    bool IsGunMode() const { return m_gunMode; }
    void SetTopOffset(float ox, float oy) { m_topOffsetX = ox; m_topOffsetY = oy; }

    void OnGunShotFired(CharacterMovement* movement = nullptr);

    // Set current gun by texture id
    void SetGunByTextureId(int texId);

    void SetGrenadeMode(bool enabled);
    bool IsGrenadeMode() const { return m_grenadeMode; }
    // BatDemon mode control
    void SetBatDemonMode(bool enabled);
    bool IsBatDemon() const { return m_isBatDemon; }
    void TriggerBatDemonSlash();
    bool IsBatDemonSlashActive() const { return m_batSlashActive; }
    float GetBatSlashCooldownTimer() const { return m_batSlashCooldownTimer; }
    // Werewolf mode control
    void SetWerewolfMode(bool enabled);
    bool IsWerewolf() const { return m_isWerewolf; }
    void TriggerWerewolfCombo();
    void TriggerWerewolfPounce();
    void SetWerewolfBodyOffsetY(float oy) { m_werewolfBodyOffsetY = oy; }
    void SetWerewolfPounceSpeed(float v) { m_werewolfPounceSpeed = v; }
    void SetWerewolfPounceCooldown(float seconds) { m_werewolfPounceCooldown = seconds; }
    void SetWerewolfComboCooldown(float seconds) { m_werewolfComboCooldown = seconds; }
    bool IsWerewolfComboHitWindowActive() const { return m_werewolfComboHitWindowTimer > 0.0f; }
    bool IsWerewolfPounceHitWindowActive() const { return m_werewolfPounceHitWindowTimer > 0.0f; }

    void GetTopFrameUV(float& u0, float& v0, float& u1, float& v1) const;
    int GetHeadTextureId() const { return (m_objectId == 1000) ? 8 : 9; }
    int GetBodyTextureId() const { return m_isBatDemon ? 61 : (m_isWerewolf ? 60 : (m_isKitsune ? 62 : (m_isOrc ? 63 : ((m_objectId == 1000) ? 10 : 11)))); }
    float GetTopOffsetX() const { return m_topOffsetX; }
    float GetTopOffsetY() const { return m_topOffsetY; }

    bool IsHardLandingActive() const { return m_hardLandingActive; }
    void StartHardLanding(class CharacterMovement* movement);

    // Kitsune mode control
    void SetKitsuneMode(bool enabled);
    bool IsKitsune() const { return m_isKitsune; }
    void TriggerKitsuneEnergyOrb();
    bool IsKitsuneEnergyOrbActive() const { return m_kitsuneEnergyOrbActive; }
    float GetKitsuneEnergyOrbCooldownTimer() const { return m_kitsuneEnergyOrbCooldownTimer; }
    bool IsKitsuneEnergyOrbAnimationComplete() const { return m_kitsuneEnergyOrbAnimationComplete; }
    void ResetKitsuneEnergyOrbAnimationComplete() { m_kitsuneEnergyOrbAnimationComplete = false; }

    // Orc mode control
    void SetOrcMode(bool enabled);
    bool IsOrc() const { return m_isOrc; }
    void TriggerOrcMeteorStrike();
    void TriggerOrcFlameBurst();
    bool IsOrcMeteorStrikeActive() const { return m_orcMeteorStrikeActive; }
    bool IsOrcFlameBurstActive() const { return m_orcFlameBurstActive; }
    bool IsOrcFireActive() const { return m_orcFireActive; }
    void GetOrcFireAabb(float& left, float& right, float& bottom, float& top) const;
    void TriggerOrcAppearEffectAt(float x, float y);

private:
    void StartTurn(bool toLeft, bool initialLeft);
    void HandleGunAim(const bool* keyStates, const PlayerInputConfig& inputConfig);
}; 