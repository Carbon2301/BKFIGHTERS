#pragma once
#include "GameStateBase.h"
#include "SceneManager.h"
#include "../GameObject/AnimationManager.h"
#include "../GameObject/Character.h"
#include "../GameObject/InputManager.h"
#include "../GameObject/WallCollision.h"
#include "../GameObject/EnergyOrbProjectile.h"
#include "../../Utilities/Math.h"
#include <vector>
#include <unordered_map>

class GSPlay : public GameStateBase {
private:
    float m_gameTime;
    
    static const int MENU_BUTTON_ID = 301;
    static const int ANIM_OBJECT_ID = 1000;
    std::shared_ptr<AnimationManager> m_animManager;
    
    static bool s_showHitboxHurtbox;
    static bool s_showPlatformBoxes;
    static bool s_showWallBoxes;
    static bool s_showLadderBoxes;
    static bool s_showTeleportBoxes;
    
    // Health system variables
    float m_player1Health;
    float m_player2Health;
    const float MAX_HEALTH = 100.0f;
    const float HEALTH_DAMAGE = 10.0f; // Máu mất mỗi lần nhấn T
    
    // Cloud movement system
    float m_cloudSpeed;
    struct Bullet {
        float x; float y;
        float vx; float vy;
        float life; int objIndex;
        float angleRad; float faceSign;
        int ownerId; float damage;
        bool isBazoka = false;
        float trailTimer = 0.0f;
    };
    std::vector<Bullet> m_bullets;
    static constexpr int MAX_BULLETS = 1024;
    const float BULLET_SPEED = 3.5f;
    const float BULLET_LIFETIME = 2.0f;
    const float BULLET_SPAWN_OFFSET_X = 0.12f;
    const float BULLET_SPAWN_OFFSET_Y = -0.01f;
    const float BULLET_COLLISION_WIDTH  = 0.02f;
    const float BULLET_COLLISION_HEIGHT = 0.02f;
    int m_bulletObjectId = 1300;
    int m_bazokaBulletObjectId = 1301;
    std::vector<std::unique_ptr<Object>> m_bulletObjs;
    std::vector<int> m_freeBulletSlots;
    int CreateOrAcquireBulletObject();
    int CreateOrAcquireBulletObjectFromProto(int protoObjectId);
    void DrawBullets(Camera* cam);
    
    void SpawnBulletFromCharacter(const Character& ch);
    void SpawnBulletFromCharacterWithJitter(const Character& ch, float jitterDeg);
    void SpawnBazokaBulletFromCharacter(const Character& ch, float jitterDeg, float speedMul, float damage);
    
    void UpdateBullets(float dt);
    void UpdateGunBursts();
    void UpdateGunReloads();

    // Bazoka trail ghosts
    struct Trail { float x; float y; float life; int objIndex; float angle; float alpha; };
    std::vector<Trail> m_bazokaTrails;
    static constexpr int MAX_BAZOKA_TRAILS = 2048;
    std::vector<std::unique_ptr<Object>> m_bazokaTrailObjs;
    std::vector<int> m_freeBazokaTrailSlots;
    int CreateOrAcquireBazokaTrailObject();
    const float BAZOKA_TRAIL_LIFETIME = 0.35f;
    const float BAZOKA_TRAIL_SPAWN_INTERVAL = 0.002f;
    const float BAZOKA_TRAIL_SCALE_X = 0.6f;
    const float BAZOKA_TRAIL_SCALE_Y = 0.6f;
    const float BAZOKA_TRAIL_BACK_OFFSET = 0.01f;
    std::vector<std::shared_ptr<class Texture2D>> m_bazokaTrailTextures;

    struct BloodDrop {
        float x; float y;
        float vx; float vy;
        float angle;
        int objIdx;
    };
    std::vector<BloodDrop> m_bloodDrops;
    std::vector<std::unique_ptr<Object>> m_bloodObjs;
    std::vector<int> m_freeBloodSlots;
    static constexpr float BLOOD_GRAVITY = 3.8f;
    static constexpr float BLOOD_COLLISION_WIDTH  = 0.015f;
    static constexpr float BLOOD_COLLISION_HEIGHT = 0.015f;
    int m_bloodProtoIdA = 1400;
    int m_bloodProtoIdB = 1401;
    int m_bloodProtoIdC = 1402;
    int CreateOrAcquireBloodObjectFromProto(int protoObjectId);
    void SpawnBloodAt(float x, float y, float baseAngleRad);
    void UpdateBloods(float dt);
    void DrawBloods(class Camera* cam);

     // Bomb system
     struct Bomb { float x; float y; float vx; float vy; float life; int objIndex; float angleRad; float faceSign; bool grounded = false; bool atRest = false; };
     std::vector<Bomb> m_bombs;
     std::vector<std::unique_ptr<Object>> m_bombObjs;
     std::vector<int> m_freeBombSlots;
     int m_bombObjectId = 1500;
     int CreateOrAcquireBombObjectFromProto(int protoObjectId);
     void SpawnBombFromCharacter(const Character& ch, float overrideLife = -1.0f);
     void UpdateBombs(float dt);
     void DrawBombs(class Camera* cam);
    static constexpr float BOMB_SPEED    = 0.8f;
    static constexpr float BOMB_GRAVITY  = 1.2f;
    static constexpr float BOMB_LIFETIME = 3.0f;
     static constexpr float BOMB_COLLISION_WIDTH  = 0.04f;
     static constexpr float BOMB_COLLISION_HEIGHT = 0.04f;
     static constexpr float BOMB_BOUNCE_DAMPING   = 0.55f;
     static constexpr float BOMB_WALL_FRICTION    = 0.95f;
     static constexpr float BOMB_GROUND_FRICTION  = 0.85f;
     static constexpr float BOMB_GROUND_DRAG      = 2.0f;
     static constexpr float BOMB_MIN_BOUNCE_SPEED = 0.15f;

     // Explosion system
     struct Explosion {
         float x; float y;
         int objIdx;
         int cols; int rows;
         int frameIndex; int frameCount;
         float frameTimer; float frameDuration;
         bool damagedP1 = false;
         bool damagedP2 = false;
     };
     std::vector<Explosion> m_explosions;
     std::vector<std::unique_ptr<Object>> m_explosionObjs;
     std::vector<int> m_freeExplosionSlots;
     int m_explosionObjectId = 1501;
     int CreateOrAcquireExplosionObjectFromProto(int protoObjectId);
     void SpawnExplosionAt(float x, float y);
     void UpdateExplosions(float dt);
     void DrawExplosions(class Camera* cam);
     void SetSpriteUV(Object* obj, int cols, int rows, int frameIndex);
     static constexpr float EXPLOSION_FRAME_DURATION = 0.05f; // seconds per frame
     void ApplyExplosionDamageAt(float x, float y, float halfW, float halfH, Explosion* e = nullptr);

    std::unique_ptr<WallCollision> m_wallCollision;
    // Grenade fuse tracking
    float m_p1GrenadePressTime = -1.0f;
    float m_p2GrenadePressTime = -1.0f;
    int   m_p1HeldBombObj = -1;
    int   m_p2HeldBombObj = -1;
    bool  m_p1GrenadeExplodedInHand = false;
    bool  m_p2GrenadeExplodedInHand = false;

    void UpdateGrenadeFuse();
    void UpdateHeldBombVisuals();
    void CreateHeldBombVisualFor(const Character& ch, int& objSlot);
    void RemoveHeldBombVisual(int& objSlot);

    bool m_p1ShotPending = false;
    bool m_p2ShotPending = false;
    float m_p1GunStartTime = -1.0f;
    float m_p2GunStartTime = -1.0f;
    static constexpr float GUN_MIN_REVERSE_TIME = 0.15f;
    static constexpr float GUN_MIN_HOLD_ANIM1   = 0.15f;
    float GetGunRequiredTime() const { return GUN_MIN_REVERSE_TIME + GUN_MIN_HOLD_ANIM1; }
    void TryCompletePendingShots();
    const float CLOUD_MOVE_SPEED = 0.2f;
    const float CLOUD_SPACING = 1.74f;
    const float CLOUD_LEFT_BOUNDARY = -6.0f;
    const int TOTAL_CLOUDS = 10;
    
    // HUD gun display
    int m_player1GunTexId = 40;
    int m_player2GunTexId = 40;
    Vector3 m_hudGun1BaseScale = Vector3(0.0f, 0.0f, 1.0f);
    Vector3 m_hudGun2BaseScale = Vector3(0.0f, 0.0f, 1.0f);

    static constexpr int   M4A1_BURST_COUNT = 5;
    static constexpr float M4A1_BURST_INTERVAL = 0.08f;
    bool  m_p1BurstActive = false;
    int   m_p1BurstRemaining = 0;
    float m_p1NextBurstTime = 0.0f;
    bool  m_p2BurstActive = false;
    int   m_p2BurstRemaining = 0;
    float m_p2NextBurstTime = 0.0f;

    // Energy Orb Projectiles
    std::vector<std::unique_ptr<EnergyOrbProjectile>> m_energyOrbProjectiles;
    static constexpr int MAX_ENERGY_ORB_PROJECTILES = 1000;
    void SpawnEnergyOrbProjectile(Character& character);
    void UpdateEnergyOrbProjectiles(float deltaTime);
    void DrawEnergyOrbProjectiles(class Camera* camera);
    void DetonatePlayerProjectiles(int playerId);
    
    struct LightningEffect {
        float x;
        float lifetime;
        float maxLifetime;
        int objectIndex;
        bool isActive;
        int currentFrame;
        float frameTimer;
        float frameDuration;
        
        float hitboxLeft;
        float hitboxRight;
        float hitboxTop;
        float hitboxBottom;
        bool hasDealtDamage;
    };
    std::vector<LightningEffect> m_lightningEffects;
    std::vector<std::unique_ptr<Object>> m_lightningObjects;
    std::vector<int> m_freeLightningSlots;
    static constexpr int MAX_LIGHTNING_EFFECTS = 1000;
    void SpawnLightningEffect(float x);
    void UpdateLightningEffects(float deltaTime);
    void DrawLightningEffects(class Camera* camera);
    int CreateOrAcquireLightningObject();
    void CheckLightningDamage();

    struct FireRain {
        bool isActive = false;
        bool isFading = false;
        float lifetime = 0.0f;
        float maxLifetime = 5.0f;
        float fadeTimer = 0.0f;
        float fadeDuration = 0.8f;
        Vector3 position = Vector3(0.0f, 0.0f, 0.0f);
        Vector3 velocity = Vector3(0.0f, -1.5f, 0.0f);
        int objectIndex = -1;
        std::shared_ptr<AnimationManager> anim;
        bool damagedP1 = false;
        bool damagedP2 = false;
    };
    std::vector<FireRain> m_fireRains;
    std::vector<std::unique_ptr<Object>> m_fireRainObjects;
    std::vector<int> m_freeFireRainSlots;
    static constexpr int MAX_FIRERAIN = 100000;
    static constexpr float FIRE_RAIN_COLLISION_W = 0.05f;
    static constexpr float FIRE_RAIN_COLLISION_H = 0.05f;
    static constexpr float FIRE_RAIN_DAMAGE_W = 0.18f;
    static constexpr float FIRE_RAIN_DAMAGE_H = 0.18f;
    int CreateOrAcquireFireRainObject();
    void SpawnFireRainAt(float x, float y);
    void UpdateFireRains(float deltaTime);
    void DrawFireRains(class Camera* camera);
    bool CheckFireRainWallCollision(const Vector3& pos, float halfW, float halfH) const;

    // Fire Rain spawn 
    struct FireRainEvent { float spawnTime; float x; };
    std::vector<FireRainEvent> m_fireRainSpawnQueue;
    void QueueFireRainWave(float xStart, float xEnd, float step, float y, float duration);
    void UpdateFireRainSpawnQueue();

    static constexpr float SHOTGUN_RELOAD_TIME = 0.30f;
    bool  m_p1ReloadPending = false;
    float m_p1ReloadExitTime = -1.0f;
    bool  m_p2ReloadPending = false;
    float m_p2ReloadExitTime = -1.0f;

    // Ammo per gun
    int m_p1Ammo40 = 15;  int m_p2Ammo40 = 15;   // Pistol
    int m_p1Ammo41 = 30;  int m_p2Ammo41 = 30;   // M4A1 (5 per shot)
    int m_p1Ammo42 = 60;  int m_p2Ammo42 = 60;   // Shotgun (5 per shot)
    int m_p1Ammo43 = 3;   int m_p2Ammo43 = 3;    // Bazoka
    
    int m_p1Ammo45 = 12;  int m_p2Ammo45 = 12;   // Deagle
    int m_p1Ammo46 = 6;   int m_p2Ammo46 = 6;    // Sniper
    int m_p1Ammo47 = 30;  int m_p2Ammo47 = 30;   // Uzi (5 per shot)

    void UpdateHudAmmoDigits();
    int& AmmoRefFor(int texId, bool isPlayer1);
    int  AmmoCostFor(int texId) const;
    int  AmmoCapacityFor(int texId) const;
    void TryUnequipIfEmpty(int texId, bool isPlayer1);
    int   m_p1HudAmmoShown = 0;
    int   m_p2HudAmmoShown = 0;
    int   m_p1HudAmmoTarget = 0;
    int   m_p2HudAmmoTarget = 0;
    float m_hudAmmoStepInterval = 0.05f;
    float m_p1HudAmmoNextTick = 0.0f;
    float m_p2HudAmmoNextTick = 0.0f;
    void StartHudAmmoAnimation(bool isPlayer1);
    void RefreshHudAmmoInstant(bool isPlayer1);
    void UpdateHudAmmoAnim(float deltaTime);
    
public:
    GSPlay();
    ~GSPlay();

    void Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void HandleKeyEvent(unsigned char key, bool bIsPressed) override;
    void HandleMouseEvent(int x, int y, bool bIsPressed) override;
    void HandleMouseMove(int x, int y) override;
    void Resume() override;
    void Pause() override;
    void Exit() override;
    void Cleanup() override;
    
    static bool IsShowHitboxHurtbox() { return s_showHitboxHurtbox; }
    static bool IsShowPlatformBoxes() { return s_showPlatformBoxes; }
    static bool IsShowWallBoxes() { return s_showWallBoxes; }
    static bool IsShowLadderBoxes() { return s_showLadderBoxes; }
    static bool IsShowTeleportBoxes() { return s_showTeleportBoxes; }
    
    // Health system methods
    void UpdateHealthBars();
    float GetPlayer1Health() const { return m_player1Health; }
    float GetPlayer2Health() const { return m_player2Health; }
    
    // Cloud movement system
    void UpdateCloudMovement(float deltaTime);
    
    // Fan rotation system
    void UpdateFanRotation(float deltaTime);

private:
    bool m_prevJumpingP1 = false;
    bool m_prevJumpingP2 = false;
    bool m_prevRollingP1 = false;
    bool m_prevRollingP2 = false;
    void DrawHudPortraits();
    void UpdateStaminaBars();
    
    void UpdateSpecialFormTimers();
    int  GetSpecialType(const class Character& ch) const; // 0=normal, 1=Werewolf, 2=BatDemon, 3=Kitsune, 4=Orc
    float m_p1SpecialExpireTime = -1.0f;
    float m_p2SpecialExpireTime = -1.0f;
    int   m_p1SpecialType = 0;
    int   m_p2SpecialType = 0;
    // HUD portrait
    Vector3 ComputeHudPortraitScale(const class Character& ch, const Vector3& baseScale) const;
    Vector3 ComputeHudPortraitOffset(const class Character& ch) const;
    float  m_portraitScaleMulWerewolf = 0.6f;
    float  m_portraitScaleMulBatDemon = 0.4f;
    float  m_portraitScaleMulKitsune  = 0.5f;
    float  m_portraitScaleMulOrc      = 0.5f;
    Vector3 m_portraitOffsetWerewolf = Vector3(0.0f, 0.01f, 0.0f);
    Vector3 m_portraitOffsetBatDemon = Vector3(0.0f, -0.03f, 0.0f);
    Vector3 m_portraitOffsetKitsune  = Vector3(0.0f, 0.01f, 0.0f);
    Vector3 m_portraitOffsetOrc      = Vector3(0.0f, 0.0f, 0.0f);
    // Item pickup system
    void HandleItemPickup();
    static const int AXE_OBJECT_ID = 1100;
    static const int SWORD_OBJECT_ID = 1101;
    static const int PIPE_OBJECT_ID  = 1102;
    static const int HUD_TEX_AXE   = 30;
    static const int HUD_TEX_SWORD = 31;
    static const int HUD_TEX_PIPE  = 32;
    bool m_isAxeAvailable = false;
    bool m_isSwordAvailable = false;
    bool m_isPipeAvailable  = false;
    
    // HUD weapons
    void UpdateHudWeapons();
    Vector3 m_hudWeapon1BaseScale = Vector3(0.0f, 0.0f, 1.0f);
    Vector3 m_hudWeapon2BaseScale = Vector3(0.0f, 0.0f, 1.0f);
    Vector3 m_hudAmmo1BaseScale = Vector3(0.042f, 0.055f, 1.0f);
    Vector3 m_hudAmmo2BaseScale = Vector3(0.042f, 0.055f, 1.0f);

    Vector3 m_hudSpecialTime1BaseScale = Vector3(0.0f, 0.0f, 1.0f);
    Vector3 m_hudSpecialTime2BaseScale = Vector3(0.0f, 0.0f, 1.0f);

    // HUD Special Character icons
    int m_p1SpecialItemTexId = -1;
    int m_p2SpecialItemTexId = -1;
    Vector3 m_hudSpecial1BaseScale = Vector3(0.07f, 0.07f, 1.0f);
    Vector3 m_hudSpecial2BaseScale = Vector3(0.07f, 0.07f, 1.0f);
    void UpdateHudSpecialIcon(bool isPlayer1);

    // Bomb HUD
    int m_p1Bombs = 3;
    int m_p2Bombs = 3;
    Vector3 m_hudBomb1BaseScale = Vector3(0.042f, 0.055f, 1.0f);
    Vector3 m_hudBomb2BaseScale = Vector3(0.042f, 0.055f, 1.0f);
    Vector3 m_hudBombIcon1BaseScale = Vector3(0.0f, 0.0f, 1.0f);
    Vector3 m_hudBombIcon2BaseScale = Vector3(0.0f, 0.0f, 1.0f);
    void UpdateHudBombDigits();

    // Item lifetime tracking
    struct ItemLife { int id; float timer; };
    std::vector<ItemLife> m_itemLives;

    // Random item
    struct ItemTemplate { int modelId; std::vector<int> textureIds; int shaderId; Vector3 scale; };
    struct SpawnSlot { Vector3 pos; int currentId = -1; float lifeTimer = 0.0f; float respawnTimer = 0.0f; bool active = false; };
    std::unordered_map<int, ItemTemplate> m_itemTemplates;
    std::vector<int> m_candidateItemIds;
    std::vector<SpawnSlot> m_spawnSlots;
    float m_itemBlinkTimer = 0.0f;
    void InitializeRandomItemSpawns();
    void UpdateRandomItemSpawns(float deltaTime);
    bool SpawnItemIntoSlot(int slotIndex, int itemId);
    int  ChooseRandomAvailableItemId();
}; 

