#include "stdafx.h"
#include <random>
#include "GSPlay.h"
#include "GameStateMachine.h"
#include "../Core/Globals.h"
#include "../../Utilities/Math.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <random>
#include "../GameObject/Object.h"
#include "../GameObject/Texture2D.h"
#include "../GameObject/Camera.h"
#include <memory>
#include "../GameObject/AnimationManager.h"
#include "../GameObject/Character.h"
#include "../GameObject/CharacterAnimation.h"
#include "../GameObject/CharacterMovement.h"
#include "../GameObject/InputManager.h"
#include "ResourceManager.h"
#include <fstream>
#include <sstream>
#include <SDL_ttf.h>
#include "SoundManager.h"

#define MENU_BUTTON_ID 301

// HURTBOX CONFIG 
namespace {
    struct HurtboxPreset { float w; float h; float ox; float oy; };
    // Player
    HurtboxPreset P1_HURTBOX_DEFAULT   { 0.088f, 0.13f,  -0.0088f, -0.038f };
    HurtboxPreset P1_HURTBOX_FACE_LEFT { 0.08f, 0.13f,  0.01f, -0.038f };
    HurtboxPreset P1_HURTBOX_FACE_RIGHT{ 0.088f, 0.13f,  -0.0088f, -0.038f };
    HurtboxPreset P1_HURTBOX_CROUCH    { 0.07f, 0.09f,  -0.0f, -0.053f };
    // Player 2
    HurtboxPreset P2_HURTBOX_DEFAULT   { 0.088f, 0.13f,  -0.0088f, -0.038f };
    HurtboxPreset P2_HURTBOX_FACE_LEFT { 0.08f, 0.13f,  0.01f, -0.038f };
    HurtboxPreset P2_HURTBOX_FACE_RIGHT{ 0.088f, 0.13f,  -0.0088f, -0.038f };
    HurtboxPreset P2_HURTBOX_CROUCH    { 0.07f, 0.09f,  -0.0f, -0.053f };
}

static Character m_player;
static Character m_player2;
static InputManager* m_inputManager = nullptr;

bool GSPlay::s_showHitboxHurtbox = false;
bool GSPlay::s_showPlatformBoxes = false;
bool GSPlay::s_showWallBoxes = false;
bool GSPlay::s_showLadderBoxes = false;
bool GSPlay::s_showTeleportBoxes = false;

static void ToggleSpecialForm_Internal(Character& character, int specialTexId) {
    if (specialTexId < 0) return;
    character.SetGunMode(false);
    character.SetGrenadeMode(false);

    if (specialTexId == 75) { // Werewolf
        bool toWerewolf = !character.IsWerewolf();
        if (toWerewolf) { character.SetBatDemonMode(false); character.SetKitsuneMode(false); character.SetOrcMode(false); }
        character.SetWerewolfMode(toWerewolf);
    } else if (specialTexId == 76) { // BatDemon
        bool toBat = !character.IsBatDemon();
        if (toBat) { character.SetWerewolfMode(false); character.SetKitsuneMode(false); character.SetOrcMode(false); }
        character.SetBatDemonMode(toBat);
    } else if (specialTexId == 77) { // Kitsune
        bool toKitsune = !character.IsKitsune();
        if (toKitsune) { character.SetBatDemonMode(false); character.SetWerewolfMode(false); character.SetOrcMode(false); }
        character.SetKitsuneMode(toKitsune);
    } else if (specialTexId == 78) { // Orc
        bool toOrc = !character.IsOrc();
        if (toOrc) { character.SetBatDemonMode(false); character.SetWerewolfMode(false); character.SetKitsuneMode(false); }
        character.SetOrcMode(toOrc);
    }

    if (auto mv = character.GetMovement()) mv->SetInputLocked(false);
}

//Random Item Spawns
void GSPlay::InitializeRandomItemSpawns() {
    SceneManager* scene = SceneManager::GetInstance();

    m_candidateItemIds = { 1200,1201,1202,1203,1205,1206,1207,
                           AXE_OBJECT_ID, SWORD_OBJECT_ID, PIPE_OBJECT_ID,
                           1502, 1510,
                           1506,1507,1508,1509 };

    auto cacheTemplate = [&](int id){
        if (Object* o = scene->GetObject(id)) {
            ItemTemplate t{};
            t.modelId = o->GetModelId();
            t.textureIds = o->GetTextureIds();
            t.shaderId = o->GetShaderId();
            t.scale = Vector3();
            t.scale = o->GetScale();
            m_itemTemplates[id] = std::move(t);
        }
    };
    for (int id : m_candidateItemIds) cacheTemplate(id);

    const Vector3 kPositions[11] = {
        Vector3(-3.22f,  1.47f, 0.0f),
        Vector3(-3.60f, -0.76f, 0.0f),
        Vector3(-2.96f, -1.22f, 0.0f),
        Vector3(-1.80f, -1.375f, 0.0f),
        Vector3(-0.90f, -1.375f, 0.0f),
        Vector3(-0.45f, -0.70f, 0.0f),
        Vector3( 0.05f, -0.47f, 0.0f),
        Vector3(-0.15f, -1.375f, 0.0f),
        Vector3( 2.00f, -1.3f, 0.0f),
        Vector3( 2.50f, -0.44f,0.0f),
        Vector3( 2.95f,  0.345f, 0.0f)
    };
    m_spawnSlots.clear();
    m_spawnSlots.resize(11);
    for (int i = 0; i < 11; ++i) {
        m_spawnSlots[i].pos = kPositions[i];
        m_spawnSlots[i].currentId = -1;
        m_spawnSlots[i].lifeTimer = 0.0f;
        m_spawnSlots[i].respawnTimer = 0.0f;
        m_spawnSlots[i].respawnDelay = 1.0f;
        m_spawnSlots[i].active = false;
    }
    m_objectIdToSlot.clear();

    for (int id : m_candidateItemIds) {
        if (scene->GetObject(id)) {
            scene->RemoveObject(id);
        }
    }

    for (int i = 0; i < (int)m_spawnSlots.size(); ++i) {
        int itemId = ChooseRandomAvailableItemId();
        if (!SpawnItemIntoSlot(i, itemId)) {
            for (int id : m_candidateItemIds) {
                if (SpawnItemIntoSlot(i, id)) break;
            }
        }
    }
}

int GSPlay::ChooseRandomAvailableItemId() {
    if (m_candidateItemIds.empty()) return -1;
    struct Candidate { int id; int weight; };
    std::vector<Candidate> pool;
    pool.reserve(m_candidateItemIds.size());
    auto isActiveId = [&](int id){
        for (const auto& s : m_spawnSlots) {
            if (s.active && s.currentId == id) return true;
        }
        return false;
    };
    for (int id : m_candidateItemIds) {
        if (m_itemTemplates.find(id) == m_itemTemplates.end()) continue;
        if (isActiveId(id)) continue;
        bool isSpecial = (id == 1506 || id == 1507 || id == 1508 || id == 1509);
        int weight = isSpecial ? 1 : 8;
        pool.push_back({id, weight});
    }
    if (pool.empty()) return -1;
    int total = 0;
    for (const auto& c : pool) total += c.weight;
    static std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<int> dist(0, total - 1);
    int r = dist(rng);
    for (const auto& c : pool) {
        if (r < c.weight) return c.id;
        r -= c.weight;
    }
    return pool.back().id;
}

bool GSPlay::SpawnItemIntoSlot(int slotIndex, int itemId) {
    if (slotIndex < 0 || slotIndex >= (int)m_spawnSlots.size()) return false;
    SceneManager* scene = SceneManager::GetInstance();
    auto it = m_itemTemplates.find(itemId);
    if (it == m_itemTemplates.end()) return false;
    int objectId = itemId * 100 + slotIndex;
    if (Object* existing = scene->GetObject(objectId)) { scene->RemoveObject(objectId); }
    Object* obj = scene->CreateObject(objectId);
    if (!obj) return false;

    const ItemTemplate& t = it->second;
    obj->SetModel(t.modelId);
    for (int i = 0; i < (int)t.textureIds.size(); ++i) obj->SetTexture(t.textureIds[i], i);
    obj->SetShader(1);
    obj->SetScale(t.scale);
    obj->SetPosition(m_spawnSlots[slotIndex].pos);
    obj->SetVisible(true);

    m_spawnSlots[slotIndex].currentId = objectId;
    m_spawnSlots[slotIndex].typeId = itemId;
    m_spawnSlots[slotIndex].lifeTimer = 0.0f;
    m_spawnSlots[slotIndex].respawnTimer = 0.0f;
    m_spawnSlots[slotIndex].respawnDelay = 1.0f;
    m_spawnSlots[slotIndex].active = true;
    m_objectIdToSlot[objectId] = slotIndex;
    return true;
}

void GSPlay::MarkSlotPickedByObjectId(int itemId) {
    auto it = m_objectIdToSlot.find(itemId);
    if (it == m_objectIdToSlot.end()) return;
    int slotIndex = it->second;
    if (slotIndex >= 0 && slotIndex < (int)m_spawnSlots.size()) {
        m_spawnSlots[slotIndex].active = false;
        m_spawnSlots[slotIndex].currentId = -1;
        m_spawnSlots[slotIndex].typeId = -1;
        m_spawnSlots[slotIndex].lifeTimer = 0.0f;
        m_spawnSlots[slotIndex].respawnTimer = 0.0f;
        m_spawnSlots[slotIndex].respawnDelay = 10.0f;
    }
}

void GSPlay::UpdateRandomItemSpawns(float deltaTime) {
    SceneManager* scene = SceneManager::GetInstance();
    const float LIFETIME = 20.0f;
    const float BLINK_START = 12.0f;
    const float RESPAWN_DELAY = 1.0f;
    m_itemBlinkTimer += deltaTime;
    bool blinkVisible = fmodf(m_itemBlinkTimer, 0.3f) < 0.15f;

    for (int i = 0; i < (int)m_spawnSlots.size(); ++i) {
        SpawnSlot& slot = m_spawnSlots[i];
        if (slot.active) {
            Object* obj = scene->GetObject(slot.currentId);
            if (!obj) {
                slot.active = false;
                slot.currentId = -1;
                slot.respawnTimer = 0.0f;
            } else {
                slot.lifeTimer += deltaTime;
                if (slot.lifeTimer >= BLINK_START) {
                    obj->SetVisible(blinkVisible);
                } else {
                    obj->SetVisible(true);
                }
                if (slot.lifeTimer >= LIFETIME) {
                    scene->RemoveObject(slot.currentId);
                    slot.active = false;
                    slot.currentId = -1;
                    slot.respawnTimer = 0.0f;
                    slot.respawnDelay = 1.0f;
                }
            }
        } else {
            slot.respawnTimer += deltaTime;
            if (slot.respawnTimer >= slot.respawnDelay) {
                int itemId = ChooseRandomAvailableItemId();
                if (!SpawnItemIntoSlot(i, itemId)) {
                    for (int id : m_candidateItemIds) {
                        if (SpawnItemIntoSlot(i, id)) break;
                    }
                }
            }
        }
    }
}

int GSPlay::GetSpecialType(const Character& ch) const {
    if (ch.IsWerewolf()) return 1;
    if (ch.IsBatDemon()) return 2;
    if (ch.IsKitsune())  return 3;
    if (ch.IsOrc())      return 4;
    return 0;
}

void GSPlay::UpdateSpecialFormTimers() {
    auto ensureExpire = [&](Character& ch, float& expire, int& type){
        int cur = GetSpecialType(ch);
        if (cur != 0) {
            if (type != cur) {
                type = cur;
                expire = m_gameTime + 10.0f;
            }
        } else {
            type = 0; expire = -1.0f;
        }
    };
    ensureExpire(m_player,  m_p1SpecialExpireTime, m_p1SpecialType);
    ensureExpire(m_player2, m_p2SpecialExpireTime, m_p2SpecialType);

    auto updateHudTime = [&](bool isP1){
        int id = isP1 ? 936 : 937;
        float expire = isP1 ? m_p1SpecialExpireTime : m_p2SpecialExpireTime;
        int   type   = isP1 ? m_p1SpecialType      : m_p2SpecialType;
        Vector3 base = isP1 ? m_hudSpecialTime1BaseScale : m_hudSpecialTime2BaseScale;
        Object* bar = SceneManager::GetInstance()->GetObject(id);
        if (!bar) return;
        if (type == 0 || expire < 0.0f) {
            bar->SetScale(0.0f, 0.0f, base.z);
            return;
        }
        float remain = expire - m_gameTime;
        if (remain < 0.0f) remain = 0.0f;
        float t = remain / 10.0f; 
        float smooth = 1.0f - (1.0f - t) * (1.0f - t);
        float width = base.x * smooth;
        bar->SetScale(width, base.y, base.z);
    };
    updateHudTime(true);
    updateHudTime(false);

    auto revertIfExpired = [&](Character& ch, float& expire, int& type){
        if (type != 0 && m_gameTime >= expire) {
            switch (type) {
                case 1: ch.SetWerewolfMode(false); break;
                case 2: ch.SetBatDemonMode(false); break;
                case 3: ch.SetKitsuneMode(false);  break;
                case 4: ch.SetOrcMode(false);      break;
            }
            type = 0; expire = -1.0f;
            if (&ch == &m_player) { m_p1SpecialItemTexId = -1; UpdateHudSpecialIcon(true); }
            else { m_p2SpecialItemTexId = -1; UpdateHudSpecialIcon(false); }
        }
    };
    revertIfExpired(m_player,  m_p1SpecialExpireTime, m_p1SpecialType);
    revertIfExpired(m_player2, m_p2SpecialExpireTime, m_p2SpecialType);
}

bool GSPlay_IsShowPlatformBoxes() {
    return GSPlay::IsShowPlatformBoxes();
}

GSPlay::GSPlay() 
    : GameStateBase(StateType::PLAY), m_gameTime(0.0f), m_player1Health(100.0f), m_player2Health(100.0f), m_cloudSpeed(0.5f), m_p1Respawned(false), m_p2Respawned(false) {
}

GSPlay::~GSPlay() {
}

void GSPlay::Init() {
    std::cout << "=== GAMEPLAY MODE ===" << std::endl;
    std::cout << "Game started!" << std::endl;
    std::cout << "Press C to toggle hitbox/hurtbox display" << std::endl;
    std::cout << "Press V to toggle platform boxes display" << std::endl;
    std::cout << "Press Z to toggle camera auto-zoom" << std::endl;
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    SoundManager::Instance().PlayMusicByID(22, -1);
    
    ResourceManager* resourceManager = ResourceManager::GetInstance();
    std::shared_ptr<Texture2D> healthTexture = resourceManager->GetTexture(12);
    if (healthTexture) {
    } else {
        if (resourceManager->LoadTexture(12, "../Resources/Fighter/UI/Health.tga", "GL_CLAMP_TO_EDGE")) {
        } else {
        }
    }
    
    SceneManager* sceneManager = SceneManager::GetInstance();
    if (!sceneManager->LoadSceneForState(StateType::PLAY)) {
        sceneManager->Clear();
        
        Camera* camera = sceneManager->CreateCamera();
        if (camera) {
            float aspect = (float)Globals::screenWidth / (float)Globals::screenHeight;
            camera->SetOrthographic(-aspect, aspect, -1.0f, 1.0f, 0.1f, 100.0f);
            camera->SetLookAt(Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
            
            camera->EnableAutoZoom(true);
            camera->SetZoomRange(0.4f, 2.4f);
            camera->SetZoomSpeed(3.0f);
            camera->SetCameraPadding(0.8f, 0.60f);
            camera->SetVerticalOffset(0.50f);
            
            sceneManager->SetActiveCamera(0);
        }
    } else {
        Camera* camera = sceneManager->GetActiveCamera();
        if (camera) {
            float aspect = (float)Globals::screenWidth / (float)Globals::screenHeight;
            camera->SetOrthographic(-aspect, aspect, -1.0f, 1.0f, 0.1f, 100.0f);
            
            camera->EnableAutoZoom(true);
            camera->SetZoomRange(0.4f, 2.4f);
            camera->SetZoomSpeed(3.0f);
            camera->SetCameraPadding(0.8f, 0.60f);
            camera->SetVerticalOffset(0.50f);
        }
    }    
    
    Object* healthBar1 = sceneManager->GetObject(2000);
    Object* healthBar2 = sceneManager->GetObject(2001);

    m_gameTime = 0.0f;

    m_inputManager = InputManager::GetInstance();
    m_prevJumpingP1 = false;
    m_prevJumpingP2 = false;

    m_animManager = std::make_shared<AnimationManager>();
    
    const TextureData* textureData = ResourceManager::GetInstance()->GetTextureData(10);
    if (textureData && textureData->spriteWidth > 0 && textureData->spriteHeight > 0) {
        std::vector<AnimationData> animations;
        for (const auto& anim : textureData->animations) {
            animations.push_back({anim.startFrame, anim.numFrames, anim.duration, 0.0f});
        }
        m_animManager->Initialize(textureData->spriteWidth, textureData->spriteHeight, animations);
    } else {
    }
    
    m_player.Initialize(m_animManager, 1000);
    m_player.SetInputConfig(CharacterMovement::PLAYER1_INPUT);
    m_player.ResetHealth();
    // Apply Player 1 hurtbox presets
    m_player.SetHurtboxDefault   (P1_HURTBOX_DEFAULT.w,    P1_HURTBOX_DEFAULT.h,    P1_HURTBOX_DEFAULT.ox,    P1_HURTBOX_DEFAULT.oy);
    m_player.SetHurtboxFacingLeft(P1_HURTBOX_FACE_LEFT.w,  P1_HURTBOX_FACE_LEFT.h,  P1_HURTBOX_FACE_LEFT.ox,  P1_HURTBOX_FACE_LEFT.oy);
    m_player.SetHurtboxFacingRight(P1_HURTBOX_FACE_RIGHT.w, P1_HURTBOX_FACE_RIGHT.h, P1_HURTBOX_FACE_RIGHT.ox, P1_HURTBOX_FACE_RIGHT.oy);
    m_player.SetHurtboxCrouchRoll(P1_HURTBOX_CROUCH.w,     P1_HURTBOX_CROUCH.h,     P1_HURTBOX_CROUCH.ox,     P1_HURTBOX_CROUCH.oy);
    
    auto animManager2 = std::make_shared<AnimationManager>();
    
    const TextureData* textureData2 = ResourceManager::GetInstance()->GetTextureData(11);
    if (textureData2 && textureData2->spriteWidth > 0 && textureData2->spriteHeight > 0) {
        std::vector<AnimationData> animations2;
        for (const auto& anim : textureData2->animations) {
            animations2.push_back({anim.startFrame, anim.numFrames, anim.duration, 0.0f});
        }
        animManager2->Initialize(textureData2->spriteWidth, textureData2->spriteHeight, animations2);
    } else {
    }
    
    m_player2.Initialize(animManager2, 1001);
    m_player2.SetInputConfig(CharacterMovement::PLAYER2_INPUT);
    m_player2.ResetHealth();
    // Apply Player 2 hurtbox presets
    m_player2.SetHurtboxDefault   (P2_HURTBOX_DEFAULT.w,    P2_HURTBOX_DEFAULT.h,    P2_HURTBOX_DEFAULT.ox,    P2_HURTBOX_DEFAULT.oy);
    m_player2.SetHurtboxFacingLeft(P2_HURTBOX_FACE_LEFT.w,  P2_HURTBOX_FACE_LEFT.h,  P2_HURTBOX_FACE_LEFT.ox,  P2_HURTBOX_FACE_LEFT.oy);
    m_player2.SetHurtboxFacingRight(P2_HURTBOX_FACE_RIGHT.w, P2_HURTBOX_FACE_RIGHT.h, P2_HURTBOX_FACE_RIGHT.ox, P2_HURTBOX_FACE_RIGHT.oy);
    m_player2.SetHurtboxCrouchRoll(P2_HURTBOX_CROUCH.w,     P2_HURTBOX_CROUCH.h,     P2_HURTBOX_CROUCH.ox,     P2_HURTBOX_CROUCH.oy);

    // Player 1 werewolf
    m_player.SetWerewolfHurtboxIdle   (0.19f, 0.19f,  0.015f, -0.1f);
    m_player.SetWerewolfHurtboxWalk   (0.19f, 0.23f, 0.015f, -0.08f);
    m_player.SetWerewolfHurtboxRun    (0.24f, 0.19f, 0.02f, -0.1f);
    m_player.SetWerewolfHurtboxJump   (0.19f, 0.19f, 0.015f, -0.1f);
    m_player.SetWerewolfHurtboxCombo  (0.19f, 0.23f, 0.015f, -0.08f);
    m_player.SetWerewolfHurtboxPounce (0.24f, 0.19f, 0.02f, -0.1f);
    // Player 2 werewolf
    m_player2.SetWerewolfHurtboxIdle    (0.19f, 0.19f, 0.015f, -0.1f);
    m_player2.SetWerewolfHurtboxWalk    (0.19f, 0.23f, 0.015f, -0.08f);
    m_player2.SetWerewolfHurtboxRun     (0.24f, 0.19f, 0.02f, -0.1f);
    m_player2.SetWerewolfHurtboxJump    (0.19f, 0.19f, 0.015f, -0.1f);
    m_player2.SetWerewolfHurtboxCombo   (0.19f, 0.23f, 0.015f, -0.08f);
    m_player2.SetWerewolfHurtboxPounce  (0.24f, 0.19f, 0.02f, -0.1f);
    
    // Player 1 Kitsune
    m_player.SetKitsuneHurtboxIdle      (0.12f, 0.216f, -0.03f, -0.095f);
    m_player.SetKitsuneHurtboxWalk      (0.15f, 0.18f, 0.0f, -0.05f);
    m_player.SetKitsuneHurtboxRun       (0.15f, 0.18f, 0.0f, -0.05f);
    m_player.SetKitsuneHurtboxEnergyOrb (0.15f, 0.18f, 0.0f, -0.05f);
    
    // Player 2 Kitsune
    m_player2.SetKitsuneHurtboxIdle     (0.12f, 0.216f, -0.03f, -0.095f);
    m_player2.SetKitsuneHurtboxWalk     (0.15f, 0.18f, 0.0f, -0.05f);
    m_player2.SetKitsuneHurtboxRun      (0.15f, 0.18f, 0.0f, -0.05f);
    m_player2.SetKitsuneHurtboxEnergyOrb(0.15f, 0.18f, 0.0f, -0.05f);

    // P1 Orc
    m_player.SetOrcHurtboxIdle  (0.145f, 0.245f, 0.012f, -0.085f);
    m_player.SetOrcHurtboxWalk  (0.125f, 0.245f, 0.012f, -0.085f);
    m_player.SetOrcHurtboxMeteor(0.22f, 0.26f, 0.016f, -0.06f);
    m_player.SetOrcHurtboxFlame (0.18f, 0.22f, 0.012f, -0.09f);
    // P2 Orc
    m_player2.SetOrcHurtboxIdle(0.145f, 0.245f, 0.012f, -0.085f);
    m_player2.SetOrcHurtboxWalk(0.125f, 0.245f, 0.012f, -0.085f);
    m_player2.SetOrcHurtboxMeteor(0.22f, 0.26f, 0.016f, -0.06f);
    m_player2.SetOrcHurtboxFlame(0.18f, 0.22f, 0.012f, -0.09f);
    
    m_player.GetMovement()->ClearPlatforms();
    m_player2.GetMovement()->ClearPlatforms();
    m_player.GetMovement()->ClearMovingPlatforms();
    m_player2.GetMovement()->ClearMovingPlatforms();
    {
        std::ifstream sceneFile("../Resources/GSPlay.txt");
        if (sceneFile.is_open()) {
            std::string line;
            bool inPlatformBlock = false;
            float boxX = 0, boxY = 0, boxWidth = 0, boxHeight = 0;
            while (std::getline(sceneFile, line)) {
                if (line.find("# Platform") != std::string::npos) {
                    inPlatformBlock = true;
                } else if (inPlatformBlock) {
                    if (line.find("POS") == 0) {
                        std::istringstream iss(line.substr(4));
                        float z;
                        iss >> boxX >> boxY >> z;
                    } else if (line.find("SCALE") == 0) {
                        std::istringstream iss(line.substr(6));
                        float scaleZ;
                        iss >> boxWidth >> boxHeight >> scaleZ;
                        m_player.GetMovement()->AddPlatform(boxX, boxY, boxWidth, boxHeight);
                        m_player2.GetMovement()->AddPlatform(boxX, boxY, boxWidth, boxHeight);
                        inPlatformBlock = false;
                    }
                }
            }
        }
    }
    
    m_player.GetMovement()->SetCharacterSize(0.16f, 0.24f);
    m_player2.GetMovement()->SetCharacterSize(0.16f, 0.24f);
    
    // Setup lift platform (Object ID 30)
    Object* liftPlatform = sceneManager->GetObject(30);
    if (liftPlatform) {
        liftPlatform->SetLiftPlatform(true, 
            1.375005f, -1.301985f, 0.0f,  // Start position
            1.375005f, 0.217009f, 0.0f,   // End position
            0.2f, 1.0f);          // Speed: 0.2 units/sec, Pause time: 1.0 second
        std::cout << "Lift platform (ID 30) configured successfully" << std::endl;

        m_player.GetMovement()->AddMovingPlatformById(30);
        m_player2.GetMovement()->AddMovingPlatformById(30);
    } else {
        std::cout << "Warning: Lift platform (ID 30) not found in scene" << std::endl;
    }
    
    // Mark weapon availability if present in scene
    m_isAxeAvailable   = (sceneManager->GetObject(AXE_OBJECT_ID)   != nullptr);
    m_isSwordAvailable = (sceneManager->GetObject(SWORD_OBJECT_ID) != nullptr);
    m_isPipeAvailable  = (sceneManager->GetObject(PIPE_OBJECT_ID)  != nullptr);

    InitializeRandomItemSpawns();
    InitializeRespawnSlots();
    
    m_gameStartBlinkActive = true;
    m_gameStartBlinkTimer = 0.0f;

    // Initialize HUD weapons: cache base scales and hide by default
    if (Object* hudWeapon1 = sceneManager->GetObject(918)) {
        m_hudWeapon1BaseScale = hudWeapon1->GetScale();
        hudWeapon1->SetScale(0.0f, 0.0f, m_hudWeapon1BaseScale.z);
    }
    if (Object* hudWeapon2 = sceneManager->GetObject(919)) {
        m_hudWeapon2BaseScale = hudWeapon2->GetScale();
        hudWeapon2->SetScale(0.0f, 0.0f, m_hudWeapon2BaseScale.z);
    }

    if (Object* hudSpec1 = sceneManager->GetObject(934)) {
        m_hudSpecial1BaseScale = hudSpec1->GetScale();
        hudSpec1->SetScale(0.0f, 0.0f, m_hudSpecial1BaseScale.z);
    }
    if (Object* hudSpec2 = sceneManager->GetObject(935)) {
        m_hudSpecial2BaseScale = hudSpec2->GetScale();
        hudSpec2->SetScale(0.0f, 0.0f, m_hudSpecial2BaseScale.z);
    }

    if (Object* hudTime1 = sceneManager->GetObject(936)) {
        m_hudSpecialTime1BaseScale = hudTime1->GetScale();
        hudTime1->SetScale(0.0f, 0.0f, m_hudSpecialTime1BaseScale.z);
    }
    if (Object* hudTime2 = sceneManager->GetObject(937)) {
        m_hudSpecialTime2BaseScale = hudTime2->GetScale();
        hudTime2->SetScale(0.0f, 0.0f, m_hudSpecialTime2BaseScale.z);
    }

    if (Object* hudGun1 = sceneManager->GetObject(920)) {
        m_hudGun1BaseScale = hudGun1->GetScale();
        if (m_player1GunTexId >= 0) {
            hudGun1->SetScale(m_hudGun1BaseScale);
            hudGun1->SetTexture(m_player1GunTexId, 0);
        } else {
            hudGun1->SetScale(0.0f, 0.0f, m_hudGun1BaseScale.z);
        }
    }
    if (Object* hudGun2 = sceneManager->GetObject(921)) {
        m_hudGun2BaseScale = hudGun2->GetScale();
        if (m_player2GunTexId >= 0) {
            hudGun2->SetScale(m_hudGun2BaseScale);
            hudGun2->SetTexture(m_player2GunTexId, 0);
        } else {
            hudGun2->SetScale(0.0f, 0.0f, m_hudGun2BaseScale.z);
        }
    }

    m_wallCollision = std::make_unique<WallCollision>();
    if (m_wallCollision) {
        m_wallCollision->LoadWallsFromScene();
    }

    if (Object* bA = sceneManager->GetObject(m_bloodProtoIdA)) { bA->SetVisible(false); }
    if (Object* bB = sceneManager->GetObject(m_bloodProtoIdB)) { bB->SetVisible(false); }
    if (Object* bC = sceneManager->GetObject(m_bloodProtoIdC)) { bC->SetVisible(false); }

    std::cout << "Gameplay initialized" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "- Z: Toggle camera auto zoom" << std::endl;
    std::cout << "- R: Reset health for both players" << std::endl;
    std::cout << "=== PLAYER 1 MOVEMENT CONTROLS ===" << std::endl;
    std::cout << "- A: Walk left (Animation 1: Walk)" << std::endl;
    std::cout << "- D: Walk right (Animation 1: Walk)" << std::endl;
    std::cout << "- Double-tap A: Run left (Animation 2: Run)" << std::endl;
    std::cout << "- Double-tap D: Run right (Animation 2: Run)" << std::endl;
    std::cout << "- S: Sit down (Animation 3: Sit)" << std::endl;
    std::cout << "- W: Jump (Animation 16: Jump)" << std::endl;

    std::cout << "- A + S: Roll left (Animation 4: Roll)" << std::endl;
    std::cout << "- S + D: Roll right (Animation 4: Roll)" << std::endl;
    std::cout << "- Release keys: Idle (Animation 0: Idle)" << std::endl;
    std::cout << "=== PLAYER 1 COMBO SYSTEM ===" << std::endl;
    std::cout << "- J: Combo (Punch by default; Axe after pickup)" << std::endl;
    std::cout << "  * Combo window: 0.5 seconds" << std::endl;
    std::cout << "- K: Kick (Animation 19: Kick)" << std::endl;
    std::cout << "=== PLAYER 2 MOVEMENT CONTROLS ===" << std::endl;
    std::cout << "- Left Arrow: Walk left (Animation 1: Walk)" << std::endl;
    std::cout << "- Right Arrow: Walk right (Animation 1: Walk)" << std::endl;
    std::cout << "- Double-tap Left Arrow: Run left (Animation 2: Run)" << std::endl;
    std::cout << "- Double-tap Right Arrow: Run right (Animation 2: Run)" << std::endl;
    std::cout << "- Down Arrow: Sit down (Animation 3: Sit)" << std::endl;
    std::cout << "- Up Arrow: Jump (Animation 16: Jump)" << std::endl;

    std::cout << "- Down Arrow + Left Arrow: Roll left (Animation 4: Roll)" << std::endl;
    std::cout << "- Down Arrow + Right Arrow: Roll right (Animation 4: Roll)" << std::endl;
    std::cout << "- Release keys: Idle (Animation 0: Idle)" << std::endl;
    std::cout << "=== PLAYER 2 COMBO SYSTEM ===" << std::endl;
    std::cout << "- 1: Combo (Punch by default; Axe after pickup)" << std::endl;
    std::cout << "  * Combo window: 0.5 seconds" << std::endl;
    std::cout << "- 2: Kick (Animation 19: Kick)" << std::endl;
    std::cout << "=== PLATFORM SYSTEM ===" << std::endl;
    std::cout << "- White box acts as a platform" << std::endl;
    std::cout << "- Jump onto the box to land on it" << std::endl;
    std::cout << "- Move off the platform to fall down" << std::endl;
    std::cout << "=== COMBAT SYSTEM ===" << std::endl;
    std::cout << "- Each hit deals 10 damage" << std::endl;
    std::cout << "- Characters die when health reaches 0" << std::endl;
    std::cout << "- Use R to reset health" << std::endl;

    
    Camera* cam = SceneManager::GetInstance()->GetActiveCamera();
    std::cout << "Camera actual: left=" << cam->GetLeft() << ", right=" << cam->GetRight()
              << ", bottom=" << cam->GetBottom() << ", top=" << cam->GetTop() << std::endl;
    

    UpdateHealthBars();
    if (Object* bombProto = SceneManager::GetInstance()->GetObject(m_bombObjectId)) {
        bombProto->SetVisible(false);
    }
    if (Object* explProto = SceneManager::GetInstance()->GetObject(m_explosionObjectId)) {
        explProto->SetVisible(false);
    }
    UpdateHudWeapons();

    {
        if (TTF_WasInit() == 0) {
            TTF_Init();
        }
        TTF_Font* font = TTF_OpenFont("../Resources/Font/PressStart2P-Regular.ttf", 64);
        if (!font) {
            std::cout << "Failed to load font for HUD digits: " << TTF_GetError() << std::endl;
        } else {
            TTF_SetFontHinting(font, TTF_HINTING_NONE);
            TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
            auto makeTextTexture = [&](const char* text) -> std::shared_ptr<Texture2D> {
                SDL_Color color = {255, 255, 255, 255};
                SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text, color);
                if (!surf) { return nullptr; }
                std::shared_ptr<Texture2D> tex = std::make_shared<Texture2D>();
                if (!tex->LoadFromSDLSurface(surf)) {
                    SDL_FreeSurface(surf);
                    return nullptr;
                }
                SDL_FreeSurface(surf);
                tex->SetSharpFiltering();
                return tex;
            };

            SceneManager* scene = SceneManager::GetInstance();
            auto setTwoDigits = [&](int value, int leftId, int rightId){
                if (value < 0) value = 0;
                if (value > 99) value = 99;
                int left = (value / 10) % 10;
                int right = value % 10;
                char lbuf[2] = {(char)('0' + left), '\0'};
                char rbuf[2] = {(char)('0' + right), '\0'};
                if (Object* L = scene->GetObject(leftId))  { if (auto t = makeTextTexture(lbuf)) L->SetDynamicTexture(t); }
                if (Object* R = scene->GetObject(rightId)) { if (auto t = makeTextTexture(rbuf)) R->SetDynamicTexture(t); }
            };

            auto setDigitsVisible = [&](bool isP1, bool visible){
                int leftId  = isP1 ? 924 : 926;
                int rightId = isP1 ? 925 : 927;
                if (Object* L = scene->GetObject(leftId))  { if (!visible) L->SetScale(0.0f, 0.0f, 1.0f); }
                if (Object* R = scene->GetObject(rightId)) { if (!visible) R->SetScale(0.0f, 0.0f, 1.0f); }
            };
            setDigitsVisible(true,  m_player1GunTexId >= 0);
            setDigitsVisible(false, m_player2GunTexId >= 0);
            UpdateHudAmmoDigits();

            auto setBombDigitsVisible = [&](bool isP1, bool visible){
                int leftId  = isP1 ? 928 : 930;
                int rightId = isP1 ? 929 : 931;
                if (Object* L = scene->GetObject(leftId))  { if (!visible) L->SetScale(0.0f, 0.0f, 1.0f); }
                if (Object* R = scene->GetObject(rightId)) { if (!visible) R->SetScale(0.0f, 0.0f, 1.0f); }
            };
            setBombDigitsVisible(true,  m_p1Bombs > 0);
            setBombDigitsVisible(false, m_p2Bombs > 0);
            if (Object* bombIcon1 = scene->GetObject(922)) m_hudBombIcon1BaseScale = bombIcon1->GetScale();
            if (Object* bombIcon2 = scene->GetObject(923)) m_hudBombIcon2BaseScale = bombIcon2->GetScale();
            UpdateHudBombDigits();

            TTF_CloseFont(font);
        }
    }
    
    CreateScoreTextObjects();
}

void GSPlay::UpdateHudAmmoDigits() {
    if (TTF_WasInit() == 0) return;
    TTF_Font* font = TTF_OpenFont("../Resources/Font/PressStart2P-Regular.ttf", 64);
    if (!font) { return; }
    TTF_SetFontHinting(font, TTF_HINTING_NONE);
    TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
    auto makeTextTexture = [&](const char* text) -> std::shared_ptr<Texture2D> {
        SDL_Color color = {255, 255, 255, 255};
        SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text, color);
        if (!surf) { return nullptr; }
        std::shared_ptr<Texture2D> tex = std::make_shared<Texture2D>();
        if (!tex->LoadFromSDLSurface(surf)) { SDL_FreeSurface(surf); return nullptr; }
        SDL_FreeSurface(surf);
        tex->SetSharpFiltering();
        return tex;
    };
    SceneManager* scene = SceneManager::GetInstance();
    auto setTwoDigits = [&](int value, int leftId, int rightId){
        if (value < 0) value = 0;
        if (value > 99) value = 99;
        int left = (value / 10) % 10;
        int right = value % 10;
        char lbuf[2] = {(char)('0' + left), '\0'};
        char rbuf[2] = {(char)('0' + right), '\0'};
        if (Object* L = scene->GetObject(leftId))  { if (auto t = makeTextTexture(lbuf)) L->SetDynamicTexture(t); }
        if (Object* R = scene->GetObject(rightId)) { if (auto t = makeTextTexture(rbuf)) R->SetDynamicTexture(t); }
    };
    auto currentAmmo = [&](bool isP1)->int{
        int tex = isP1 ? m_player1GunTexId : m_player2GunTexId;
        switch (tex) {
            case 40: return isP1 ? m_p1Ammo40 : m_p2Ammo40;
            case 41: return isP1 ? m_p1Ammo41 : m_p2Ammo41;
            case 42: return isP1 ? m_p1Ammo42 : m_p2Ammo42;
            case 43: return isP1 ? m_p1Ammo43 : m_p2Ammo43;
            
            case 45: return isP1 ? m_p1Ammo45 : m_p2Ammo45;
            case 46: return isP1 ? m_p1Ammo46 : m_p2Ammo46;
            case 47: return isP1 ? m_p1Ammo47 : m_p2Ammo47;
            default: return 0;
        }
    };
    auto showDigits = [&](bool isP1, bool show){
        int leftId  = isP1 ? 924 : 926;
        int rightId = isP1 ? 925 : 927;
        Vector3 base = isP1 ? m_hudAmmo1BaseScale : m_hudAmmo2BaseScale;
        if (Object* L = scene->GetObject(leftId))  { L->SetScale(show ? base : Vector3(0.0f,0.0f,base.z)); }
        if (Object* R = scene->GetObject(rightId)) { R->SetScale(show ? base : Vector3(0.0f,0.0f,base.z)); }
    };
    int a1 = currentAmmo(true);
    int a2 = currentAmmo(false);
    m_p1HudAmmoTarget = (m_player1GunTexId >= 0) ? a1 : 0;
    m_p2HudAmmoTarget = (m_player2GunTexId >= 0) ? a2 : 0;
    if (m_p1HudAmmoShown <= 0) m_p1HudAmmoShown = m_p1HudAmmoTarget;
    if (m_p2HudAmmoShown <= 0) m_p2HudAmmoShown = m_p2HudAmmoTarget;
    showDigits(true,  a1 > 0 && m_player1GunTexId >= 0);
    showDigits(false, a2 > 0 && m_player2GunTexId >= 0);
    if (m_p1HudAmmoShown > 0 && m_player1GunTexId >= 0) setTwoDigits(m_p1HudAmmoShown, 924, 925);
    if (m_p2HudAmmoShown > 0 && m_player2GunTexId >= 0) setTwoDigits(m_p2HudAmmoShown, 926, 927);
    TTF_CloseFont(font);
}

// Return reference to ammo counter for given gun texture and player
int& GSPlay::AmmoRefFor(int texId, bool isPlayer1) {
    switch (texId) {
        case 40: return isPlayer1 ? m_p1Ammo40 : m_p2Ammo40;
        case 41: return isPlayer1 ? m_p1Ammo41 : m_p2Ammo41;
        case 42: return isPlayer1 ? m_p1Ammo42 : m_p2Ammo42;
        case 43: return isPlayer1 ? m_p1Ammo43 : m_p2Ammo43;
        case 45: return isPlayer1 ? m_p1Ammo45 : m_p2Ammo45;
        case 46: return isPlayer1 ? m_p1Ammo46 : m_p2Ammo46;
        case 47: return isPlayer1 ? m_p1Ammo47 : m_p2Ammo47;
        default: return isPlayer1 ? m_p1Ammo40 : m_p2Ammo40; // fallback
    }
}

// Ammo cost for one trigger release per gun
int GSPlay::AmmoCostFor(int texId) const {
    switch (texId) {
        case 41: // M4A1
        case 42: // Shotgun
        case 47: // Uzi
            return 5;
        case 43: // Bazoka
        case 46: // Sniper
        case 45: // Deagle
        case 40: // Pistol
        default:
            return 1;
    }
}

void GSPlay::TryUnequipIfEmpty(int texId, bool isPlayer1) {
    int& ammo = AmmoRefFor(texId, isPlayer1);
    if (ammo > 0) return;
            if (isPlayer1) m_player1GunTexId = -1; else m_player2GunTexId = -1;
    UpdateHudWeapons();
    UpdateHudAmmoDigits();
}

void GSPlay::StartHudAmmoAnimation(bool isPlayer1) {
    if (isPlayer1) {
        m_p1HudAmmoNextTick = m_gameTime + m_hudAmmoStepInterval;
    } else {
        m_p2HudAmmoNextTick = m_gameTime + m_hudAmmoStepInterval;
    }
}

void GSPlay::RefreshHudAmmoInstant(bool isPlayer1) {
    if (isPlayer1) {
        m_p1HudAmmoShown = m_p1HudAmmoTarget;
    } else {
        m_p2HudAmmoShown = m_p2HudAmmoTarget;
    }
    UpdateHudAmmoDigits();
}

void GSPlay::UpdateHudAmmoAnim(float deltaTime) {
    // Step P1
    if (m_player1GunTexId >= 0 && m_p1HudAmmoShown > m_p1HudAmmoTarget) {
        if (m_gameTime >= m_p1HudAmmoNextTick) {
            m_p1HudAmmoShown -= 1;
            m_p1HudAmmoNextTick = m_gameTime + m_hudAmmoStepInterval;
            UpdateHudAmmoDigits();
        }
    }
    // Step P2
    if (m_player2GunTexId >= 0 && m_p2HudAmmoShown > m_p2HudAmmoTarget) {
        if (m_gameTime >= m_p2HudAmmoNextTick) {
            m_p2HudAmmoShown -= 1;
            m_p2HudAmmoNextTick = m_gameTime + m_hudAmmoStepInterval;
            UpdateHudAmmoDigits();
        }
    }
}

void GSPlay::UpdateHudBombDigits() {
    if (TTF_WasInit() == 0) return;
    TTF_Font* font = TTF_OpenFont("../Resources/Font/PressStart2P-Regular.ttf", 64);
    if (!font) return;
    TTF_SetFontHinting(font, TTF_HINTING_NONE);
    TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
    auto makeTextTexture = [&](const char* text) -> std::shared_ptr<Texture2D> {
        SDL_Color color = {255, 255, 255, 255};
        SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text, color);
        if (!surf) return nullptr;
        std::shared_ptr<Texture2D> tex = std::make_shared<Texture2D>();
        if (!tex->LoadFromSDLSurface(surf)) { SDL_FreeSurface(surf); return nullptr; }
        SDL_FreeSurface(surf);
        tex->SetSharpFiltering();
        return tex;
    };
    SceneManager* scene = SceneManager::GetInstance();
    auto setTwoDigits = [&](int value, int leftId, int rightId){
        if (value < 0) value = 0; if (value > 99) value = 99;
        int left = (value / 10) % 10; int right = value % 10;
        char lbuf[2] = {(char)('0' + left), '\0'};
        char rbuf[2] = {(char)('0' + right), '\0'};
        if (Object* L = scene->GetObject(leftId))  { if (auto t = makeTextTexture(lbuf)) L->SetDynamicTexture(t); }
        if (Object* R = scene->GetObject(rightId)) { if (auto t = makeTextTexture(rbuf)) R->SetDynamicTexture(t); }
    };
    auto showDigits = [&](bool isP1, bool show){
        int leftId  = isP1 ? 928 : 930;
        int rightId = isP1 ? 929 : 931;
        Vector3 base = isP1 ? m_hudBomb1BaseScale : m_hudBomb2BaseScale;
        if (Object* L = scene->GetObject(leftId))  { L->SetScale(show ? base : Vector3(0.0f,0.0f,base.z)); }
        if (Object* R = scene->GetObject(rightId)) { R->SetScale(show ? base : Vector3(0.0f,0.0f,base.z)); }
    };
    showDigits(true,  m_p1Bombs > 0);
    showDigits(false, m_p2Bombs > 0);
    if (Object* bombIcon1 = scene->GetObject(922)) {
        Vector3 base;
        if (m_hudBombIcon1BaseScale.x != 0.0f || m_hudBombIcon1BaseScale.y != 0.0f) {
            base = m_hudBombIcon1BaseScale;
        } else {
            base = bombIcon1->GetScale();
        }
        bombIcon1->SetScale(m_p1Bombs > 0 ? base : Vector3(0.0f, 0.0f, base.z));
    }
    if (Object* bombIcon2 = scene->GetObject(923)) {
        Vector3 base;
        if (m_hudBombIcon2BaseScale.x != 0.0f || m_hudBombIcon2BaseScale.y != 0.0f) {
            base = m_hudBombIcon2BaseScale;
        } else {
            base = bombIcon2->GetScale();
        }
        bombIcon2->SetScale(m_p2Bombs > 0 ? base : Vector3(0.0f, 0.0f, base.z));
    }
    if (m_p1Bombs > 0) setTwoDigits(m_p1Bombs, 928, 929);
    if (m_p2Bombs > 0) setTwoDigits(m_p2Bombs, 930, 931);
    TTF_CloseFont(font);
}

void GSPlay::CreateScoreTextObjects() {
    if (TTF_WasInit() == 0) {
        TTF_Init();
    }
    TTF_Font* font = TTF_OpenFont("../Resources/Font/PressStart2P-Regular.ttf", 32);
    if (!font) {
        std::cout << "Failed to load font for SCORE text: " << TTF_GetError() << std::endl;
        return;
    }
    
    TTF_SetFontHinting(font, TTF_HINTING_NONE);
    TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
    
    SDL_Color color = {255, 255, 255, 255};
    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, "SCORE", color);
    if (!surf) {
        std::cout << "Failed to render SCORE text: " << TTF_GetError() << std::endl;
        TTF_CloseFont(font);
        return;
    }
    
    m_scoreTextTexture = std::make_shared<Texture2D>();
    if (!m_scoreTextTexture->LoadFromSDLSurface(surf)) {
        std::cout << "Failed to create texture from SCORE text surface" << std::endl;
        SDL_FreeSurface(surf);
        TTF_CloseFont(font);
        return;
    }
    SDL_FreeSurface(surf);
    m_scoreTextTexture->SetSharpFiltering();
    
    m_scoreTextP1 = std::make_shared<Object>();
    m_scoreTextP1->SetId(940);
    m_scoreTextP1->SetModel(0);
    m_scoreTextP1->SetShader(0);
    m_scoreTextP1->SetDynamicTexture(m_scoreTextTexture);
    m_scoreTextP1->SetPosition(Vector3(-0.838f, 0.925f, 0.0f));
    m_scoreTextP1->SetRotation(Vector3(0.0f, 0.0f, 0.0f));
    m_scoreTextP1->SetScale(Vector3(0.235f, 0.1f, 1.0f));
    
    m_scoreTextP2 = std::make_shared<Object>();
    m_scoreTextP2->SetId(941);
    m_scoreTextP2->SetModel(0);
    m_scoreTextP2->SetShader(0);
    m_scoreTextP2->SetDynamicTexture(m_scoreTextTexture);
    m_scoreTextP2->SetPosition(Vector3(0.838f, 0.925f, 0.0f));
    m_scoreTextP2->SetRotation(Vector3(0.0f, 0.0f, 0.0f));
    m_scoreTextP2->SetScale(Vector3(0.235f, 0.1f, 1.0f));
    
    TTF_CloseFont(font);
}

int GSPlay::AmmoCapacityFor(int texId) const {
    switch (texId) {
        case 40: return 15; // Pistol
        case 41: return 30; // M4A1
        case 42: return 60; // Shotgun
        case 43: return 3;  // Bazoka
        case 45: return 12; // Deagle
        case 46: return 6;  // Sniper
        case 47: return 30; // Uzi
        default: return 0;
    }
}

void GSPlay::Update(float deltaTime) {
    m_gameTime += deltaTime;
    
    SceneManager::GetInstance()->Update(deltaTime);
    UpdateSpecialFormTimers();
    UpdateRandomItemSpawns(deltaTime);
    UpdateCharacterRespawn(deltaTime);
    UpdateRespawnInvincibility(deltaTime);
    UpdateGameStartBlink(deltaTime);
    
    if (m_inputManager) {
        HandleItemPickup();
        m_player.ProcessInput(deltaTime, m_inputManager);
        m_player2.ProcessInput(deltaTime, m_inputManager);
        m_inputManager->Update();
    } else {
        m_inputManager = InputManager::GetInstance();
    }
    
    m_player.Update(deltaTime);
    m_player2.Update(deltaTime);

    auto checkFireDamage = [&](Character& source, Character& target){
        CharacterAnimation* anim = source.GetAnimation();
        if (!anim) return;
        if (!anim->IsOrcFireActive()) return;
        float l, r, b, t; anim->GetOrcFireAabb(l, r, b, t);
        {
            const float DAMAGE_Y_SCALE = 0.7f;
            float centerY = 0.5f * (b + t);
            float halfH = 0.5f * (t - b) * DAMAGE_Y_SCALE;
            b = centerY - halfH;
            t = centerY + halfH;
        }
        Vector3 pos = target.GetPosition();
        float hx = pos.x + target.GetHurtboxOffsetX();
        float hy = pos.y + target.GetHurtboxOffsetY();
        float halfW = target.GetHurtboxWidth() * 0.5f;
        float halfH = target.GetHurtboxHeight() * 0.5f;
        float tl = hx - halfW, tr = hx + halfW, tb = hy - halfH, tt = hy + halfH;
        bool overlapX = (r >= tl) && (l <= tr);
        bool overlapY = (t >= tb) && (b <= tt);
        if (overlapX && overlapY) {
            if (IsCharacterInvincible(target)) {
                return;
            }
            
            float prev = target.GetHealth();
            target.TakeDamage(100.0f);
            target.CancelAllCombos();
            if (CharacterMovement* mv = target.GetMovement()) { mv->SetInputLocked(false); }
            if (prev > 0.0f && target.GetHealth() <= 0.0f) {
                target.TriggerDie();
            }
        }
    };
    checkFireDamage(m_player, m_player2);
    checkFireDamage(m_player2, m_player);

    auto checkBatWindDamage = [&](Character& source, Character& target){
        CharacterAnimation* anim = source.GetAnimation();
        if (!anim) return;
        if (!anim->IsBatWindActive()) return;
        if (anim->HasBatWindDealtDamage()) return;
        float l, r, b, t; anim->GetBatWindAabb(l, r, b, t);
        Vector3 pos = target.GetPosition();
        float hx = pos.x + target.GetHurtboxOffsetX();
        float hy = pos.y + target.GetHurtboxOffsetY();
        float halfW = target.GetHurtboxWidth() * 0.5f;
        float halfH = target.GetHurtboxHeight() * 0.5f;
        float tl = hx - halfW, tr = hx + halfW, tb = hy - halfH, tt = hy + halfH;
        bool overlapX = (r >= tl) && (l <= tr);
        bool overlapY = (t >= tb) && (b <= tt);
        if (overlapX && overlapY) {
            float prev = target.GetHealth();
            target.TakeDamage(100.0f);
            target.CancelAllCombos();
            if (CharacterMovement* mv = target.GetMovement()) { mv->SetInputLocked(false); }
            if (prev > 0.0f && target.GetHealth() <= 0.0f) {
                target.TriggerDie();
            }
            anim->MarkBatWindDealtDamage();
        }
    };
    checkBatWindDamage(m_player, m_player2);
    checkBatWindDamage(m_player2, m_player);

    auto onJump = [&](Character& ch){
        if (ch.GetMovement() && ch.GetMovement()->ConsumeJustStartedUpwardJump()) {
            SoundManager::Instance().PlaySFXByID(18, 0);
        }
    };
    onJump(m_player);
    onJump(m_player2);
    bool rollingP1 = m_player.GetMovement() ? m_player.GetMovement()->IsRolling() : false;
    bool rollingP2 = m_player2.GetMovement() ? m_player2.GetMovement()->IsRolling() : false;
    if (!m_prevRollingP1 && rollingP1) {
        SoundManager::Instance().PlaySFXByID(3, 0);
    }
    if (!m_prevRollingP2 && rollingP2) {
        SoundManager::Instance().PlaySFXByID(3, 0);
    }
    m_prevRollingP1 = rollingP1;
    m_prevRollingP2 = rollingP2;
    UpdateBullets(deltaTime);
    UpdateEnergyOrbProjectiles(deltaTime);
    UpdateLightningEffects(deltaTime);
    UpdateFireRains(deltaTime);
    UpdateFireRainSpawnQueue();
    CheckLightningDamage();
    
    if (m_player.IsKitsuneEnergyOrbAnimationComplete()) {
        SpawnEnergyOrbProjectile(m_player);
    }
    if (m_player2.IsKitsuneEnergyOrbAnimationComplete()) {
        SpawnEnergyOrbProjectile(m_player2);
    }
    
    UpdateBombs(deltaTime);
    UpdateExplosions(deltaTime);
    UpdateGrenadeFuse();
    UpdateGunBursts();
    UpdateGunReloads();
    TryCompletePendingShots();
    UpdateHudWeapons();
    UpdateHudAmmoAnim(deltaTime);
    UpdateHudBombDigits();
    
            if (m_player.CheckHitboxCollision(m_player2)) {
            if (!IsCharacterInvincible(m_player2)) {
                if (m_player.IsWerewolf() && m_player.GetAnimation() && (((m_player.GetAnimation()->GetCurrentAnimation() == 1) && m_player.IsAnimationPlaying()) || m_player.GetAnimation()->IsWerewolfComboHitWindowActive())) {
                    m_player2.TakeDamage(100.0f);
                    m_player2.TriggerGetHit(m_player);
                } else if (m_player.IsWerewolf() && m_player.GetAnimation() && ((m_player.GetAnimation()->GetCurrentAnimation() == 3 && m_player.IsAnimationPlaying()))) {
                    m_player2.TakeDamage(100.0f);
                    m_player2.TriggerGetHit(m_player);
                } else {
                    m_player2.TriggerGetHit(m_player);
                }
            }
        }
    
    if (m_player2.CheckHitboxCollision(m_player)) {
        if (!IsCharacterInvincible(m_player)) {
            if (m_player2.IsWerewolf() && m_player2.GetAnimation() && (((m_player2.GetAnimation()->GetCurrentAnimation() == 1) && m_player2.IsAnimationPlaying()) || m_player2.GetAnimation()->IsWerewolfComboHitWindowActive())) {
                m_player.TakeDamage(100.0f);
                m_player.TriggerGetHit(m_player2);
            } else if (m_player2.IsWerewolf() && m_player2.GetAnimation() && ((m_player2.GetAnimation()->GetCurrentAnimation() == 3 && m_player2.IsAnimationPlaying()))) {
                m_player.TakeDamage(100.0f);
                m_player.TriggerGetHit(m_player2);
            } else {
                m_player.TriggerGetHit(m_player2);
            }
        }
    }
    
    Camera* camera = SceneManager::GetInstance()->GetActiveCamera();
    if (camera && camera->IsAutoZoomEnabled()) {
        Vector3 player1Pos = m_player.GetPosition();
        Vector3 player2Pos = m_player2.GetPosition();
        camera->UpdateCameraForCharacters(player1Pos, player2Pos, deltaTime);
    }
    
    m_player1Health = m_player.GetHealth();
    m_player2Health = m_player2.GetHealth();
    
    UpdateHealthBars();
    UpdateStaminaBars();
    
    Object* menuButton = SceneManager::GetInstance()->GetObject(MENU_BUTTON_ID);
    if (menuButton) {
        menuButton->SetScale(Vector3(0.2f, 0.1f, 1.0f));
    }
    
    // Update cloud movement
    UpdateCloudMovement(deltaTime);
    
    // Update fan rotation
    UpdateFanRotation(deltaTime);

    UpdateBloods(deltaTime);
}

void GSPlay::Draw() {
    SceneManager::GetInstance()->Draw();
    
    Camera* cam = SceneManager::GetInstance()->GetActiveCamera();
    if (cam) {
        m_player.Draw(cam);
        m_player2.Draw(cam);
    }

    // Draw HUD portraits with independent UVs
    DrawHudPortraits();
    
    if (m_scoreTextP1 && m_scoreTextP2) {
        float aspect = (float)Globals::screenWidth / (float)Globals::screenHeight;
        Matrix uiView;
        Matrix uiProj;
        uiView.SetLookAt(Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
        uiProj.SetOrthographic(-aspect, aspect, -1.0f, 1.0f, 0.1f, 100.0f);
        
        m_scoreTextP1->Draw(uiView, uiProj);
        m_scoreTextP2->Draw(uiView, uiProj);
    }
    
    if (cam) { DrawBullets(cam); }
    if (cam) { DrawEnergyOrbProjectiles(cam); }
    if (cam) { DrawLightningEffects(cam); }
    if (cam) { DrawFireRains(cam); }
    if (cam) { DrawBombs(cam); }
    if (cam) { DrawExplosions(cam); }
    if (cam) { DrawBloods(cam); }
    
    static float lastPosX = m_player.GetPosition().x;
    static int lastAnim = m_player.GetCurrentAnimation();
    static float lastPosX2 = m_player2.GetPosition().x;
    static int lastAnim2 = m_player2.GetCurrentAnimation();
    static bool wasMoving = false;
    static bool wasMoving2 = false;
        
    const bool* keyStates = m_inputManager ? m_inputManager->GetKeyStates() : nullptr;
    if (!keyStates) {
        return;
    }
    bool isMoving = keyStates ? (keyStates['A'] || keyStates['D']) : false;
    bool isMoving2 = keyStates ? (keyStates[0x25] || keyStates[0x27]) : false;
        
    if (abs(m_player.GetPosition().x - lastPosX) > 0.01f || 
        lastAnim != m_player.GetCurrentAnimation() ||
        (isMoving && !wasMoving)) {
            
        if (m_player.IsInCombo()) {
            if (m_player.GetComboCount() > 0) {
                std::cout << "Combo: " << m_player.GetComboCount() << "/3 (Timer: " << m_player.GetComboTimer() << "s)";
                if (m_player.IsComboCompleted()) {
                    std::cout << " [COMPLETED]";
                }
                if (isMoving) {
                    std::cout << " [MOVING DURING COMBO]";
                }
                std::cout << std::endl;
            } else if (m_player.GetAxeComboCount() > 0) {
                std::cout << "Axe Combo: " << m_player.GetAxeComboCount() << "/3 (Timer: " << m_player.GetAxeComboTimer() << "s)";
                if (m_player.IsAxeComboCompleted()) {
                    std::cout << " [COMPLETED]";
                }
                if (isMoving) {
                    std::cout << " [MOVING DURING COMBO]";
                }
                std::cout << std::endl;
            }
        } else if (m_player.GetCurrentAnimation() == 19) {
            std::cout << "Action: KICK [Animation 19]" << std::endl;
        }            
        lastPosX = m_player.GetPosition().x;
        lastAnim = m_player.GetCurrentAnimation();
        wasMoving = isMoving;
    }
    
    if (abs(m_player2.GetPosition().x - lastPosX2) > 0.01f || 
        lastAnim2 != m_player2.GetCurrentAnimation() ||
        (isMoving2 && !wasMoving2)) {
            
        if (m_player2.IsInCombo()) {
            if (m_player2.GetComboCount() > 0) {
                std::cout << "Combo: " << m_player2.GetComboCount() << "/3 (Timer: " << m_player2.GetComboTimer() << "s)";
                if (m_player2.IsComboCompleted()) {
                    std::cout << " [COMPLETED]";
                }
                if (isMoving2) {
                    std::cout << " [MOVING DURING COMBO]";
                }
                std::cout << std::endl;
            } else if (m_player2.GetAxeComboCount() > 0) {
                std::cout << "Axe Combo: " << m_player2.GetAxeComboCount() << "/3 (Timer: " << m_player2.GetAxeComboTimer() << "s)";
                if (m_player2.IsAxeComboCompleted()) {
                    std::cout << " [COMPLETED]";
                }
                if (isMoving2) {
                    std::cout << " [MOVING DURING COMBO]";
                }
                std::cout << std::endl;
            }
        } else if (m_player2.GetCurrentAnimation() == 19) {
            std::cout << "Action: KICK [Animation 19]" << std::endl;
        }
            
        lastPosX2 = m_player2.GetPosition().x;
        lastAnim2 = m_player2.GetCurrentAnimation();
        wasMoving2 = isMoving2;
    }
}

int GSPlay::CreateOrAcquireFireRainObject() {
    if (!m_freeFireRainSlots.empty()) {
        int idx = m_freeFireRainSlots.back();
        m_freeFireRainSlots.pop_back();
        if (idx >= 0 && idx < (int)m_fireRainObjects.size() && m_fireRainObjects[idx]) {
            m_fireRainObjects[idx]->SetVisible(true);
        }
        return idx;
    }
    std::unique_ptr<Object> obj = std::make_unique<Object>(61000 + (int)m_fireRainObjects.size());
    obj->SetModel(0);
    obj->SetTexture(66, 0);
    obj->SetShader(0);
    obj->SetScale(0.3f, 0.3f, 1.0f);
    obj->SetVisible(true);
    obj->MakeModelInstanceCopy();
    m_fireRainObjects.push_back(std::move(obj));
    return (int)m_fireRainObjects.size() - 1;
}

void GSPlay::SpawnFireRainAt(float x, float y) {
    GSPlay::FireRain* fr = nullptr;
    for (auto& r : m_fireRains) {
        if (!r.isActive) { fr = &r; break; }
    }
    if (!fr) {
        if ((int)m_fireRains.size() < MAX_FIRERAIN) {
            m_fireRains.push_back(FireRain{});
            fr = &m_fireRains.back();
        }
    }
    if (!fr) return;

    int objIndex = CreateOrAcquireFireRainObject();
    fr->objectIndex = objIndex;
    fr->isActive = true;
    fr->isFading = false;
    fr->lifetime = 0.0f;
    fr->fadeTimer = 0.0f;
    fr->position = Vector3(x, y, 0.0f);
    fr->velocity = Vector3(0.0f, -1.5f, 0.0f);

    if (!fr->anim) fr->anim = std::make_shared<AnimationManager>();
    if (auto texData = ResourceManager::GetInstance()->GetTextureData(66)) {
        std::vector<AnimationData> anims;
        anims.reserve(texData->animations.size());
        for (const auto& a : texData->animations) {
            anims.push_back({a.startFrame, a.numFrames, a.duration, 0.0f});
        }
        fr->anim->Initialize(texData->spriteWidth, texData->spriteHeight, anims);
        fr->anim->Play(0, true);
        if (texData->animations.size() > 1) {
            fr->fadeDuration = texData->animations[1].duration;
        }
    }

    if (objIndex >= 0 && objIndex < (int)m_fireRainObjects.size() && m_fireRainObjects[objIndex]) {
        m_fireRainObjects[objIndex]->SetTexture(66, 0);
        m_fireRainObjects[objIndex]->SetPosition(fr->position);
        if (fr->anim) {
            float u0, v0, u1, v1;
            fr->anim->GetUV(u0, v0, u1, v1);
            m_fireRainObjects[objIndex]->SetCustomUV(u0, v0, u1, v1);
        }
    }
}

void GSPlay::UpdateFireRains(float deltaTime) {
    for (auto& fr : m_fireRains) {
        if (!fr.isActive) continue;

        fr.lifetime += deltaTime;
        if (!fr.isFading) {
            fr.position += fr.velocity * deltaTime;

            bool hitWall = CheckFireRainWallCollision(fr.position, FIRE_RAIN_COLLISION_W * 0.5f, FIRE_RAIN_COLLISION_H * 0.5f);
            if (hitWall) {
                if (Camera* cam = SceneManager::GetInstance()->GetActiveCamera()) {
                    cam->AddShake(0.01f, 0.18f, 18.0f);
                }
                fr.isFading = true;
                fr.fadeTimer = 0.0f;
                if (fr.anim) fr.anim->Play(1, false);
                fr.damagedP1 = false;
                fr.damagedP2 = false;
            }
        } else {
            fr.fadeTimer += deltaTime;
            bool fadeAnimFinished = (fr.anim && !fr.anim->IsPlaying());
            if (fr.fadeTimer >= fr.fadeDuration || fadeAnimFinished) {
                fr.isActive = false;
                fr.isFading = false;
                fr.lifetime = 0.0f;
                fr.fadeTimer = 0.0f;
                if (fr.objectIndex >= 0 && fr.objectIndex < (int)m_fireRainObjects.size() && m_fireRainObjects[fr.objectIndex]) {
                    m_fireRainObjects[fr.objectIndex]->SetVisible(false);
                    m_freeFireRainSlots.push_back(fr.objectIndex);
                }
                fr.objectIndex = -1;
                continue;
            }
        }

        if (fr.anim) fr.anim->Update(deltaTime);
        if (fr.objectIndex >= 0 && fr.objectIndex < (int)m_fireRainObjects.size() && m_fireRainObjects[fr.objectIndex]) {
            m_fireRainObjects[fr.objectIndex]->SetPosition(fr.position);
            if (fr.anim) {
                float u0, v0, u1, v1;
                fr.anim->GetUV(u0, v0, u1, v1);
                m_fireRainObjects[fr.objectIndex]->SetCustomUV(u0, v0, u1, v1);
            }
        }

        if (fr.isFading) {
            float halfW = FIRE_RAIN_DAMAGE_W * 0.5f;
            float halfH = FIRE_RAIN_DAMAGE_H * 0.5f;
            float aLeft = fr.position.x - halfW;
            float aRight = fr.position.x + halfW;
            float aBottom = fr.position.y - halfH;
            float aTop = fr.position.y + halfH;

            auto checkDamage = [&](Character& target, bool& alreadyDamaged){
                if (alreadyDamaged) return;
                Vector3 pos = target.GetPosition();
                float hx = pos.x + target.GetHurtboxOffsetX();
                float hy = pos.y + target.GetHurtboxOffsetY();
                float halfW2 = target.GetHurtboxWidth() * 0.5f;
                float halfH2 = target.GetHurtboxHeight() * 0.5f;
                float bLeft = hx - halfW2;
                float bRight = hx + halfW2;
                float bBottom = hy - halfH2;
                float bTop = hy + halfH2;
                bool overlapX = aRight >= bLeft && aLeft <= bRight;
                bool overlapY = aTop >= bBottom && aBottom <= bTop;
                if (overlapX && overlapY) {
                    if (IsCharacterInvincible(target)) {
                        return;
                    }
                    
                    float prev = target.GetHealth();
                    target.TakeDamage(100.0f);
                    target.CancelAllCombos();
                    if (CharacterMovement* mv = target.GetMovement()) { mv->SetInputLocked(false); }
                    if (prev > 0.0f && target.GetHealth() <= 0.0f) {
                        target.TriggerDie();
                    }
                    alreadyDamaged = true;
                }
            };

            checkDamage(m_player, fr.damagedP1);
            checkDamage(m_player2, fr.damagedP2);
        }
    }
}

bool GSPlay::CheckFireRainWallCollision(const Vector3& pos, float halfW, float halfH) const {
    if (!m_wallCollision) return false;
    const auto& walls = m_wallCollision->GetWalls();
    float aLeft = pos.x - halfW;
    float aRight = pos.x + halfW;
    float aBottom = pos.y - halfH;
    float aTop = pos.y + halfH;
    for (const auto& w : walls) {
        float bLeft = w.GetLeft();
        float bRight = w.GetRight();
        float bBottom = w.GetBottom();
        float bTop = w.GetTop();
        bool overlapX = aRight >= bLeft && aLeft <= bRight;
        bool overlapY = aTop >= bBottom && aBottom <= bTop;
        if (overlapX && overlapY) return true;
    }
    return false;
}

void GSPlay::DrawFireRains(Camera* camera) {
    for (auto& frObj : m_fireRainObjects) {
        if (frObj && frObj->IsVisible()) {
            frObj->Draw(camera->GetViewMatrix(), camera->GetProjectionMatrix());
        }
    }
}

void GSPlay::QueueFireRainWave(float xStart, float xEnd, float step, float y, float duration) {
    if (step <= 0.0f) return;
    std::vector<float> xs;
    for (float x = xStart; x <= xEnd + 1e-6f; x += step) {
        xs.push_back(x);
    }
    for (float x : xs) {
        float r = (float)rand() / (float)RAND_MAX;
        float spawnOffset = r * duration;
        m_fireRainSpawnQueue.push_back(FireRainEvent{ m_gameTime + spawnOffset, x });
    }
}

void GSPlay::UpdateFireRainSpawnQueue() {
    if (m_fireRainSpawnQueue.empty()) return;
    size_t writeIdx = 0;
    for (size_t i = 0; i < m_fireRainSpawnQueue.size(); ++i) {
        const FireRainEvent& ev = m_fireRainSpawnQueue[i];
        if (m_gameTime >= ev.spawnTime) {
            SpawnFireRainAt(ev.x, 1.2f);
        } else {
            if (writeIdx != i) m_fireRainSpawnQueue[writeIdx] = ev;
            ++writeIdx;
        }
    }
    m_fireRainSpawnQueue.resize(writeIdx);
}

int GSPlay::CreateOrAcquireBombObjectFromProto(int protoObjectId) {
    if (!m_freeBombSlots.empty()) {
        int idx = m_freeBombSlots.back();
        m_freeBombSlots.pop_back();
        if (m_bombObjs[idx]) {
            if (Object* proto = SceneManager::GetInstance()->GetObject(protoObjectId)) {
                m_bombObjs[idx]->SetModel(proto->GetModelId());
                const std::vector<int>& texIds = proto->GetTextureIds();
                if (!texIds.empty()) m_bombObjs[idx]->SetTexture(texIds[0], 0);
                m_bombObjs[idx]->SetShader(proto->GetShaderId());
                m_bombObjs[idx]->SetScale(proto->GetScale());
            }
            m_bombObjs[idx]->SetVisible(true);
        }
        return idx;
    }
    std::unique_ptr<Object> obj = std::make_unique<Object>(50000 + (int)m_bombObjs.size());
    if (Object* proto = SceneManager::GetInstance()->GetObject(protoObjectId)) {
        obj->SetModel(proto->GetModelId());
        const std::vector<int>& texIds = proto->GetTextureIds();
        if (!texIds.empty()) obj->SetTexture(texIds[0], 0);
        obj->SetShader(proto->GetShaderId());
        obj->SetScale(proto->GetScale());
    }
    obj->SetVisible(true);
    m_bombObjs.push_back(std::move(obj));
    return (int)m_bombObjs.size() - 1;
}

void GSPlay::SpawnBombFromCharacter(const Character& ch, float overrideLife) {
    Vector3 pivot = ch.GetGunTopWorldPosition();
    Vector3 base  = ch.GetPosition();
    const float aimDeg = ch.GetAimAngleDeg();
    const float faceSign = ch.IsFacingLeft() ? -1.0f : 1.0f;
    const float aimRad = aimDeg * 3.14159265f / 180.0f;
    const float angleWorld = faceSign * aimRad;

    Vector3 baseSpawn0(base.x + faceSign * BULLET_SPAWN_OFFSET_X,
                       base.y + BULLET_SPAWN_OFFSET_Y,
                       0.0f);
    Vector3 vLocal0(baseSpawn0.x - pivot.x, baseSpawn0.y - pivot.y, 0.0f);
    float c = cosf(angleWorld), s = sinf(angleWorld);
    Vector3 vRot(vLocal0.x * c - vLocal0.y * s, vLocal0.x * s + vLocal0.y * c, 0.0f);
    Vector3 spawn(pivot.x + vRot.x, pivot.y + vRot.y, 0.0f);

    const float forwardStep = 0.02f;
    Vector3 vLocalForward(faceSign * forwardStep, 0.0f, 0.0f);
    Vector3 vRotForward(vLocalForward.x * c - vLocalForward.y * s,
                        vLocalForward.x * s + vLocalForward.y * c, 0.0f);
    Vector3 dir(vRotForward.x, vRotForward.y, 0.0f);
    float len = dir.Length();
    if (len > 1e-6f) dir = dir / len; else dir = Vector3(faceSign, 0.0f, 0.0f);

    int slot = CreateOrAcquireBombObjectFromProto(m_bombObjectId);
    Bomb b; b.x = spawn.x; b.y = spawn.y;
    b.vx = dir.x * BOMB_SPEED; b.vy = dir.y * BOMB_SPEED;
    b.life = (overrideLife > 0.0f) ? overrideLife : BOMB_LIFETIME; b.objIndex = slot; b.angleRad = angleWorld; b.faceSign = faceSign;
    m_bombs.push_back(b);
}

void GSPlay::UpdateBombs(float dt) {
    auto removeBomb = [&](decltype(m_bombs.begin())& it){
        if (it->objIndex >= 0 && it->objIndex < (int)m_bombObjs.size() && m_bombObjs[it->objIndex]) {
            m_freeBombSlots.push_back(it->objIndex);
            m_bombObjs[it->objIndex]->SetVisible(false);
        }
        it = m_bombs.erase(it);
    };

    for (auto it = m_bombs.begin(); it != m_bombs.end(); ) {
        if (!it->atRest) {
            it->vy -= BOMB_GRAVITY * dt;
        }
        float dx = it->vx * dt;
        float dy = it->vy * dt;
        Vector3 curPos(it->x, it->y, 0.0f);
        Vector3 newPos(it->x + dx, it->y + dy, 0.0f);

        bool collided = false;
        bool hitVertical = false;
        bool hitHorizontal = false;

        if (m_wallCollision) {
            Vector3 resolved = m_wallCollision->ResolveWallCollision(curPos, newPos,
                BOMB_COLLISION_WIDTH, BOMB_COLLISION_HEIGHT, 0.0f, 0.0f);
            collided = (resolved.x != newPos.x) || (resolved.y != newPos.y);
            if (collided) {
                if (fabsf(resolved.x - newPos.x) > 1e-6f) hitHorizontal = true;
                if (fabsf(resolved.y - newPos.y) > 1e-6f) hitVertical = true;
                newPos = resolved;
            }
        }

        it->x = newPos.x;
        it->y = newPos.y;
        it->life -= dt;

        if (collided) {
            if (hitHorizontal) {
                it->vx = -it->vx * BOMB_BOUNCE_DAMPING;
                it->vy *= BOMB_WALL_FRICTION;
            }
            if (hitVertical) {
                it->vy = -it->vy * BOMB_BOUNCE_DAMPING;
                it->vx *= BOMB_GROUND_FRICTION;
                if (fabsf(it->vy) < BOMB_MIN_BOUNCE_SPEED) {
                    it->vy = 0.0f;
                    it->grounded = true;
                } else {
                    it->grounded = false;
                }
            }
        }

        if (it->grounded) {
            float sign = (it->vx > 0.0f) ? 1.0f : (it->vx < 0.0f ? -1.0f : 0.0f);
            float mag = fabsf(it->vx);
            mag -= BOMB_GROUND_DRAG * dt;
            if (mag <= 0.0f) { mag = 0.0f; }
            it->vx = sign * mag;
            if (mag == 0.0f) {
                it->atRest = true;
            }
        }

        if (it->life <= 0.0f) {
            SpawnExplosionAt(it->x, it->y, BAZOKA_EXPLOSION_RADIUS_MUL);
            if (Camera* cam = SceneManager::GetInstance()->GetActiveCamera()) {
                cam->AddShake(0.04f, 0.4f, 18.0f);
            }
            removeBomb(it);
            continue;
        }
        ++it;
    }
}

int GSPlay::CreateOrAcquireExplosionObjectFromProto(int protoObjectId) {
    if (!m_freeExplosionSlots.empty()) {
        int idx = m_freeExplosionSlots.back();
        m_freeExplosionSlots.pop_back();
        if (m_explosionObjs[idx]) {
            if (Object* proto = SceneManager::GetInstance()->GetObject(protoObjectId)) {
                m_explosionObjs[idx]->SetModel(proto->GetModelId());
                const std::vector<int>& texIds = proto->GetTextureIds();
                if (!texIds.empty()) m_explosionObjs[idx]->SetTexture(texIds[0], 0);
                m_explosionObjs[idx]->SetShader(proto->GetShaderId());
                m_explosionObjs[idx]->SetScale(proto->GetScale());
            }
            m_explosionObjs[idx]->MakeModelInstanceCopy();
            m_explosionObjs[idx]->SetVisible(true);
        }
        return idx;
    }
    std::unique_ptr<Object> obj = std::make_unique<Object>(60000 + (int)m_explosionObjs.size());
    if (Object* proto = SceneManager::GetInstance()->GetObject(protoObjectId)) {
        obj->SetModel(proto->GetModelId());
        const std::vector<int>& texIds = proto->GetTextureIds();
        if (!texIds.empty()) obj->SetTexture(texIds[0], 0);
        obj->SetShader(proto->GetShaderId());
        obj->SetScale(proto->GetScale());
    }
    obj->MakeModelInstanceCopy();
    obj->SetVisible(true);
    m_explosionObjs.push_back(std::move(obj));
    return (int)m_explosionObjs.size() - 1;
}

void GSPlay::SpawnExplosionAt(float x, float y, float radiusMul) {
    int idx = CreateOrAcquireExplosionObjectFromProto(m_explosionObjectId);
    Explosion e{}; e.x = x; e.y = y; e.objIdx = idx; e.cols = 11; e.rows = 1; e.frameIndex = 0; e.frameCount = 11; e.frameTimer = 0.0f; e.frameDuration = EXPLOSION_FRAME_DURATION; e.damageRadiusMul = radiusMul;
    if (idx >= 0 && idx < (int)m_explosionObjs.size() && m_explosionObjs[idx]) {
        m_explosionObjs[idx]->SetPosition(x, y, 0.0f);
        SetSpriteUV(m_explosionObjs[idx].get(), e.cols, e.rows, 0);
    }
    SoundManager::Instance().PlaySFXByID(8, 0); 
    m_explosions.push_back(e);
}

void GSPlay::UpdateExplosions(float dt) {
    for (size_t i = 0; i < m_explosions.size(); ) {
        Explosion& e = m_explosions[i];
        if (e.frameIndex == 0 && e.frameTimer <= 0.0f) {
            float halfW = 0.15f, halfH = 0.15f;
            if (e.objIdx >= 0 && e.objIdx < (int)m_explosionObjs.size() && m_explosionObjs[e.objIdx]) {
                const Vector3& sc = m_explosionObjs[e.objIdx]->GetScale();
                float mul = (e.damageRadiusMul > 0.0f) ? e.damageRadiusMul : EXPLOSION_DAMAGE_RADIUS_MUL;
                halfW = fabsf(sc.x) * 0.5f * mul;
                halfH = fabsf(sc.y) * 0.5f * mul;
            }
            auto applyDamage = [&](Character& target){
                Vector3 pos = target.GetPosition();
                float hx = pos.x + target.GetHurtboxOffsetX();
                float hy = pos.y + target.GetHurtboxOffsetY();
                float thw = target.GetHurtboxWidth() * 0.5f;
                float thh = target.GetHurtboxHeight() * 0.5f;
                float tLeft = hx - thw, tRight = hx + thw, tBottom = hy - thh, tTop = hy + thh;
                float eLeft = e.x - halfW, eRight = e.x + halfW, eBottom = e.y - halfH, eTop = e.y + halfH;
                bool overlap = !(tRight < eLeft || tLeft > eRight || tTop < eBottom || tBottom > eTop);
                if (!overlap) return;
                // radial falloff by distance to center
                float dx = hx - e.x; float dy = hy - e.y;
                float dist = sqrtf(dx*dx + dy*dy);
                float maxR = (halfW > halfH ? halfW : halfH);
                float ratio = 1.0f - (dist / (maxR + 1e-6f));
                if (ratio < 0.0f) ratio = 0.0f; if (ratio > 1.0f) ratio = 1.0f;
                float damage = 100.0f * ratio;
                if (damage <= 0.0f) return;
                
                if (IsCharacterInvincible(target)) {
                    return;
                }
                
                float prev = target.GetHealth();
                target.TakeDamage(damage);
                target.CancelAllCombos();
                if (CharacterMovement* mv = target.GetMovement()) mv->SetInputLocked(false);
                if (prev > 0.0f && target.GetHealth() <= 0.0f) {
                    target.TriggerDie();
                }
            };
            applyDamage(m_player);
            applyDamage(m_player2);
        }

        e.frameTimer += dt;
        while (e.frameTimer >= e.frameDuration) {
            e.frameTimer -= e.frameDuration;
            e.frameIndex += 1;
            if (e.frameIndex >= e.frameCount) break;
            int idx = e.objIdx;
            if (idx >= 0 && idx < (int)m_explosionObjs.size() && m_explosionObjs[idx]) {
                SetSpriteUV(m_explosionObjs[idx].get(), e.cols, e.rows, e.frameIndex);
            }
        }
        if (e.frameIndex >= e.frameCount) {
            int idx = e.objIdx;
            if (idx >= 0 && idx < (int)m_explosionObjs.size() && m_explosionObjs[idx]) {
                m_explosionObjs[idx]->SetVisible(false);
                m_freeExplosionSlots.push_back(idx);
            }
            m_explosions[i] = m_explosions.back();
            m_explosions.pop_back();
        } else {
            ++i;
        }
    }
}

void GSPlay::DrawExplosions(class Camera* cam) {
    for (const Explosion& e : m_explosions) {
        int idx = e.objIdx;
        if (idx >= 0 && idx < (int)m_explosionObjs.size() && m_explosionObjs[idx]) {
            m_explosionObjs[idx]->Draw(cam->GetViewMatrix(), cam->GetProjectionMatrix());
        }
    }
}

void GSPlay::SetSpriteUV(Object* obj, int cols, int rows, int frameIndex) {
    if (!obj || cols <= 0 || rows <= 0 || frameIndex < 0) return;
    int total = cols * rows;
    if (frameIndex >= total) frameIndex = total - 1;
    int col = frameIndex % cols;
    int row = frameIndex / cols;
    float du = 1.0f / cols;
    float dv = 1.0f / rows;
    float u0 = col * du;
    float v0 = row * dv;
    float u1 = u0 + du;
    float v1 = v0 + dv;
    obj->SetCustomUV(u0, v0, u1, v1);
}

void GSPlay::UpdateGrenadeFuse() {
    auto checkFuse = [&](Character& ch, float& pressTime){
        if (pressTime < 0.0f) return;
        float held = m_gameTime - pressTime;
        if (held >= BOMB_LIFETIME) {
            Vector3 pos = ch.GetPosition();
            SpawnExplosionAt(pos.x, pos.y);
            if (Camera* cam = SceneManager::GetInstance()->GetActiveCamera()) {
                cam->AddShake(0.04f, 0.4f, 18.0f);
            }
            pressTime = -1.0f;
            ch.SetGrenadeMode(false);
            if (&ch == &m_player) { m_p1GrenadeExplodedInHand = true; } else { m_p2GrenadeExplodedInHand = true; }
        }
    };
    checkFuse(m_player, m_p1GrenadePressTime);
    checkFuse(m_player2, m_p2GrenadePressTime);
}

void GSPlay::DrawBombs(Camera* cam) {
    for (const Bomb& b : m_bombs) {
        int idx = b.objIndex;
        if (idx >= 0 && idx < (int)m_bombObjs.size() && m_bombObjs[idx]) {
            m_bombObjs[idx]->SetPosition(b.x, b.y, 0.0f);
            float desired = atan2f(b.vy, b.vx);
            m_bombObjs[idx]->SetRotation(0.0f, 0.0f, desired);
            m_bombObjs[idx]->Draw(cam->GetViewMatrix(), cam->GetProjectionMatrix());
        }
    }
}

void GSPlay::UpdateHudWeapons() {
    SceneManager* scene = SceneManager::GetInstance();
    if (Object* hudWeapon1 = scene->GetObject(918)) {
        auto w = m_player.GetWeapon();
        if (w != Character::WeaponType::None) {
            hudWeapon1->SetScale(m_hudWeapon1BaseScale);
            int texId = (w == Character::WeaponType::Axe) ? HUD_TEX_AXE : (w == Character::WeaponType::Sword) ? HUD_TEX_SWORD : HUD_TEX_PIPE;
            hudWeapon1->SetTexture(texId, 0);
        } else {
            hudWeapon1->SetScale(0.0f, 0.0f, m_hudWeapon1BaseScale.z);
        }
    }
    if (Object* hudWeapon2 = scene->GetObject(919)) {
        auto w = m_player2.GetWeapon();
        if (w != Character::WeaponType::None) {
            hudWeapon2->SetScale(m_hudWeapon2BaseScale);
            int texId = (w == Character::WeaponType::Axe) ? HUD_TEX_AXE : (w == Character::WeaponType::Sword) ? HUD_TEX_SWORD : HUD_TEX_PIPE;
            hudWeapon2->SetTexture(texId, 0);
        } else {
            hudWeapon2->SetScale(0.0f, 0.0f, m_hudWeapon2BaseScale.z);
        }
    }

    auto worldGunSizeForTex = [](int texId)->std::pair<float,float> {
        switch (texId) {
            case 40: return {0.07f,   0.05f};   // Pistol (7x5)
            case 41: return {0.132f,  0.042f}; // M4A1 (22x7)
            case 42: return {0.114f, 0.03f}; // Shotgun (19x5)
            case 43: return {0.138f, 0.054f}; // Bazoka (23x9)
            case 45: return {0.09f,   0.05f};   // Deagle (9x5)
            case 46: return {0.13125f, 0.042f};   // Sniper (25x8)
            case 47: return {0.0675f, 0.06f};   // Uzi (9x8)
            default: return {0.07f,   0.05f};   // fallback pistol
        }
    };
    auto computeHudGunScale = [&](int texId, const Vector3& baseScale)->Vector3 {
        const float pistolWorldH = 0.05f;
        float baseAbsH = fabsf(baseScale.y);
        if (baseAbsH < 1e-6f) return Vector3(baseScale.x, baseScale.y, baseScale.z);
        float k = baseAbsH / pistolWorldH;
        auto wh = worldGunSizeForTex(texId);
        float sx = (wh.first  * k) * (baseScale.x < 0.0f ? -1.0f : 1.0f);
        float sy = (wh.second * k) * (baseScale.y < 0.0f ? -1.0f : 1.0f);
        return Vector3(sx, sy, baseScale.z);
    };

    if (Object* hudGun1 = scene->GetObject(920)) {
        if (m_player1GunTexId < 0) {
            hudGun1->SetScale(0.0f, 0.0f, m_hudGun1BaseScale.z);
        } else {
            hudGun1->SetTexture(m_player1GunTexId, 0);
            hudGun1->SetScale(computeHudGunScale(m_player1GunTexId, m_hudGun1BaseScale));
        }
    }
    if (Object* hudGun2 = scene->GetObject(921)) {
        if (m_player2GunTexId < 0) {
            hudGun2->SetScale(0.0f, 0.0f, m_hudGun2BaseScale.z);
        } else {
            hudGun2->SetTexture(m_player2GunTexId, 0);
            hudGun2->SetScale(computeHudGunScale(m_player2GunTexId, m_hudGun2BaseScale));
        }
    }
}

void GSPlay::DrawHudPortraits() {
    SceneManager* scene = SceneManager::GetInstance();
    Camera* activeCamera = scene->GetActiveCamera();
    if (!activeCamera) return;

    // Prepare UI matrices (screen-space)
    float aspect = (float)Globals::screenWidth / (float)Globals::screenHeight;
    Matrix uiView;
    Matrix uiProj;
    uiView.SetLookAt(Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
    uiProj.SetOrthographic(-aspect, aspect, -1.0f, 1.0f, 0.1f, 100.0f);
    UpdateHudSpecialIcon(true);
    UpdateHudSpecialIcon(false);

    // HUD Player 1 (ID 916)
    if (Object* hud1 = scene->GetObject(916)) {
        hud1->SetTexture(m_player.GetBodyTextureId(), 0);
        float u0, v0, u1, v1;
        m_player.GetCurrentFrameUV(u0, v0, u1, v1);
        // Flip horizontally if player is facing left so HUD mirrors in the same direction
        bool flip = m_player.IsFacingLeft();
        if (flip) {
            std::swap(u0, u1);
        }
        float baseScaleX1 = hud1->GetScale().x;
        float baseScaleY1 = hud1->GetScale().y;
        float baseScaleZ1 = hud1->GetScale().z;
        Vector3 scaled1 = ComputeHudPortraitScale(m_player, Vector3(baseScaleX1, baseScaleY1, baseScaleZ1));
        hud1->SetScale(scaled1.x, scaled1.y, scaled1.z);
        float oldPosX1b = hud1->GetPosition().x;
        float oldPosY1b = hud1->GetPosition().y;
        float oldPosZ1b = hud1->GetPosition().z;
        Vector3 offset1 = ComputeHudPortraitOffset(m_player);
        hud1->SetPosition(oldPosX1b + offset1.x, oldPosY1b + offset1.y, oldPosZ1b + offset1.z);
        hud1->SetCustomUV(u0, v0, u1, v1);
        hud1->Draw(uiView, uiProj);
        hud1->SetScale(baseScaleX1, baseScaleY1, baseScaleZ1);
        hud1->SetPosition(oldPosX1b, oldPosY1b, oldPosZ1b);

        if (m_player.IsGunMode() || m_player.IsGrenadeMode()) {
            float hu0, hv0, hu1, hv1;
            m_player.GetTopFrameUV(hu0, hv0, hu1, hv1);
            if (flip) {
                std::swap(hu0, hu1);
            }
            float offX = m_player.GetHeadOffsetX();
            float offY = m_player.GetHeadOffsetY();
            float sign = flip ? -1.0f : 1.0f;
            float hudOffsetX = sign * offX;
            float hudOffsetY = offY;

            const Vector3& _pos1 = hud1->GetPosition();
            const Vector3& _rot1 = hud1->GetRotation();
            float oldPosX1 = _pos1.x, oldPosY1 = _pos1.y, oldPosZ1 = _pos1.z;
            float oldRotX1 = _rot1.x, oldRotY1 = _rot1.y, oldRotZ1 = _rot1.z;

            float faceSign = flip ? -1.0f : 1.0f;
            float aimDeg = m_player.GetAimAngleDeg();
            float rotZ = faceSign * aimDeg * 3.14159265f / 180.0f;

            hud1->SetPosition(oldPosX1 + hudOffsetX, oldPosY1 + hudOffsetY, oldPosZ1);
            hud1->SetRotation(oldRotX1, oldRotY1, rotZ);

            int headTex = m_player.GetHeadTextureId();
            hud1->SetTexture(headTex, 0);
            hud1->SetCustomUV(hu0, hv0, hu1, hv1);
            hud1->Draw(uiView, uiProj);
            // Restore body texture
            hud1->SetTexture(m_player.GetBodyTextureId(), 0);
            hud1->SetCustomUV(u0, v0, u1, v1);
            hud1->SetPosition(oldPosX1, oldPosY1, oldPosZ1);
            hud1->SetRotation(oldRotX1, oldRotY1, oldRotZ1);
        }
    }

    // HUD Player 2 (ID 917)
    if (Object* hud2 = scene->GetObject(917)) {
        hud2->SetTexture(m_player2.GetBodyTextureId(), 0);
        float u0, v0, u1, v1;
        m_player2.GetCurrentFrameUV(u0, v0, u1, v1);
        bool flip2 = m_player2.IsFacingLeft();
        if (flip2) {
            std::swap(u0, u1);
        }
        float baseScaleX2 = hud2->GetScale().x;
        float baseScaleY2 = hud2->GetScale().y;
        float baseScaleZ2 = hud2->GetScale().z;
        Vector3 scaled2 = ComputeHudPortraitScale(m_player2, Vector3(baseScaleX2, baseScaleY2, baseScaleZ2));
        hud2->SetScale(scaled2.x, scaled2.y, scaled2.z);
        float oldPosX2b = hud2->GetPosition().x;
        float oldPosY2b = hud2->GetPosition().y;
        float oldPosZ2b = hud2->GetPosition().z;
        Vector3 offset2 = ComputeHudPortraitOffset(m_player2);
        hud2->SetPosition(oldPosX2b + offset2.x, oldPosY2b + offset2.y, oldPosZ2b + offset2.z);
        hud2->SetCustomUV(u0, v0, u1, v1);
        hud2->Draw(uiView, uiProj);
        hud2->SetScale(baseScaleX2, baseScaleY2, baseScaleZ2);
        hud2->SetPosition(oldPosX2b, oldPosY2b, oldPosZ2b);

        if (m_player2.IsGunMode() || m_player2.IsGrenadeMode()) {
            float hu0, hv0, hu1, hv1;
            m_player2.GetTopFrameUV(hu0, hv0, hu1, hv1);
            if (flip2) {
                std::swap(hu0, hu1);
            }
            float offX = m_player2.GetHeadOffsetX();
            float offY = m_player2.GetHeadOffsetY();
            float sign2 = flip2 ? -1.0f : 1.0f;
            float hudOffsetX = sign2 * offX;
            float hudOffsetY = offY;

            const Vector3& _pos2 = hud2->GetPosition();
            const Vector3& _rot2 = hud2->GetRotation();
            float oldPosX2 = _pos2.x, oldPosY2 = _pos2.y, oldPosZ2 = _pos2.z;
            float oldRotX2 = _rot2.x, oldRotY2 = _rot2.y, oldRotZ2 = _rot2.z;

            float faceSign2 = flip2 ? -1.0f : 1.0f;
            float aimDeg2 = m_player2.GetAimAngleDeg();
            float rotZ2 = faceSign2 * aimDeg2 * 3.14159265f / 180.0f;

            hud2->SetPosition(oldPosX2 + hudOffsetX, oldPosY2 + hudOffsetY, oldPosZ2);
            hud2->SetRotation(oldRotX2, oldRotY2, rotZ2);

            int headTex = m_player2.GetHeadTextureId();
            hud2->SetTexture(headTex, 0);
            hud2->SetCustomUV(hu0, hv0, hu1, hv1);
            hud2->Draw(uiView, uiProj);
            hud2->SetTexture(m_player2.GetBodyTextureId(), 0);
            hud2->SetCustomUV(u0, v0, u1, v1);
            hud2->SetPosition(oldPosX2, oldPosY2, oldPosZ2);
            hud2->SetRotation(oldRotX2, oldRotY2, oldRotZ2);
        }
    }
}

void GSPlay::UpdateHudSpecialIcon(bool isPlayer1) {
    SceneManager* scene = SceneManager::GetInstance();
    int objId = isPlayer1 ? 934 : 935;
    int texId = isPlayer1 ? m_p1SpecialItemTexId : m_p2SpecialItemTexId;
    if (Object* hudSp = scene->GetObject(objId)) {
        if (texId < 0) {
            Vector3 base = isPlayer1 ? m_hudSpecial1BaseScale : m_hudSpecial2BaseScale;
            hudSp->SetScale(0.0f, 0.0f, base.z);
            return;
        }
        hudSp->SetTexture(texId, 0);
        Vector3 base = isPlayer1 ? m_hudSpecial1BaseScale : m_hudSpecial2BaseScale;
        hudSp->SetScale(base.x, base.y, base.z);
    }
}

Vector3 GSPlay::ComputeHudPortraitScale(const Character& ch, const Vector3& baseScale) const {
    float mul = 1.0f;
    if (ch.IsWerewolf()) {
        mul = m_portraitScaleMulWerewolf;
    } else if (ch.IsBatDemon()) {
        mul = m_portraitScaleMulBatDemon;
    } else if (ch.IsKitsune()) {
        mul = m_portraitScaleMulKitsune;
    } else if (ch.IsOrc()) {
        mul = m_portraitScaleMulOrc;
    }
    return Vector3(baseScale.x * mul, baseScale.y * mul, baseScale.z);
}

Vector3 GSPlay::ComputeHudPortraitOffset(const Character& ch) const {
    if (ch.IsWerewolf()) {
        return Vector3(m_portraitOffsetWerewolf.x, m_portraitOffsetWerewolf.y, m_portraitOffsetWerewolf.z);
    } else if (ch.IsBatDemon()) {
        return Vector3(m_portraitOffsetBatDemon.x, m_portraitOffsetBatDemon.y, m_portraitOffsetBatDemon.z);
    } else if (ch.IsKitsune()) {
        return Vector3(m_portraitOffsetKitsune.x, m_portraitOffsetKitsune.y, m_portraitOffsetKitsune.z);
    } else if (ch.IsOrc()) {
        return Vector3(m_portraitOffsetOrc.x, m_portraitOffsetOrc.y, m_portraitOffsetOrc.z);
    }
    return Vector3(0.0f, 0.0f, 0.0f);
}

void GSPlay::HandleKeyEvent(unsigned char key, bool bIsPressed) {
    if (m_inputManager) {
        m_inputManager->UpdateKeyState(key, bIsPressed);
    }
    
    if (key == 'M' || key == 'm') {
        if (bIsPressed) {
            DetonatePlayerProjectiles(2);
        }
        
        if (m_player2.IsOrc()) {
            if (bIsPressed) { m_player2.TriggerOrcFlameBurst(); }
            return;
        }
        if (m_player2.IsKitsune()) {
            return;
        }
        if (m_player2.IsBatDemon()) {
            if (bIsPressed) return;
        }
        if (m_player2.IsWerewolf()) {
            if (bIsPressed) { m_player2.TriggerWerewolfPounce(); }
            return;
        }
        bool was = m_player2.IsGunMode();
        if (bIsPressed) {
            bool hasGun = (m_player2GunTexId >= 0);
            bool ammoOk = true;
            if (hasGun) {
                int need = AmmoCostFor(m_player2GunTexId);
                int have = AmmoRefFor(m_player2GunTexId, false);
                ammoOk = (have >= need);
            }
            if (!m_player2.IsGrenadeMode() && !m_player2.IsJumping() && hasGun && ammoOk) {
                m_player2.SetGunMode(true);
                m_player2.GetMovement()->SetInputLocked(true);
                if (!was) {
                    m_p2ShotPending = false; m_p2GunStartTime = m_gameTime;
                    if (m_player2GunTexId == 43 || m_player2GunTexId == 44) {
                        SoundManager::Instance().PlaySFXByID(23, 0);
                    } else {
                        SoundManager::Instance().PlaySFXByID(11, 0);
                    }
                }
            }
        } else {
            if (was) { m_p2ShotPending = true; }
        }
    }
    if (key == '2') {
        if (bIsPressed) {
            DetonatePlayerProjectiles(1);
        }
        
        if (m_player.IsOrc()) {
            if (bIsPressed) { m_player.TriggerOrcFlameBurst(); }
            return;
        }
        if (m_player.IsKitsune()) {
            return;
        }
        if (m_player.IsBatDemon()) {
            if (bIsPressed) return;
        }
        if (m_player.IsWerewolf()) {
            if (bIsPressed) { m_player.TriggerWerewolfPounce(); }
            return;
        }
        bool was = m_player.IsGunMode();
        if (bIsPressed) {
            bool hasGun = (m_player1GunTexId >= 0);
            bool ammoOk = true;
            if (hasGun) {
                int need = AmmoCostFor(m_player1GunTexId);
                int have = AmmoRefFor(m_player1GunTexId, true);
                ammoOk = (have >= need);
            }
            if (!m_player.IsGrenadeMode() && !m_player.IsJumping() && hasGun && ammoOk) {
                m_player.SetGunMode(true);
                m_player.GetMovement()->SetInputLocked(true);
                if (!was) {
                    m_p1ShotPending = false; m_p1GunStartTime = m_gameTime;
                    if (m_player1GunTexId == 43 || m_player1GunTexId == 44) {
                        SoundManager::Instance().PlaySFXByID(23, 0);
                    } else {
                        SoundManager::Instance().PlaySFXByID(11, 0);
                    }
                }
            }
        } else {
            if (was) { m_p1ShotPending = true; }
        }
    }

    if (bIsPressed && key == '4') {
        if (!(m_player.IsWerewolf() || m_player.IsBatDemon() || m_player.IsKitsune() || m_player.IsOrc())) {
            if (m_p1SpecialItemTexId >= 0) {
                ToggleSpecialForm_Internal(m_player, m_p1SpecialItemTexId);
                if (m_player.IsWerewolf() || m_player.IsBatDemon() || m_player.IsKitsune() || m_player.IsOrc()) {
                    m_p1SpecialItemTexId = -1; UpdateHudSpecialIcon(true);
                }
            }
        }
    }
    if (bIsPressed && (key == '.' || key == 0xBE)) { 
        if (!(m_player2.IsWerewolf() || m_player2.IsBatDemon() || m_player2.IsKitsune() || m_player2.IsOrc())) {
            if (m_p2SpecialItemTexId >= 0) {
                ToggleSpecialForm_Internal(m_player2, m_p2SpecialItemTexId);
                if (m_player2.IsWerewolf() || m_player2.IsBatDemon() || m_player2.IsKitsune() || m_player2.IsOrc()) {
                    m_p2SpecialItemTexId = -1; UpdateHudSpecialIcon(false);
                }
            }
        }
    }
    
    if (bIsPressed && key == '1') { 
        if (m_player.IsOrc()) {
            m_player.TriggerOrcMeteorStrike();
            QueueFireRainWave(-3.8f, 3.4f, 0.1f, 1.2f, 5.0f);
        } else if (m_player.IsKitsune()) {
            m_player.TriggerKitsuneEnergyOrb();
        } else if (m_player.IsWerewolf()) {
            m_player.TriggerWerewolfCombo();
            if (m_player.CheckHitboxCollision(m_player2)) {
                m_player2.TakeDamage(100.0f);
                m_player2.TriggerGetHit(m_player);
            }
        } else if (m_player.IsGunMode() || m_player.IsGrenadeMode()) {
            m_player.SetGunMode(false);
            m_player.SetGrenadeMode(false);
            m_p1ShotPending = false; m_p1BurstActive = false; m_p1ReloadPending = false;
            m_p1GrenadePressTime = -1.0f; m_p1GrenadeExplodedInHand = false;
            if (m_player.GetMovement()) m_player.GetMovement()->SetInputLocked(false);
            m_player.SuppressNextPunch();
        }
    }
    
    if (bIsPressed && (key == 'N' || key == 'n')) {
        if (m_player2.IsOrc()) {
            m_player2.TriggerOrcMeteorStrike();
            QueueFireRainWave(-3.8f, 3.4f, 0.1f, 1.2f, 5.0f);
        } else if (m_player2.IsKitsune()) {
            m_player2.TriggerKitsuneEnergyOrb();
        } else if (m_player2.IsWerewolf()) {
            m_player2.TriggerWerewolfCombo();
            if (m_player2.CheckHitboxCollision(m_player)) {
                m_player.TakeDamage(100.0f);
                m_player.TriggerGetHit(m_player2);
            }
        } else if (m_player2.IsGunMode() || m_player2.IsGrenadeMode()) {
            m_player2.SetGunMode(false);
            m_player2.SetGrenadeMode(false);
            m_p2ShotPending = false; m_p2BurstActive = false; m_p2ReloadPending = false;
            m_p2GrenadePressTime = -1.0f; m_p2GrenadeExplodedInHand = false;
            if (m_player2.GetMovement()) m_player2.GetMovement()->SetInputLocked(false);
            m_player2.SuppressNextPunch();
        }
    }
    
    if (key == '3') {
        if (m_player.IsKitsune()) {
            return;
        }
        if (m_player.IsBatDemon()) {
            return;
        }
        if (m_player.IsWerewolf()) {
            return;
        }
        if (m_player.IsOrc()) {
            return;
        }
        if (bIsPressed) {
            if (!m_player.IsGunMode() && m_p1Bombs > 0) {
                bool entering = !m_player.IsGrenadeMode();
                if (entering) {
                    m_player.SetGrenadeMode(true);
                    m_p1ShotPending = false; m_p1BurstActive = false; m_p1ReloadPending = false;
                    if (m_p1GrenadePressTime < 0.0f) m_p1GrenadePressTime = m_gameTime;
                    SoundManager::Instance().PlaySFXByID(7, 0);
                }
            }
        } else {
            bool wasGrenade = m_player.IsGrenadeMode();
            m_player.SetGrenadeMode(false);
            if (wasGrenade) {
                float held = (m_p1GrenadePressTime >= 0.0f) ? (m_gameTime - m_p1GrenadePressTime) : 0.0f;
                float remain = BOMB_LIFETIME - held;
                if (remain <= 0.0f || m_p1GrenadeExplodedInHand) {
                } else {
                    if (remain < 0.2f) remain = 0.2f;
                    if (m_p1Bombs > 0 && !m_player.IsOrc()) { SpawnBombFromCharacter(m_player, remain); m_p1Bombs -= 1; UpdateHudBombDigits(); }
                }
                m_p1GrenadePressTime = -1.0f;
                m_p1GrenadeExplodedInHand = false;
            }
        }
    }
    if (key == ',' || key == 0xBC) { 
        if (m_player2.IsOrc()) {
            return;
        }
        if (m_player2.IsKitsune()) {
            return;
        }
        if (m_player2.IsBatDemon()) {
            return;
        }
        if (m_player2.IsWerewolf()) {
            return;
        }
        if (bIsPressed) {
            if (!m_player2.IsGunMode() && m_p2Bombs > 0) {
                bool entering2 = !m_player2.IsGrenadeMode();
                if (entering2) {
                    m_player2.SetGrenadeMode(true);
                    m_p2ShotPending = false; m_p2BurstActive = false; m_p2ReloadPending = false;
                    if (m_p2GrenadePressTime < 0.0f) m_p2GrenadePressTime = m_gameTime;
                    SoundManager::Instance().PlaySFXByID(7, 0);
                }
            }
        } else {
            bool wasGrenade2 = m_player2.IsGrenadeMode();
            m_player2.SetGrenadeMode(false);
            if (wasGrenade2) {
                float held = (m_p2GrenadePressTime >= 0.0f) ? (m_gameTime - m_p2GrenadePressTime) : 0.0f;
                float remain = BOMB_LIFETIME - held;
                if (remain <= 0.0f || m_p2GrenadeExplodedInHand) {
                } else {
                    if (remain < 0.2f) remain = 0.2f;
                    if (m_p2Bombs > 0 && !m_player2.IsOrc()) { SpawnBombFromCharacter(m_player2, remain); m_p2Bombs -= 1; UpdateHudBombDigits(); }
                }
                m_p2GrenadePressTime = -1.0f;
                m_p2GrenadeExplodedInHand = false;
            }
        }
    }

    if (!bIsPressed) return;
    
    switch (key) {
        case 'C':
        case 'c':
            s_showHitboxHurtbox = !s_showHitboxHurtbox;
            break;
            
        case 'Z':
        case 'z':
            {
                Camera* camera = SceneManager::GetInstance()->GetActiveCamera();
                if (camera) {
                    bool currentState = camera->IsAutoZoomEnabled();
                    camera->EnableAutoZoom(!currentState);
                    
                    if (!currentState) {
                    } else {
                        camera->ResetToInitialState();
                    }
                }
            }
            break;
            
        case 'R':
        case 'r':
            m_player.ResetHealth();
            m_player2.ResetHealth();
            m_player1Health = m_player.GetHealth();
            m_player2Health = m_player2.GetHealth();
            UpdateHealthBars();
            break;
            
        case 'V':
        case 'v':
            s_showPlatformBoxes = !s_showPlatformBoxes;
            s_showWallBoxes = !s_showWallBoxes;
            s_showLadderBoxes = !s_showLadderBoxes;
            s_showTeleportBoxes = !s_showTeleportBoxes;
            break;
    }
}

void GSPlay::SpawnBulletFromCharacter(const Character& ch) {
    // Pivot at the top overlay (shoulder/gun root)
    Vector3 pivot = ch.GetGunTopWorldPosition();
    Vector3 base  = ch.GetPosition();
    const float aimDeg = ch.GetAimAngleDeg();
    const float faceSign = ch.IsFacingLeft() ? -1.0f : 1.0f;
    const float aimRad = aimDeg * 3.14159265f / 180.0f;
    const float angleWorld = faceSign * aimRad;

    // Compute the muzzle point at aim=0 using the original constants (relative to base)
    Vector3 baseSpawn0(base.x + faceSign * BULLET_SPAWN_OFFSET_X,
                       base.y + BULLET_SPAWN_OFFSET_Y,
                       0.0f);
    // Local vector from pivot to that muzzle at aim=0
    Vector3 vLocal0(baseSpawn0.x - pivot.x, baseSpawn0.y - pivot.y, 0.0f);

    // Rotate this local vector by the same upper-body rotation to get current spawn
    float cosA = cosf(angleWorld);
    float sinA = sinf(angleWorld);
    Vector3 vRot(vLocal0.x * cosA - vLocal0.y * sinA,
                 vLocal0.x * sinA + vLocal0.y * cosA,
                 0.0f);
    Vector3 spawn(pivot.x + vRot.x, pivot.y + vRot.y, 0.0f);

    // Bullet direction: use the same rotated forward as the top by rotating a small local +X step
    const float forwardStep = 0.02f; // small step along muzzle axis at aim=0
    Vector3 vLocalForward(faceSign * forwardStep, 0.0f, 0.0f);
    Vector3 vRotForward(vLocalForward.x * cosA - vLocalForward.y * sinA,
                        vLocalForward.x * sinA + vLocalForward.y * cosA,
                        0.0f);
    Vector3 dir(vRotForward.x, vRotForward.y, 0.0f);
    float len = dir.Length();
    if (len > 1e-6f) dir = dir / len; else dir = Vector3(faceSign, 0.0f, 0.0f);

    int slot = CreateOrAcquireBulletObject();
    bool isP1 = (&ch == &m_player);
    int gunTex = isP1 ? m_player1GunTexId : m_player2GunTexId;
    float speedMul = 1.0f;
    float damage = 10.0f;
    // Per-weapon tuning
    if (gunTex == 40) { // Pistol
        speedMul = 1.5f; damage = 10.0f;
    } else if (gunTex == 41) { // M4A1
        speedMul = 1.5f; damage = 10.0f;
    } else if (gunTex == 42) { // Shotgun pellet
        speedMul = 1.5f; damage = 10.0f;
    } else if (gunTex == 45) { // Deagle
        speedMul = 1.8f; damage = 20.0f;
    } else if (gunTex == 46) { // Sniper
        speedMul = 2.2f; damage = 50.0f;
    } else if (gunTex == 47) { // Uzi
        speedMul = 1.5f; damage = 10.0f;
    }
    Bullet b;
    b.x = spawn.x; b.y = spawn.y;
    b.vx = dir.x * BULLET_SPEED * speedMul; b.vy = dir.y * BULLET_SPEED * speedMul;
    b.life = BULLET_LIFETIME; b.objIndex = slot;
    b.angleRad = angleWorld; b.faceSign = faceSign;
    b.ownerId = isP1 ? 1 : 2; b.damage = damage;
    m_bullets.push_back(b);
}

void GSPlay::SpawnBulletFromCharacterWithJitter(const Character& ch, float jitterDeg) {
    Vector3 pivot = ch.GetGunTopWorldPosition();
    Vector3 base  = ch.GetPosition();
    const float aimDeg = ch.GetAimAngleDeg();
    const float faceSign = ch.IsFacingLeft() ? -1.0f : 1.0f;
    const float aimRad = (aimDeg + jitterDeg) * 3.14159265f / 180.0f;
    const float angleWorld = faceSign * aimRad;

    Vector3 baseSpawn0(base.x + faceSign * BULLET_SPAWN_OFFSET_X,
                       base.y + BULLET_SPAWN_OFFSET_Y,
                       0.0f);
    Vector3 vLocal0(baseSpawn0.x - pivot.x, baseSpawn0.y - pivot.y, 0.0f);

    float cosA = cosf(angleWorld);
    float sinA = sinf(angleWorld);
    Vector3 vRot(vLocal0.x * cosA - vLocal0.y * sinA,
                 vLocal0.x * sinA + vLocal0.y * cosA,
                 0.0f);
    Vector3 spawn(pivot.x + vRot.x, pivot.y + vRot.y, 0.0f);

    const float forwardStep = 0.02f;
    Vector3 vLocalForward(faceSign * forwardStep, 0.0f, 0.0f);
    Vector3 vRotForward(vLocalForward.x * cosA - vLocalForward.y * sinA,
                        vLocalForward.x * sinA + vLocalForward.y * cosA,
                        0.0f);
    Vector3 dir(vRotForward.x, vRotForward.y, 0.0f);
    float len = dir.Length();
    if (len > 1e-6f) dir = dir / len; else dir = Vector3(faceSign, 0.0f, 0.0f);

    int slot = CreateOrAcquireBulletObject();
    bool isP1 = (&ch == &m_player);
    int gunTex = isP1 ? m_player1GunTexId : m_player2GunTexId;
    float speedMul = 1.0f;
    float damage = 10.0f;
    // Per-weapon tuning
    if (gunTex == 40) { // Pistol
        speedMul = 1.5f; damage = 10.0f;
    } else if (gunTex == 41) { // M4A1 burst
        speedMul = 1.5f; damage = 10.0f;
    } else if (gunTex == 42) { // Shotgun pellet
        speedMul = 1.5f; damage = 10.0f;
    } else if (gunTex == 45) { // Deagle
        speedMul = 1.8f; damage = 20.0f;
    } else if (gunTex == 46) { // Sniper
        speedMul = 2.2f; damage = 50.0f;
    } else if (gunTex == 47) { // Uzi burst
        speedMul = 1.5f; damage = 10.0f;
    }
    Bullet b; b.x = spawn.x; b.y = spawn.y; b.vx = dir.x * BULLET_SPEED * speedMul; b.vy = dir.y * BULLET_SPEED * speedMul; b.life = BULLET_LIFETIME; b.objIndex = slot; b.angleRad = angleWorld; b.faceSign = faceSign; b.ownerId = isP1 ? 1 : 2; b.damage = damage;
    m_bullets.push_back(b);
}

void GSPlay::SpawnBazokaBulletFromCharacter(const Character& ch, float jitterDeg, float speedMul, float damage) {
    Vector3 pivot = ch.GetGunTopWorldPosition();
    Vector3 base  = ch.GetPosition();
    const float aimDeg = ch.GetAimAngleDeg() + jitterDeg;
    const float faceSign = ch.IsFacingLeft() ? -1.0f : 1.0f;
    const float aimRad = aimDeg * 3.14159265f / 180.0f;
    const float angleWorld = faceSign * aimRad;

    Vector3 baseSpawn0(base.x + faceSign * BULLET_SPAWN_OFFSET_X,
                       base.y + BULLET_SPAWN_OFFSET_Y,
                       0.0f);
    Vector3 vLocal0(baseSpawn0.x - pivot.x, baseSpawn0.y - pivot.y, 0.0f);
    float c = cosf(angleWorld), s = sinf(angleWorld);
    Vector3 vRot(vLocal0.x * c - vLocal0.y * s, vLocal0.x * s + vLocal0.y * c, 0.0f);
    Vector3 spawn(pivot.x + vRot.x, pivot.y + vRot.y, 0.0f);

    const float forwardStep = 0.02f;
    Vector3 vLocalForward(faceSign * forwardStep, 0.0f, 0.0f);
    Vector3 vRotForward(vLocalForward.x * c - vLocalForward.y * s,
                        vLocalForward.x * s + vLocalForward.y * c, 0.0f);
    Vector3 dir(vRotForward.x, vRotForward.y, 0.0f);
    float len = dir.Length();
    if (len > 1e-6f) dir = dir / len; else dir = Vector3(faceSign, 0.0f, 0.0f);

    int slot = CreateOrAcquireBulletObjectFromProto(m_bazokaBulletObjectId);
    const bool isP1 = (&ch == &m_player);
    Bullet b; b.x = spawn.x; b.y = spawn.y;
    b.vx = dir.x * BULLET_SPEED * speedMul; b.vy = dir.y * BULLET_SPEED * speedMul;
    b.life = BULLET_LIFETIME; b.objIndex = slot; b.angleRad = angleWorld; b.faceSign = faceSign;
    b.ownerId = isP1 ? 1 : 2; b.damage = damage; b.isBazoka = true; b.trailTimer = 0.0f;
    if ((int)m_bullets.size() < MAX_BULLETS) {
        m_bullets.push_back(b);
    }
}

void GSPlay::UpdateBullets(float dt) {
    auto removeBullet = [&](decltype(m_bullets.begin())& it){
        if (it->objIndex >= 0 && it->objIndex < (int)m_bulletObjs.size() && m_bulletObjs[it->objIndex]) {
            m_freeBulletSlots.push_back(it->objIndex);
            m_bulletObjs[it->objIndex]->SetVisible(false);
        }
        it = m_bullets.erase(it);
    };

    auto aabbOverlap = [](float aLeft, float aRight, float aBottom, float aTop,
                          float bLeft, float bRight, float bBottom, float bTop){
        return (aLeft < bRight && aRight > bLeft && aBottom < bTop && aTop > bBottom);
    };

    // Update all bullets
    for (auto it = m_bullets.begin(); it != m_bullets.end(); ) {
        float dx = it->vx * dt;
        float dy = it->vy * dt;
        it->x += dx;
        it->y += dy;
        it->life -= dt;

        if (it->isBazoka) {
            it->trailTimer += dt;
            if (it->trailTimer >= BAZOKA_TRAIL_SPAWN_INTERVAL) {
                it->trailTimer = 0.0f;
                if ((int)m_bazokaTrails.size() < MAX_BAZOKA_TRAILS) {
                    int idx = CreateOrAcquireBazokaTrailObject();
                float backX = it->x - cosf(it->angleRad) * BAZOKA_TRAIL_BACK_OFFSET;
                float backY = it->y - sinf(it->angleRad) * BAZOKA_TRAIL_BACK_OFFSET;
                Trail t; t.x = backX; t.y = backY; t.life = BAZOKA_TRAIL_LIFETIME; t.objIndex = idx; t.angle = it->angleRad; t.alpha = 1.0f;
                    m_bazokaTrails.push_back(t);
                }
            }
        }

        if (it->life <= 0.0f) { removeBullet(it); continue; }

        if (m_wallCollision) {
            Vector3 pos(it->x, it->y, 0.0f);
            if (m_wallCollision->CheckWallCollision(pos, BULLET_COLLISION_WIDTH, BULLET_COLLISION_HEIGHT, 0.0f, 0.0f)) {
                if (it->isBazoka) {
                    SpawnExplosionAt(it->x, it->y, BAZOKA_EXPLOSION_RADIUS_MUL);
                    if (Camera* cam = SceneManager::GetInstance()->GetActiveCamera()) {
                        cam->AddShake(0.03f, 0.35f, 18.0f);
                    }
                }
                SoundManager::Instance().PlaySFXByID(4, 0); // "WallGetHit"
                removeBullet(it); continue;
            }
        }

        Character* target = (it->ownerId == 1) ? &m_player2 : &m_player;
        Character* attacker = (it->ownerId == 1) ? &m_player : &m_player2;
        if (target) {
            Vector3 targetPos = target->GetPosition();
            float hx = targetPos.x + target->GetHurtboxOffsetX();
            float hy = targetPos.y + target->GetHurtboxOffsetY();
            float halfW = target->GetHurtboxWidth() * 0.5f;
            float halfH = target->GetHurtboxHeight() * 0.5f;
            float tLeft = hx - halfW;
            float tRight = hx + halfW;
            float tBottom = hy - halfH;
            float tTop = hy + halfH;

            float bHalfW = BULLET_COLLISION_WIDTH * 0.5f;
            float bHalfH = BULLET_COLLISION_HEIGHT * 0.5f;
            float bLeft = it->x - bHalfW;
            float bRight = it->x + bHalfW;
            float bBottom = it->y - bHalfH;
            float bTop = it->y + bHalfH;

            if (aabbOverlap(bLeft, bRight, bBottom, bTop, tLeft, tRight, tBottom, tTop)) {
                if (IsCharacterInvincible(*target)) {
                    removeBullet(it); continue;
                }
                
                float prev = target->GetHealth();
                float dmg = it->damage > 0.0f ? it->damage : 10.0f;
                target->TakeDamage(dmg);
                target->CancelAllCombos();
                if (CharacterMovement* mv = target->GetMovement()) {
                    mv->SetInputLocked(false);
                }
                if (prev > 0.0f && target->GetHealth() <= 0.0f && attacker) {
                    target->TriggerDieFromAttack(*attacker);
                }
                SpawnBloodAt(it->x, it->y, it->angleRad);
                if (it->isBazoka) {
                    SpawnExplosionAt(it->x, it->y, BAZOKA_EXPLOSION_RADIUS_MUL);
                    if (Camera* cam = SceneManager::GetInstance()->GetActiveCamera()) {
                        cam->AddShake(0.03f, 0.35f, 18.0f);
                    }
                }
                removeBullet(it); continue;
            }
        }

        ++it;
    }

    for (size_t i = 0; i < m_bazokaTrails.size(); ) {
        Trail& tr = m_bazokaTrails[i];
        tr.life -= dt;
        tr.alpha = (tr.life > 0.0f) ? (tr.life / BAZOKA_TRAIL_LIFETIME) : 0.0f;
        if (tr.life <= 0.0f) {
            if (tr.objIndex >= 0 && tr.objIndex < (int)m_bazokaTrailObjs.size() && m_bazokaTrailObjs[tr.objIndex]) {
                m_freeBazokaTrailSlots.push_back(tr.objIndex);
                m_bazokaTrailObjs[tr.objIndex]->SetVisible(false);
            }
            m_bazokaTrails[i] = m_bazokaTrails.back();
            m_bazokaTrails.pop_back();
        } else {
            if (tr.objIndex >= 0 && tr.objIndex < (int)m_bazokaTrailObjs.size() && !m_bazokaTrailTextures.empty()) {
                float ratio = tr.alpha;
                int idxTex = (ratio > 0.75f) ? 0 : (ratio > 0.5f) ? 1 : (ratio > 0.25f) ? 2 : 3;
                if (idxTex >= (int)m_bazokaTrailTextures.size()) {
                    idxTex = (int)m_bazokaTrailTextures.size() - 1;
                }
                if (idxTex < 0) idxTex = 0;
                m_bazokaTrailObjs[tr.objIndex]->SetDynamicTexture(m_bazokaTrailTextures[idxTex]);
            }
            ++i;
        }
    }
}

int GSPlay::CreateOrAcquireBulletObject() {
    if (!m_freeBulletSlots.empty()) {
        int idx = m_freeBulletSlots.back();
        m_freeBulletSlots.pop_back();
        if (m_bulletObjs[idx]) {
            SceneManager* scene = SceneManager::GetInstance();
            if (Object* proto = scene->GetObject(m_bulletObjectId)) {
                m_bulletObjs[idx]->SetModel(proto->GetModelId());
                const std::vector<int>& texIds = proto->GetTextureIds();
                if (!texIds.empty()) m_bulletObjs[idx]->SetTexture(texIds[0], 0);
                m_bulletObjs[idx]->SetShader(proto->GetShaderId());
                m_bulletObjs[idx]->SetScale(proto->GetScale());
            }
            m_bulletObjs[idx]->SetVisible(true);
        }
        return idx;
    }
    SceneManager* scene = SceneManager::GetInstance();
    Object* proto = scene->GetObject(m_bulletObjectId);
    std::unique_ptr<Object> obj = std::make_unique<Object>(20000 + (int)m_bulletObjs.size());
    if (proto) {
        obj->SetModel(proto->GetModelId());
        const std::vector<int>& texIds = proto->GetTextureIds();
        if (!texIds.empty()) obj->SetTexture(texIds[0], 0);
        obj->SetShader(proto->GetShaderId());
        obj->SetScale(proto->GetScale());
    }
    obj->SetVisible(true);
    m_bulletObjs.push_back(std::move(obj));
    return (int)m_bulletObjs.size() - 1;
}

int GSPlay::CreateOrAcquireBulletObjectFromProto(int protoObjectId) {
    if (!m_freeBulletSlots.empty()) {
        int idx = m_freeBulletSlots.back();
        m_freeBulletSlots.pop_back();
        if (m_bulletObjs[idx]) {
            SceneManager* scene = SceneManager::GetInstance();
            if (Object* proto = scene->GetObject(protoObjectId)) {
                m_bulletObjs[idx]->SetModel(proto->GetModelId());
                const std::vector<int>& texIds = proto->GetTextureIds();
                if (!texIds.empty()) m_bulletObjs[idx]->SetTexture(texIds[0], 0);
                m_bulletObjs[idx]->SetShader(proto->GetShaderId());
                m_bulletObjs[idx]->SetScale(proto->GetScale());
            }
            m_bulletObjs[idx]->SetVisible(true);
        }
        return idx;
    }
    SceneManager* scene = SceneManager::GetInstance();
    Object* proto = scene->GetObject(protoObjectId);
    std::unique_ptr<Object> obj = std::make_unique<Object>(20000 + (int)m_bulletObjs.size());
    if (proto) {
        obj->SetModel(proto->GetModelId());
        const std::vector<int>& texIds = proto->GetTextureIds();
        if (!texIds.empty()) obj->SetTexture(texIds[0], 0);
        obj->SetShader(proto->GetShaderId());
        obj->SetScale(proto->GetScale());
    }
    obj->SetVisible(true);
    m_bulletObjs.push_back(std::move(obj));
    return (int)m_bulletObjs.size() - 1;
}

int GSPlay::CreateOrAcquireBazokaTrailObject() {
    if (m_bazokaTrailTextures.empty()) {
        for (int i = 0; i < 4; ++i) {
            int alpha = 220 - i * 60; if (alpha < 40) alpha = 40;
            auto tex = std::make_shared<Texture2D>();
            if (tex->CreateColorTexture(32, 32, 255, 215, 0, alpha)) {
                m_bazokaTrailTextures.push_back(tex);
            }
        }
    }

    if (!m_freeBazokaTrailSlots.empty()) {
        int idx = m_freeBazokaTrailSlots.back();
        m_freeBazokaTrailSlots.pop_back();
        if (m_bazokaTrailObjs[idx]) {
            m_bazokaTrailObjs[idx]->SetVisible(true);
        }
        return idx;
    }
    std::unique_ptr<Object> obj = std::make_unique<Object>(30000 + (int)m_bazokaTrailObjs.size());
    if (Object* proto = SceneManager::GetInstance()->GetObject(m_bulletObjectId)) {
        obj->SetModel(proto->GetModelId());
        obj->SetShader(proto->GetShaderId());
        const Vector3& sc = proto->GetScale();
        obj->SetScale(sc.x * BAZOKA_TRAIL_SCALE_X, sc.y * BAZOKA_TRAIL_SCALE_Y, sc.z);
        if (!m_bazokaTrailTextures.empty()) obj->SetDynamicTexture(m_bazokaTrailTextures[0]);
    }
    obj->SetVisible(true);
    m_bazokaTrailObjs.push_back(std::move(obj));
    return (int)m_bazokaTrailObjs.size() - 1;
}

void GSPlay::DrawBullets(Camera* cam) {
    for (const Bullet& b : m_bullets) {
        int idx = b.objIndex;
        if (idx >= 0 && idx < (int)m_bulletObjs.size() && m_bulletObjs[idx]) {
            m_bulletObjs[idx]->SetPosition(b.x, b.y, 0.0f);
            float desiredAngle = (b.faceSign < 0.0f) ? (b.angleRad + 3.14159265f) : b.angleRad;
            const Vector3& sc = m_bulletObjs[idx]->GetScale();
            float sx = fabsf(sc.x);
            float sy = fabsf(sc.y);
            if (sy < 1e-6f) sy = 1e-6f;
            float k = sx / sy;
            float c = cosf(desiredAngle);
            float s = sinf(desiredAngle);
            float compensated = atan2f(k * s, c);
            m_bulletObjs[idx]->SetRotation(0.0f, 0.0f, compensated);
            m_bulletObjs[idx]->Draw(cam->GetViewMatrix(), cam->GetProjectionMatrix());
        }
    }

    for (const Trail& t : m_bazokaTrails) {
        int idx = t.objIndex;
        if (idx >= 0 && idx < (int)m_bazokaTrailObjs.size() && m_bazokaTrailObjs[idx]) {
            m_bazokaTrailObjs[idx]->SetPosition(t.x, t.y, 0.0f);
            m_bazokaTrailObjs[idx]->SetRotation(0.0f, 0.0f, t.angle);
            m_bazokaTrailObjs[idx]->Draw(cam->GetViewMatrix(), cam->GetProjectionMatrix());
        }
    }
}

int GSPlay::CreateOrAcquireBloodObjectFromProto(int protoObjectId) {
    if (!m_freeBloodSlots.empty()) {
        int idx = m_freeBloodSlots.back();
        m_freeBloodSlots.pop_back();
        if (m_bloodObjs[idx]) {
            SceneManager* scene = SceneManager::GetInstance();
            if (Object* proto = scene->GetObject(protoObjectId)) {
                m_bloodObjs[idx]->SetModel(proto->GetModelId());
                const std::vector<int>& texIds = proto->GetTextureIds();
                if (!texIds.empty()) m_bloodObjs[idx]->SetTexture(texIds[0], 0);
                m_bloodObjs[idx]->SetShader(proto->GetShaderId());
                m_bloodObjs[idx]->SetScale(proto->GetScale());
            }
            m_bloodObjs[idx]->SetVisible(true);
        }
        return idx;
    }
    SceneManager* scene = SceneManager::GetInstance();
    Object* proto = scene->GetObject(protoObjectId);
    std::unique_ptr<Object> obj = std::make_unique<Object>(40000 + (int)m_bloodObjs.size());
    if (proto) {
        obj->SetModel(proto->GetModelId());
        const std::vector<int>& texIds = proto->GetTextureIds();
        if (!texIds.empty()) obj->SetTexture(texIds[0], 0);
        obj->SetShader(proto->GetShaderId());
        obj->SetScale(proto->GetScale());
    }
    obj->SetVisible(true);
    m_bloodObjs.push_back(std::move(obj));
    return (int)m_bloodObjs.size() - 1;
}

void GSPlay::SpawnBloodAt(float x, float y, float baseAngleRad) {
    float backX = cosf(baseAngleRad) * -0.15f;
    float backY = sinf(baseAngleRad) * -0.05f;

    int protos[3] = { m_bloodProtoIdA, m_bloodProtoIdB, m_bloodProtoIdC };
    for (int i = 0; i < 3; ++i) {
        int idx = CreateOrAcquireBloodObjectFromProto(protos[i]);
        float rx = ((float)rand() / (float)RAND_MAX - 0.5f) * 0.02f;
        float ry = ((float)rand() / (float)RAND_MAX - 0.5f) * 0.02f;
        float angJitter = ((float)rand() / (float)RAND_MAX - 0.5f) * 0.6f;
        float speedMul = 0.6f + ((float)rand() / (float)RAND_MAX) * 0.6f; // [0.6,1.2]

        if (idx >= 0 && idx < (int)m_bloodObjs.size() && m_bloodObjs[idx]) {
            m_bloodObjs[idx]->SetPosition(x + rx, y + ry, 0.0f);
            m_bloodObjs[idx]->SetRotation(0.0f, 0.0f, baseAngleRad + angJitter);
        }

        BloodDrop d;
        d.x = x + rx; d.y = y + ry;
        // initial velocity: slight backward arc, plus small outward component
        float dirX = cosf(baseAngleRad + angJitter);
        float dirY = sinf(baseAngleRad + angJitter);
        d.vx = backX * speedMul + dirX * 0.05f;
        d.vy = backY * speedMul + dirY * 0.03f + 0.05f; // give a tad upward kick
        d.angle = baseAngleRad + angJitter;
        d.objIdx = idx;
        m_bloodDrops.push_back(d);
    }
}

void GSPlay::UpdateBloods(float dt) {
    auto removeDrop = [&](size_t idx){
        BloodDrop& d = m_bloodDrops[idx];
        if (d.objIdx >= 0 && d.objIdx < (int)m_bloodObjs.size() && m_bloodObjs[d.objIdx]) {
            m_bloodObjs[d.objIdx]->SetVisible(false);
            m_freeBloodSlots.push_back(d.objIdx);
        }
        m_bloodDrops[idx] = m_bloodDrops.back();
        m_bloodDrops.pop_back();
    };

    for (size_t i = 0; i < m_bloodDrops.size(); ) {
        BloodDrop& d = m_bloodDrops[i];
        d.vy -= BLOOD_GRAVITY * dt;
        float dx = d.vx * dt;
        float dy = d.vy * dt;
        d.x += dx;
        d.y += dy;

        d.angle += 2.0f * dt;

        if (d.objIdx >= 0 && d.objIdx < (int)m_bloodObjs.size() && m_bloodObjs[d.objIdx]) {
            m_bloodObjs[d.objIdx]->SetPosition(d.x, d.y, 0.0f);
            m_bloodObjs[d.objIdx]->SetRotation(0.0f, 0.0f, d.angle);
        }

        bool collided = false;
        if (m_wallCollision) {
            Vector3 pos(d.x, d.y, 0.0f);
            collided = m_wallCollision->CheckWallCollision(pos, BLOOD_COLLISION_WIDTH, BLOOD_COLLISION_HEIGHT, 0.0f, 0.0f);
        }
        if (collided) {
            removeDrop(i);
        } else {
            ++i;
        }
    }
}

void GSPlay::DrawBloods(Camera* cam) {
    for (const BloodDrop& d : m_bloodDrops) {
        int idx = d.objIdx;
        if (idx >= 0 && idx < (int)m_bloodObjs.size() && m_bloodObjs[idx]) {
            m_bloodObjs[idx]->Draw(cam->GetViewMatrix(), cam->GetProjectionMatrix());
        }
    }
}

void GSPlay::TryCompletePendingShots() {
    auto tryFinish = [&](Character& ch, bool& pendingFlag, float& startTime){
        if (!pendingFlag) return;
        if (ch.IsOrc()) { pendingFlag = false; ch.SetGunMode(false); if (ch.GetMovement()) ch.GetMovement()->SetInputLocked(false); return; }
        // Require minimum time for anim0 + anim1 display
        float elapsed = m_gameTime - startTime;
        if (elapsed < GetGunRequiredTime()) return;
        const bool isP1 = (&ch == &m_player);
        int currentGunTex = isP1 ? m_player1GunTexId : m_player2GunTexId;
        if (currentGunTex == 41 || currentGunTex == 47) { // M4A1 or Uzi
            if (isP1) {
                m_p1BurstActive = true;
                m_p1BurstRemaining = M4A1_BURST_COUNT;
                m_p1NextBurstTime = m_gameTime;
            } else {
                m_p2BurstActive = true;
                m_p2BurstRemaining = M4A1_BURST_COUNT;
                m_p2NextBurstTime = m_gameTime;
            }
            ch.MarkGunShotFired();
            pendingFlag = false;
        } else if (currentGunTex == 42) {
            int& ammoShot = isP1 ? m_p1Ammo42 : m_p2Ammo42;
            if (ammoShot < 5) { pendingFlag = false; ch.SetGunMode(false); if (ch.GetMovement()) ch.GetMovement()->SetInputLocked(false); return; }
            const int pellets = 5;
            const float totalSpreadDeg = 6.0f;
            for (int i = 0; i < pellets; ++i) {
                float t = (pellets == 1) ? 0.0f : (float)i / (float)(pellets - 1);
                float jitter = (t - 0.5f) * totalSpreadDeg;
                SpawnBulletFromCharacterWithJitter(ch, jitter);
            }
            SoundManager::Instance().PlaySFXByID(14, 0);
            ch.MarkGunShotFired();
            pendingFlag = false;
            ammoShot -= 5; UpdateHudAmmoDigits(); StartHudAmmoAnimation(isP1); TryUnequipIfEmpty(42, isP1);
            if ((isP1 && m_player1GunTexId == 42) || (!isP1 && m_player2GunTexId == 42)) {
                if (isP1) {
                    m_p1ReloadPending = true;
                    m_p1ReloadExitTime = m_gameTime + SHOTGUN_RELOAD_TIME;
                } else {
                    m_p2ReloadPending = true;
                    m_p2ReloadExitTime = m_gameTime + SHOTGUN_RELOAD_TIME;
                }
                if (ch.GetMovement()) ch.GetMovement()->SetInputLocked(true);
            } else {
                if (ch.GetMovement()) ch.GetMovement()->SetInputLocked(false);
            }
            if (ch.GetMovement()) ch.GetMovement()->SetInputLocked(true);
        } else if (currentGunTex == 43) { // Bazoka
            int& ammoBaz = isP1 ? m_p1Ammo43 : m_p2Ammo43;
            if (ammoBaz < 1) { pendingFlag = false; ch.SetGunMode(false); if (ch.GetMovement()) ch.GetMovement()->SetInputLocked(false); return; }
            Vector3 pivot = ch.GetGunTopWorldPosition();
            Vector3 base  = ch.GetPosition();
            const float faceSign = ch.IsFacingLeft() ? -1.0f : 1.0f;
            const float aimRad = ch.GetAimAngleDeg() * 3.14159265f / 180.0f;
            const float angleWorld = faceSign * aimRad;
            Vector3 baseSpawn0(base.x + faceSign * BULLET_SPAWN_OFFSET_X,
                               base.y + BULLET_SPAWN_OFFSET_Y, 0.0f);
            Vector3 vLocal0(baseSpawn0.x - pivot.x, baseSpawn0.y - pivot.y, 0.0f);
            float c = cosf(angleWorld), s = sinf(angleWorld);
            Vector3 vRot(vLocal0.x * c - vLocal0.y * s, vLocal0.x * s + vLocal0.y * c, 0.0f);
            Vector3 spawn(pivot.x + vRot.x, pivot.y + vRot.y, 0.0f);
            const float forwardStep = 0.02f;
            Vector3 vLocalForward(faceSign * forwardStep, 0.0f, 0.0f);
            Vector3 vRotForward(vLocalForward.x * c - vLocalForward.y * s,
                                vLocalForward.x * s + vLocalForward.y * c, 0.0f);
            Vector3 dir(vRotForward.x, vRotForward.y, 0.0f);
            float len = dir.Length();
            if (len > 1e-6f) dir = dir / len; else dir = Vector3(faceSign, 0.0f, 0.0f);

            int slot = CreateOrAcquireBulletObjectFromProto(m_bazokaBulletObjectId);
            Bullet b; b.x = spawn.x; b.y = spawn.y;
            b.vx = dir.x * BULLET_SPEED * 1.4f; b.vy = dir.y * BULLET_SPEED * 1.4f;
            b.life = BULLET_LIFETIME; b.objIndex = slot; b.angleRad = angleWorld; b.faceSign = faceSign;
            b.ownerId = isP1 ? 1 : 2; b.damage = 100.0f; b.isBazoka = true; b.trailTimer = 0.0f;
            if ((int)m_bullets.size() < MAX_BULLETS) {
                m_bullets.push_back(b);
            }
            ammoBaz -= 1; UpdateHudAmmoDigits(); StartHudAmmoAnimation(isP1); TryUnequipIfEmpty(43, isP1);
            ch.MarkGunShotFired();
            SoundManager::Instance().PlaySFXByID(24, 0);
            pendingFlag = false;
            ch.SetGunMode(false);
            ch.GetMovement()->SetInputLocked(false);
        } else {
            int& ammoGeneric = AmmoRefFor(currentGunTex, isP1);
            int need = AmmoCostFor(currentGunTex);
            if (ammoGeneric < need) { pendingFlag = false; ch.SetGunMode(false); if (ch.GetMovement()) ch.GetMovement()->SetInputLocked(false); return; }
            SpawnBulletFromCharacter(ch);
            ammoGeneric -= need; UpdateHudAmmoDigits(); StartHudAmmoAnimation(isP1); TryUnequipIfEmpty(currentGunTex, isP1);
            ch.MarkGunShotFired();
            pendingFlag = false;
            ch.SetGunMode(false);
            ch.GetMovement()->SetInputLocked(false);
            if (currentGunTex == 45) {
                SoundManager::Instance().PlaySFXByID(10, 0);
            } else if (currentGunTex == 46) {
                SoundManager::Instance().PlaySFXByID(15, 0);
            } else if (currentGunTex == 40) {
                SoundManager::Instance().PlaySFXByID(13, 0);
            }
        }
    };
    tryFinish(m_player,  m_p1ShotPending, m_p1GunStartTime);
    tryFinish(m_player2, m_p2ShotPending, m_p2GunStartTime);
}

void GSPlay::UpdateGunBursts() {
    auto stepBurst = [&](Character& ch, bool& active, int& remain, float& nextTime){
        if (!active) return;
        if (m_gameTime < nextTime) return;
        const bool isP1Local = (&ch == &m_player);
        int gunTex = isP1Local ? m_player1GunTexId : m_player2GunTexId;
        if (gunTex == 41 || gunTex == 47) {
            int& ammo = (isP1Local ? m_p1Ammo41 : m_p2Ammo41);
            int& ammoUzi = (isP1Local ? m_p1Ammo47 : m_p2Ammo47);
            if (gunTex == 41 && remain == M4A1_BURST_COUNT) {
                if (ammo < 5) { active = false; ch.SetGunMode(false); if (ch.GetMovement()) ch.GetMovement()->SetInputLocked(false); return; }
                ammo -= 5; UpdateHudAmmoDigits(); StartHudAmmoAnimation(isP1Local); TryUnequipIfEmpty(41, isP1Local);
            }
            if (gunTex == 47 && remain == M4A1_BURST_COUNT) {
                if (ammoUzi < 5) { active = false; ch.SetGunMode(false); if (ch.GetMovement()) ch.GetMovement()->SetInputLocked(false); return; }
                ammoUzi -= 5; UpdateHudAmmoDigits(); StartHudAmmoAnimation(isP1Local); TryUnequipIfEmpty(47, isP1Local);
            }
            float r = (float)rand() / (float)RAND_MAX; // [0,1]
            float baseJitter = 1.0f;
            if (gunTex == 47) baseJitter = 3.0f;
            float jitter = (r * 2.0f - 1.0f) * baseJitter;
            SpawnBulletFromCharacterWithJitter(ch, jitter);
            if (gunTex == 41) {
                SoundManager::Instance().PlaySFXByID(12, 0); // GunM4A1
            } else if (gunTex == 47) {
                SoundManager::Instance().PlaySFXByID(16, 0); // GunUzi
            }
            if (gunTex == 41) {
                SoundManager::Instance().PlaySFXByID(12, 0); // GunM4A1
            }
        } else {
            if (!ch.IsOrc()) { SpawnBulletFromCharacter(ch); }
            if (gunTex == 45) {
                SoundManager::Instance().PlaySFXByID(10, 0); // GunDEagle
            } else if (gunTex == 40) {
                SoundManager::Instance().PlaySFXByID(13, 0); // GunPistol
            }
        }
        ch.MarkGunShotFired();
        remain -= 1;
        if (remain > 0) {
            nextTime += M4A1_BURST_INTERVAL;
        } else {
            active = false;
            ch.SetGunMode(false);
            if (ch.GetMovement()) ch.GetMovement()->SetInputLocked(false);
        }
    };
    stepBurst(m_player,  m_p1BurstActive, m_p1BurstRemaining, m_p1NextBurstTime);
    stepBurst(m_player2, m_p2BurstActive, m_p2BurstRemaining, m_p2NextBurstTime);
}

void GSPlay::UpdateGunReloads() {
    if (m_p1ReloadPending && m_gameTime >= m_p1ReloadExitTime) {
        m_p1ReloadPending = false;
        if (m_player1GunTexId == 42) {
            m_player.SetGunMode(false);
        }
        if (m_player.GetMovement()) m_player.GetMovement()->SetInputLocked(false);
    }
    if (m_p2ReloadPending && m_gameTime >= m_p2ReloadExitTime) {
        m_p2ReloadPending = false;
        if (m_player2GunTexId == 42) {
            m_player2.SetGunMode(false);
        }
        if (m_player2.GetMovement()) m_player2.GetMovement()->SetInputLocked(false);
    }
}

static float MousePixelToWorldX(int x, Camera* cam) {
    if (x < 0) x = 0;
    if (x >= Globals::screenWidth) x = Globals::screenWidth - 1;
    
    float left = cam->GetLeft();
    float right = cam->GetRight();
    return left + (right - left) * ((float)x / Globals::screenWidth);
}
static float MousePixelToWorldY(int y, Camera* cam) {
    if (y < 0) y = 0;
    if (y >= Globals::screenHeight) y = Globals::screenHeight - 1;
    
    float top = cam->GetTop();
    float bottom = cam->GetBottom();
    return bottom + (top - bottom) * (1.0f - (float)y / Globals::screenHeight);
}

void GSPlay::HandleMouseEvent(int x, int y, bool bIsPressed) {
    if (!bIsPressed) return;

    SceneManager* sceneManager = SceneManager::GetInstance();
    Camera* camera = sceneManager->GetActiveCamera();
    if (!camera) return;

    float worldX = MousePixelToWorldX(x, camera);
    float worldY = MousePixelToWorldY(y, camera);

    std::cout << "Mouse pixel: (" << x << ", " << y << "), world: (" << worldX << ", " << worldY << ")\n";
    std::cout << "Camera: left=" << camera->GetLeft() << ", right=" << camera->GetRight()
              << ", top=" << camera->GetTop() << ", bottom=" << camera->GetBottom() << std::endl;

    Object* closeBtn = sceneManager->GetObject(MENU_BUTTON_ID);
    if (closeBtn) {
        const Vector3& pos = closeBtn->GetPosition();
        const Vector3& scale = closeBtn->GetScale();
        float width = scale.x;
        float height = scale.y;
        float leftBtn = pos.x - width / 2.0f;
        float rightBtn = pos.x + width / 2.0f;
        float bottomBtn = pos.y - height / 2.0f;
        float topBtn = pos.y + height / 2.0f;
        std::cout << "Button ID " << MENU_BUTTON_ID << " region: left=" << leftBtn << ", right=" << rightBtn
                  << ", top=" << topBtn << ", bottom=" << bottomBtn << std::endl;
        if (worldX >= leftBtn && worldX <= rightBtn && worldY >= bottomBtn && worldY <= topBtn) {
            std::cout << "HIT button ID " << MENU_BUTTON_ID << std::endl;
            std::cout << "[Mouse] Close button clicked in Play!" << std::endl;
            GameStateMachine::GetInstance()->ChangeState(StateType::MENU);
            return;
        }
    }
}

void GSPlay::HandleMouseMove(int x, int y) {
}

void GSPlay::Resume() {
}

void GSPlay::Pause() {
}

void GSPlay::Exit() {
    SoundManager::Instance().StopMusic();
}

void GSPlay::Cleanup() {
    m_animManager = nullptr;
    if (m_inputManager) {
        InputManager::DestroyInstance();
        m_inputManager = nullptr;
    }
} 

// Health system methods
void GSPlay::UpdateHealthBars() {
    SceneManager* sceneManager = SceneManager::GetInstance();
    
    Object* healthBar1 = sceneManager->GetObject(2000);
    if (healthBar1) {
        const Vector3& player1Pos = m_player.GetPosition();
        
        Object* player1Obj = sceneManager->GetObject(1000);
        float characterHeight = 0.24f;
        if (player1Obj) {
            characterHeight = player1Obj->GetScale().y;
        }
        //chỉnh độ cao
        float healthBarOffsetY = characterHeight / 6.0f;
        //chỉnh center
        float healthBarOffsetX = -0.05f;
        healthBar1->SetPosition(player1Pos.x + healthBarOffsetX, player1Pos.y + healthBarOffsetY, 0.0f);
        
        float healthRatio1 = m_player.GetHealth() / m_player.GetMaxHealth();
        const Vector3& scaleRef = healthBar1->GetScale();
        Vector3 currentScale(scaleRef.x, scaleRef.y, scaleRef.z);
        //chỉnh độ dài ngắn
        healthBar1->SetScale(healthRatio1 * 0.18f, currentScale.y, currentScale.z);
    }
    
    Object* healthBar2 = sceneManager->GetObject(2001);
    if (healthBar2) {
        const Vector3& player2Pos = m_player2.GetPosition();
        
        Object* player2Obj = sceneManager->GetObject(1001);
        float characterHeight = 0.24f;
        if (player2Obj) {
            characterHeight = player2Obj->GetScale().y;
        }
        
        float healthBarOffsetY = characterHeight / 6.0f;
        float healthBarOffsetX = -0.05f;
        healthBar2->SetPosition(player2Pos.x + healthBarOffsetX, player2Pos.y + healthBarOffsetY, 0.0f);
        
        float healthRatio2 = m_player2.GetHealth() / m_player2.GetMaxHealth();
        const Vector3& scaleRef = healthBar2->GetScale();
        Vector3 currentScale(scaleRef.x, scaleRef.y, scaleRef.z);
        healthBar2->SetScale(healthRatio2 * 0.18f, currentScale.y, currentScale.z);
    }

    // Update HUD health bars (fixed position)
    Object* hudHealth1 = sceneManager->GetObject(914);
    if (hudHealth1) {
        float healthRatio1 = m_player.GetHealth() / m_player.GetMaxHealth();
        const Vector3& hudScale1 = hudHealth1->GetScale();
        // Base width defined in scene file: 0.94
        hudHealth1->SetScale(healthRatio1 * 0.94f, hudScale1.y, hudScale1.z);
    }

    Object* hudHealth2 = sceneManager->GetObject(915);
    if (hudHealth2) {
        float healthRatio2 = m_player2.GetHealth() / m_player2.GetMaxHealth();
        const Vector3& hudScale2 = hudHealth2->GetScale();
        // Base width defined in scene file: 0.94
        hudHealth2->SetScale(healthRatio2 * 0.94f, hudScale2.y, hudScale2.z);
    }
}

void GSPlay::UpdateStaminaBars() {
    SceneManager* sceneManager = SceneManager::GetInstance();

    // Player 1 stamina bar (ID 2002)
    if (Object* staminaBar1 = sceneManager->GetObject(2002)) {
        const Vector3& player1Pos = m_player.GetPosition();

        Object* player1Obj = sceneManager->GetObject(1000);
        float characterHeight = 0.24f;
        if (player1Obj) {
            characterHeight = player1Obj->GetScale().y;
        }
        float healthBarOffsetY = characterHeight / 6.0f;
        float staminaBarOffsetY = healthBarOffsetY - 0.025f;
        float barOffsetX = -0.05f;
        staminaBar1->SetPosition(player1Pos.x + barOffsetX, player1Pos.y + staminaBarOffsetY, 0.0f);
        float staminaRatio1 = m_player.GetStamina() / m_player.GetMaxStamina();
        const Vector3& scaleRef = staminaBar1->GetScale();
        Vector3 currentScale(scaleRef.x, scaleRef.y, scaleRef.z);
        staminaBar1->SetScale(staminaRatio1 * 0.18f, currentScale.y, currentScale.z);
    }

    if (Object* staminaBar2 = sceneManager->GetObject(2003)) {
        const Vector3& player2Pos = m_player2.GetPosition();

        Object* player2Obj = sceneManager->GetObject(1001);
        float characterHeight = 0.24f;
        if (player2Obj) {
            characterHeight = player2Obj->GetScale().y;
        }
        float healthBarOffsetY = characterHeight / 6.0f;
        float staminaBarOffsetY = healthBarOffsetY - 0.025f;
        float barOffsetX = -0.05f;
        staminaBar2->SetPosition(player2Pos.x + barOffsetX, player2Pos.y + staminaBarOffsetY, 0.0f);
        float staminaRatio2 = m_player2.GetStamina() / m_player2.GetMaxStamina();
        const Vector3& scaleRef2 = staminaBar2->GetScale();
        Vector3 currentScale2(scaleRef2.x, scaleRef2.y, scaleRef2.z);
        staminaBar2->SetScale(staminaRatio2 * 0.18f, currentScale2.y, currentScale2.z);
    }

    if (Object* hudStamina1 = sceneManager->GetObject(932)) {
        float staminaRatio1 = m_player.GetStamina() / m_player.GetMaxStamina();
        const Vector3& hudScale1 = hudStamina1->GetScale();
        hudStamina1->SetScale(staminaRatio1 * 0.94f, hudScale1.y, hudScale1.z);
    }
    if (Object* hudStamina2 = sceneManager->GetObject(933)) {
        float staminaRatio2 = m_player2.GetStamina() / m_player2.GetMaxStamina();
        const Vector3& hudScale2 = hudStamina2->GetScale();
        hudStamina2->SetScale(staminaRatio2 * 0.94f, hudScale2.y, hudScale2.z);
    }
}

void GSPlay::UpdateCloudMovement(float deltaTime) {
    SceneManager* sceneManager = SceneManager::GetInstance();
    
    int cloudIds[] = {51, 52, 53, 54, 55, 56, 57, 58, 59, 60};
    
    for (int cloudId : cloudIds) {
        Object* cloud = sceneManager->GetObject(cloudId);
        if (cloud) {
            const Vector3& currentPos = cloud->GetPosition();
            float newX = currentPos.x - CLOUD_MOVE_SPEED * deltaTime;
            cloud->SetPosition(newX, currentPos.y, currentPos.z);
        }
    }
    
    for (int cloudId : cloudIds) {
        Object* cloud = sceneManager->GetObject(cloudId);
        if (cloud) {
            const Vector3& currentPos = cloud->GetPosition();
            
            if (currentPos.x <= CLOUD_LEFT_BOUNDARY) {
                float rightmostX = -1000.0f;
                for (int otherCloudId : cloudIds) {
                    Object* otherCloud = sceneManager->GetObject(otherCloudId);
                    if (otherCloud) {
                        const Vector3& otherPos = otherCloud->GetPosition();
                        if (otherPos.x > rightmostX) {
                            rightmostX = otherPos.x;
                        }
                    }
                }
                
                float newX = rightmostX + CLOUD_SPACING;
                cloud->SetPosition(newX, currentPos.y, currentPos.z);
            }
        }
    }
}

void GSPlay::UpdateFanRotation(float deltaTime) {
    SceneManager* sceneManager = SceneManager::GetInstance();
    
    int fanIds[] = {800, 801, 802, 803, 804, 805, 806, 807, 808, 809, 810, 811, 812, 813, 814};
    const float FAN_ROTATION_SPEED = 90.0f;
    
    for (int fanId : fanIds) {
        Object* fan = sceneManager->GetObject(fanId);
        if (fan) {
            const Vector3& currentRotation = fan->GetRotation();
            float speed = (fanId == 814) ? (FAN_ROTATION_SPEED * 0.01f) : FAN_ROTATION_SPEED;
            float newZRotation = currentRotation.z + speed * deltaTime;

            if (newZRotation >= 360.0f) {
                newZRotation -= 360.0f;
            }

            fan->SetRotation(currentRotation.x, currentRotation.y, newZRotation);
        }
    }
} 

void GSPlay::HandleItemPickup() {
    if (!m_inputManager) return;
    SceneManager* scene = SceneManager::GetInstance();
    Object* axe   = scene->GetObject(AXE_OBJECT_ID);
    Object* sword = scene->GetObject(SWORD_OBJECT_ID);
    Object* pipe  = scene->GetObject(PIPE_OBJECT_ID);
    // Guns
    Object* gun_pistol  = scene->GetObject(1200);
    Object* gun_m4a1    = scene->GetObject(1201);
    Object* gun_shotgun = scene->GetObject(1202);
    Object* gun_bazoka  = scene->GetObject(1203);
    
    Object* gun_deagle  = scene->GetObject(1205);
    Object* gun_sniper  = scene->GetObject(1206);
    Object* gun_uzi     = scene->GetObject(1207);
    Object* bomb_pickup = scene->GetObject(1502);
    Object* heal_box    = scene->GetObject(1510);
    // Special form items
    Object* item_werewolf = scene->GetObject(1506);
    Object* item_batdemon = scene->GetObject(1507);
    Object* item_kitsune  = scene->GetObject(1508); 
    Object* item_orc      = scene->GetObject(1509);

    const bool* keys = m_inputManager->GetKeyStates();
    if (!keys) return;

    const PlayerInputConfig& cfg1 = m_player.GetMovement()->GetInputConfig();
    bool p1Sit = keys[cfg1.sitKey];
    bool p1PickupJust = m_inputManager->IsKeyJustPressed('1');

    const PlayerInputConfig& cfg2 = m_player2.GetMovement()->GetInputConfig();
    bool p2Sit = keys[cfg2.sitKey];
    bool p2PickupJust = m_inputManager->IsKeyJustPressed('N') || m_inputManager->IsKeyJustPressed('n');
    bool p1CanPickup = !(m_player.IsWerewolf() || m_player.IsBatDemon() || m_player.IsKitsune() || m_player.IsOrc());
    bool p2CanPickup = !(m_player2.IsWerewolf() || m_player2.IsBatDemon() || m_player2.IsKitsune() || m_player2.IsOrc());

    auto isOverlapping = [](const Vector3& pos, float w, float h, const Vector3& objPos, const Vector3& objScale) {
        float halfW = w * 0.5f;
        float halfH = h * 0.5f;
        float l1 = pos.x - halfW, r1 = pos.x + halfW, b1 = pos.y - halfH, t1 = pos.y + halfH;
        float objHalfW = std::abs(objScale.x) * 0.5f;
        float objHalfH = std::abs(objScale.y) * 0.5f;
        float l2 = objPos.x - objHalfW, r2 = objPos.x + objHalfW;
        float b2 = objPos.y - objHalfH, t2 = objPos.y + objHalfH;
        bool overlapX = (r1 >= l2) && (l1 <= r2);
        bool overlapY = (t1 >= b2) && (b1 <= t2);
        return overlapX && overlapY;
    };

    auto tryPickup = [&](Character& player, bool sitHeld, bool pickupJust, Object*& objRef, bool& availFlag, Character::WeaponType weaponType){
        if (!sitHeld || !pickupJust || !objRef) return false;
        const Vector3& objPos = objRef->GetPosition();
        const Vector3& objScale = objRef->GetScale();
        Vector3 pPos = player.GetPosition();
        float w = player.GetHurtboxWidth();
        float h = player.GetHurtboxHeight();
        pPos.x += player.GetHurtboxOffsetX();
        pPos.y += player.GetHurtboxOffsetY();
        if (isOverlapping(pPos, w, h, objPos, objScale)) {
            int removedId = objRef->GetId();
            scene->RemoveObject(removedId);
            MarkSlotPickedByObjectId(removedId);
            objRef = nullptr;
            availFlag = false;
            player.CancelAllCombos();
            player.SetWeapon(weaponType);
            player.SuppressNextPunch();
            std::cout << "Picked up weapon ID " << removedId << " (type=" << (int)weaponType << ")" << std::endl;
            SoundManager::Instance().PlaySFXByID(1, 0);
            return true;
        }
        return false;
    };

    auto tryPickupGun = [&](int texId, Object*& gunObj, Character& player, bool sitHeld, bool pickupJust, bool isPlayer1){
        if (!sitHeld || !pickupJust || !gunObj) return false;
        const Vector3& objPos = gunObj->GetPosition();
        const Vector3& objScale = gunObj->GetScale();
        Vector3 pPos = player.GetPosition();
        float w = player.GetHurtboxWidth();
        float h = player.GetHurtboxHeight();
        pPos.x += player.GetHurtboxOffsetX();
        pPos.y += player.GetHurtboxOffsetY();
        if (isOverlapping(pPos, w, h, objPos, objScale)) {
            int removedId = gunObj->GetId();
            scene->RemoveObject(removedId);
            MarkSlotPickedByObjectId(removedId);
            gunObj = nullptr;
            if (isPlayer1) m_player1GunTexId = texId; else m_player2GunTexId = texId;
            if (CharacterAnimation* a1 = player.GetAnimation()) {
                a1->SetGunByTextureId(texId);
            }
            int cap = AmmoCapacityFor(texId);
            int& ammoRef = AmmoRefFor(texId, isPlayer1);
            ammoRef = cap;
            UpdateHudWeapons();
            UpdateHudAmmoDigits();
            RefreshHudAmmoInstant(isPlayer1);
            player.SuppressNextPunch();
            std::cout << "Picked up gun ID " << removedId << " (tex=" << texId << ")" << std::endl;
            if (texId == 43 || texId == 44) {
                SoundManager::Instance().PlaySFXByID(23, 0);
            } else {
                SoundManager::Instance().PlaySFXByID(11, 0);
            }
            return true;
        }
        return false;
    };

    auto tryPickupFromSlots = [&](Character& player, bool sitHeld, bool pickupJust, bool isPlayer1){
        if (!sitHeld || !pickupJust) return false;
        auto texIdForGun = [](int typeId)->int{
            switch (typeId) {
                case 1200: return 40; // Pistol
                case 1201: return 41; // M4A1
                case 1202: return 42; // Shotgun
                case 1203: return 43; // Bazoka
                case 1205: return 45; // Deagle
                case 1206: return 46; // Sniper
                case 1207: return 47; // Uzi
                default: return -1;
            }
        };
        for (auto& slot : m_spawnSlots) {
            if (!slot.active) continue;
            Object* obj = SceneManager::GetInstance()->GetObject(slot.currentId);
            if (!obj) continue;
            const Vector3& objPos = obj->GetPosition();
            const Vector3& objScale = obj->GetScale();
            Vector3 pPos = player.GetPosition();
            float w = player.GetHurtboxWidth();
            float h = player.GetHurtboxHeight();
            pPos.x += player.GetHurtboxOffsetX();
            pPos.y += player.GetHurtboxOffsetY();
            if (!isOverlapping(pPos, w, h, objPos, objScale)) continue;

            int typeId = slot.typeId;
            if (typeId == AXE_OBJECT_ID || typeId == SWORD_OBJECT_ID || typeId == PIPE_OBJECT_ID) {
                Character::WeaponType wt = Character::WeaponType::None;
                if (typeId == AXE_OBJECT_ID) wt = Character::WeaponType::Axe;
                else if (typeId == SWORD_OBJECT_ID) wt = Character::WeaponType::Sword;
                else if (typeId == PIPE_OBJECT_ID) wt = Character::WeaponType::Pipe;
                SceneManager::GetInstance()->RemoveObject(slot.currentId);
                MarkSlotPickedByObjectId(slot.currentId);
                player.CancelAllCombos();
                player.SetWeapon(wt);
                player.SuppressNextPunch();
                SoundManager::Instance().PlaySFXByID(1, 0);
                return true;
            }
            int gunTex = texIdForGun(typeId);
            if (gunTex != -1) {
                SceneManager::GetInstance()->RemoveObject(slot.currentId);
                MarkSlotPickedByObjectId(slot.currentId);
                if (CharacterAnimation* a1 = player.GetAnimation()) {
                    a1->SetGunByTextureId(gunTex);
                }
                if (isPlayer1) m_player1GunTexId = gunTex; else m_player2GunTexId = gunTex;
                int cap = AmmoCapacityFor(gunTex);
                int& ammoRef = AmmoRefFor(gunTex, isPlayer1);
                ammoRef = cap;
                UpdateHudWeapons();
                UpdateHudAmmoDigits();
                RefreshHudAmmoInstant(isPlayer1);
                player.SuppressNextPunch();
                if (gunTex == 43 || gunTex == 44) { SoundManager::Instance().PlaySFXByID(23, 0); }
                else { SoundManager::Instance().PlaySFXByID(11, 0); }
                return true;
            }
            if (typeId == 1502) { // Bomb pickup
                SceneManager::GetInstance()->RemoveObject(slot.currentId);
                MarkSlotPickedByObjectId(slot.currentId);
                if (isPlayer1) { m_p1Bombs = 3; } else { m_p2Bombs = 3; }
                UpdateHudBombDigits();
                SoundManager::Instance().PlaySFXByID(7, 0);
                return true;
            }
            if (typeId == 1510) { // Heal
                SceneManager::GetInstance()->RemoveObject(slot.currentId);
                MarkSlotPickedByObjectId(slot.currentId);
                player.ResetHealth();
                SoundManager::Instance().PlaySFXByID(17, 0);
                return true;
            }
            if (typeId == 1506 || typeId == 1507 || typeId == 1508 || typeId == 1509) {
                int texId = (typeId == 1506 ? 75 : typeId == 1507 ? 76 : typeId == 1508 ? 77 : 78);
                SceneManager::GetInstance()->RemoveObject(slot.currentId);
                MarkSlotPickedByObjectId(slot.currentId);
                if (isPlayer1) m_p1SpecialItemTexId = texId; else m_p2SpecialItemTexId = texId;
                UpdateHudSpecialIcon(isPlayer1);
                SoundManager::Instance().PlaySFXByID(17, 0);
                return true;
            }
        }
        return false;
    };

    if ( p1CanPickup && tryPickupFromSlots(m_player, p1Sit, p1PickupJust, true) ) { return; }
    if ( p2CanPickup && tryPickupFromSlots(m_player2, p2Sit, p2PickupJust, false) ) { return; }

    // Try pick up bomb (Player 1)
    auto tryPickupBomb = [&](Object*& bombObj, Character& player, bool sitHeld, bool pickupJust, bool isPlayer1){
        if (!sitHeld || !pickupJust || !bombObj) return false;
        const Vector3& objPos = bombObj->GetPosition();
        const Vector3& objScale = bombObj->GetScale();
        Vector3 pPos = player.GetPosition();
        float w = player.GetHurtboxWidth();
        float h = player.GetHurtboxHeight();
        pPos.x += player.GetHurtboxOffsetX();
        pPos.y += player.GetHurtboxOffsetY();
        if (isOverlapping(pPos, w, h, objPos, objScale)) {
            int removedId = bombObj->GetId();
            scene->RemoveObject(removedId);
            MarkSlotPickedByObjectId(removedId);
            bombObj = nullptr;
            if (isPlayer1) { m_p1Bombs = 3; } else { m_p2Bombs = 3; }
            UpdateHudBombDigits();
            std::cout << "Picked up bomb ID " << removedId << " -> refill to 3\n";
            SoundManager::Instance().PlaySFXByID(7, 0);
            return true;
        }
        return false;
    };

    auto tryPickupHeal = [&](Object*& healObj, Character& player, bool sitHeld, bool pickupJust){
        if (!sitHeld || !pickupJust || !healObj) return false;
        const Vector3& objPos = healObj->GetPosition();
        const Vector3& objScale = healObj->GetScale();
        Vector3 pPos = player.GetPosition();
        float w = player.GetHurtboxWidth();
        float h = player.GetHurtboxHeight();
        pPos.x += player.GetHurtboxOffsetX();
        pPos.y += player.GetHurtboxOffsetY();
        if (isOverlapping(pPos, w, h, objPos, objScale)) {
            int removedId = healObj->GetId();
            scene->RemoveObject(removedId);
            MarkSlotPickedByObjectId(removedId);
            healObj = nullptr;
            player.ResetHealth();
            std::cout << "Picked up heal box ID " << removedId << " -> health restored to max\n";
            SoundManager::Instance().PlaySFXByID(17, 0);
            return true;
        }
        return false;
    };

    auto tryPickupSpecial = [&](Object*& itemObj, int texId, Character& player, bool sitHeld, bool pickupJust, bool isPlayer1){
        if (!sitHeld || !pickupJust || !itemObj) return false;
        const Vector3& objPos = itemObj->GetPosition();
        const Vector3& objScale = itemObj->GetScale();
        Vector3 pPos = player.GetPosition();
        float w = player.GetHurtboxWidth();
        float h = player.GetHurtboxHeight();
        pPos.x += player.GetHurtboxOffsetX();
        pPos.y += player.GetHurtboxOffsetY();
        if (isOverlapping(pPos, w, h, objPos, objScale)) {
            int removedId = itemObj->GetId();
            scene->RemoveObject(removedId);
            itemObj = nullptr;
            if (isPlayer1) m_p1SpecialItemTexId = texId; else m_p2SpecialItemTexId = texId;
            UpdateHudSpecialIcon(isPlayer1);
            SoundManager::Instance().PlaySFXByID(17, 0);
            return true;
        }
        return false;
    };

    if ( p1CanPickup && tryPickupBomb(bomb_pickup, m_player,  p1Sit, p1PickupJust, true) ) { return; }
    if ( p1CanPickup && tryPickupHeal(heal_box, m_player, p1Sit, p1PickupJust) ) { return; }
    if ( p1CanPickup && tryPickupSpecial(item_werewolf, 75, m_player, p1Sit, p1PickupJust, true) ) { return; }
    if ( p1CanPickup && tryPickupSpecial(item_batdemon, 76, m_player, p1Sit, p1PickupJust, true) ) { return; }
    if ( p1CanPickup && tryPickupSpecial(item_kitsune,  77, m_player, p1Sit, p1PickupJust, true) ) { return; }
    if ( p1CanPickup && tryPickupSpecial(item_orc,      78, m_player, p1Sit, p1PickupJust, true) ) { return; }

    // Check Player 1 melee
    if ( p1CanPickup && ( tryPickup(m_player,  p1Sit, p1PickupJust, axe,   m_isAxeAvailable,   Character::WeaponType::Axe)   ||
         tryPickup(m_player,  p1Sit, p1PickupJust, sword, m_isSwordAvailable, Character::WeaponType::Sword) ||
         tryPickup(m_player,  p1Sit, p1PickupJust, pipe,  m_isPipeAvailable,  Character::WeaponType::Pipe) ) ) { return; }
    // Check Player 1 guns
    if ( p1CanPickup && ( tryPickupGun(40, gun_pistol,  m_player, p1Sit, p1PickupJust, true)  ||
         tryPickupGun(41, gun_m4a1,    m_player, p1Sit, p1PickupJust, true)  ||
         tryPickupGun(42, gun_shotgun, m_player, p1Sit, p1PickupJust, true)  ||
         tryPickupGun(43, gun_bazoka,  m_player, p1Sit, p1PickupJust, true)  ||
         
         tryPickupGun(45, gun_deagle,  m_player, p1Sit, p1PickupJust, true)  ||
         tryPickupGun(46, gun_sniper,  m_player, p1Sit, p1PickupJust, true)  ||
         tryPickupGun(47, gun_uzi,     m_player, p1Sit, p1PickupJust, true) ) ) { return; }

    // Try pick up bomb (Player 2)
    if ( p2CanPickup && tryPickupBomb(bomb_pickup, m_player2, p2Sit, p2PickupJust, false) ) { return; }
    if ( p2CanPickup && tryPickupHeal(heal_box, m_player2, p2Sit, p2PickupJust) ) { return; }
    if ( p2CanPickup && tryPickupSpecial(item_werewolf, 75, m_player2, p2Sit, p2PickupJust, false) ) { return; }
    if ( p2CanPickup && tryPickupSpecial(item_batdemon, 76, m_player2, p2Sit, p2PickupJust, false) ) { return; }
    if ( p2CanPickup && tryPickupSpecial(item_kitsune,  77, m_player2, p2Sit, p2PickupJust, false) ) { return; }
    if ( p2CanPickup && tryPickupSpecial(item_orc,      78, m_player2, p2Sit, p2PickupJust, false) ) { return; }

    // Check Player 2
    if ( p2CanPickup && ( tryPickup(m_player2, p2Sit, p2PickupJust, axe,   m_isAxeAvailable,   Character::WeaponType::Axe)   ||
         tryPickup(m_player2, p2Sit, p2PickupJust, sword, m_isSwordAvailable, Character::WeaponType::Sword) ||
         tryPickup(m_player2, p2Sit, p2PickupJust, pipe,  m_isPipeAvailable,  Character::WeaponType::Pipe) ) ) { return; }
    // Check Player 2 guns
    if ( p2CanPickup && ( tryPickupGun(40, gun_pistol,  m_player2, p2Sit, p2PickupJust, false) ||
         tryPickupGun(41, gun_m4a1,    m_player2, p2Sit, p2PickupJust, false) ||
         tryPickupGun(42, gun_shotgun, m_player2, p2Sit, p2PickupJust, false) ||
         tryPickupGun(43, gun_bazoka,  m_player2, p2Sit, p2PickupJust, false) ||
         
         tryPickupGun(45, gun_deagle,  m_player2, p2Sit, p2PickupJust, false) ||
         tryPickupGun(46, gun_sniper,  m_player2, p2Sit, p2PickupJust, false) ||
         tryPickupGun(47, gun_uzi,     m_player2, p2Sit, p2PickupJust, false) ) ) { return; }
}

void GSPlay::SpawnEnergyOrbProjectile(Character& character) {
    character.ResetKitsuneEnergyOrbAnimationComplete();
    
    EnergyOrbProjectile* projectile = nullptr;
    
    for (auto& proj : m_energyOrbProjectiles) {
        if (!proj->IsActive()) {
            projectile = proj.get();
            break;
        }
    }
    
    if (!projectile && m_energyOrbProjectiles.size() < MAX_ENERGY_ORB_PROJECTILES) {
        m_energyOrbProjectiles.push_back(std::make_unique<EnergyOrbProjectile>());
        projectile = m_energyOrbProjectiles.back().get();
        projectile->Initialize();
        projectile->SetWallCollision(m_wallCollision.get());
        projectile->SetExplosionCallback([this](float x) { SpawnLightningEffect(x); });
    }
    
    if (projectile) {
        Vector3 charPos = character.GetPosition();
        bool facingLeft = character.IsFacingLeft();
        
        Vector3 spawnPos = charPos;
        spawnPos.x += facingLeft ? -0.15f : 0.15f;
        spawnPos.y -= 0.05f;
        
        Vector3 direction(facingLeft ? -1.0f : 1.0f, 0.0f, 0.0f);
        
        int ownerId = (&character == &m_player) ? 1 : 2;
        
        projectile->Spawn(spawnPos, direction, 2.0f, ownerId);
    }
}

void GSPlay::UpdateEnergyOrbProjectiles(float deltaTime) {
    for (auto& projectile : m_energyOrbProjectiles) {
        if (projectile->IsActive()) {
            projectile->Update(deltaTime);
        }
    }
}

void GSPlay::DrawEnergyOrbProjectiles(Camera* camera) {
    for (auto& projectile : m_energyOrbProjectiles) {
        if (projectile->IsActive()) {
            projectile->Draw(camera);
        }
    }
}

void GSPlay::DetonatePlayerProjectiles(int playerId) {
    for (auto& projectile : m_energyOrbProjectiles) {
        if (projectile->IsActive() && !projectile->IsExploding() && projectile->GetOwnerId() == playerId) {
            projectile->TriggerExplosion();
        }
    }
}

void GSPlay::SpawnLightningEffect(float x) {
    LightningEffect* lightning = nullptr;
    
    for (auto& effect : m_lightningEffects) {
        if (!effect.isActive) {
            lightning = &effect;
            break;
        }
    }
    
    if (!lightning && m_lightningEffects.size() < MAX_LIGHTNING_EFFECTS) {
        m_lightningEffects.push_back(LightningEffect{});
        lightning = &m_lightningEffects.back();
    }
    
    if (lightning) {
        int objIndex = CreateOrAcquireLightningObject();
        if (objIndex >= 0) {
            lightning->x = x;
            lightning->lifetime = 0.0f;
            lightning->maxLifetime = 3.0f; // 3s
            lightning->objectIndex = objIndex;
            lightning->isActive = true;
            lightning->currentFrame = 0;
            lightning->frameTimer = 0.0f;
            lightning->frameDuration = 3.0f / 30.0f;
            
            // Hitbox Lightning
            lightning->hitboxLeft = x - 0.21875f;
            lightning->hitboxRight = x + 0.21875f;
            lightning->hitboxTop = 1.95f;
            lightning->hitboxBottom = -1.95f;
            lightning->hasDealtDamage = false;
            
            // Set lightning object position and scale
            if (objIndex < (int)m_lightningObjects.size() && m_lightningObjects[objIndex]) {
                m_lightningObjects[objIndex]->SetPosition(x, 0.0f, 0.0f);
                m_lightningObjects[objIndex]->SetScale(1.0f, -3.9f, 1.0f);
                
                float frameWidth = 1.0f / 32.0f;
                float uStart = 0.0f;
                float uEnd = frameWidth;
                m_lightningObjects[objIndex]->SetCustomUV(uStart, 1.0f, uEnd, 0.0f);
                if (Camera* cam = SceneManager::GetInstance()->GetActiveCamera()) {
                    cam->AddShake(0.05f, lightning->maxLifetime, 22.0f);
                }
            }
        }
    }
}

void GSPlay::UpdateLightningEffects(float deltaTime) {
    for (auto& lightning : m_lightningEffects) {
        if (lightning.isActive) {
            lightning.lifetime += deltaTime;
            lightning.frameTimer += deltaTime;

            if (Camera* cam = SceneManager::GetInstance()->GetActiveCamera()) {
                cam->AddShake(0.03f, 0.08f, 22.0f);
            }
            
            if (lightning.frameTimer >= lightning.frameDuration) {
                lightning.frameTimer = 0.0f;
                lightning.currentFrame++;
                
                if (lightning.objectIndex >= 0 && lightning.objectIndex < (int)m_lightningObjects.size() && m_lightningObjects[lightning.objectIndex]) {
                    float frameWidth = 1.0f / 32.0f;
                    float uStart = lightning.currentFrame * frameWidth;
                    float uEnd = uStart + frameWidth;
                    m_lightningObjects[lightning.objectIndex]->SetCustomUV(uStart, 1.0f, uEnd, 0.0f); 
                }
            }
            
            if (lightning.lifetime >= lightning.maxLifetime) {
                lightning.isActive = false;
                if (lightning.objectIndex >= 0 && lightning.objectIndex < (int)m_freeLightningSlots.size()) {
                    m_freeLightningSlots.push_back(lightning.objectIndex);
                }
            }
        }
    }
}

void GSPlay::DrawLightningEffects(Camera* camera) {
    for (auto& lightning : m_lightningEffects) {
        if (lightning.isActive && lightning.objectIndex >= 0 && lightning.objectIndex < (int)m_lightningObjects.size()) {
            if (m_lightningObjects[lightning.objectIndex]) {
                m_lightningObjects[lightning.objectIndex]->Draw(camera->GetViewMatrix(), camera->GetProjectionMatrix());
            }
        }
    }
}

int GSPlay::CreateOrAcquireLightningObject() {
    if (!m_freeLightningSlots.empty()) {
        int idx = m_freeLightningSlots.back();
        m_freeLightningSlots.pop_back();
        if (m_lightningObjects[idx]) {
            m_lightningObjects[idx]->SetVisible(true);
        }
        return idx;
    }
    
    std::unique_ptr<Object> obj = std::make_unique<Object>(60000 + (int)m_lightningObjects.size());
    
    obj->SetModel(0);
    obj->SetTexture(64, 0);
    obj->SetShader(0);
    obj->SetScale(1.0f, -3.6f, 1.0f);
    obj->SetVisible(true);
    
    obj->MakeModelInstanceCopy();
    
    m_lightningObjects.push_back(std::move(obj));
    return (int)m_lightningObjects.size() - 1;
}

void GSPlay::CheckLightningDamage() {
    for (auto& lightning : m_lightningEffects) {
        if (lightning.isActive && !lightning.hasDealtDamage) {
            Vector3 playerPos = m_player.GetPosition();
            float playerHurtboxX = playerPos.x + m_player.GetHurtboxOffsetX();
            float playerHurtboxY = playerPos.y + m_player.GetHurtboxOffsetY();
            float playerHurtboxLeft = playerHurtboxX - m_player.GetHurtboxWidth() * 0.5f;
            float playerHurtboxRight = playerHurtboxX + m_player.GetHurtboxWidth() * 0.5f;
            float playerHurtboxTop = playerHurtboxY + m_player.GetHurtboxHeight() * 0.5f;
            float playerHurtboxBottom = playerHurtboxY - m_player.GetHurtboxHeight() * 0.5f;
            
            bool collisionX1 = lightning.hitboxRight >= playerHurtboxLeft && lightning.hitboxLeft <= playerHurtboxRight;
            bool collisionY1 = lightning.hitboxTop >= playerHurtboxBottom && lightning.hitboxBottom <= playerHurtboxTop;
            
            if (collisionX1 && collisionY1) {
                if (!IsCharacterInvincible(m_player)) {
                    m_player.TakeDamage(100);
                    lightning.hasDealtDamage = true;
                }
            }
            
            Vector3 player2Pos = m_player2.GetPosition();
            float player2HurtboxX = player2Pos.x + m_player2.GetHurtboxOffsetX();
            float player2HurtboxY = player2Pos.y + m_player2.GetHurtboxOffsetY();
            float player2HurtboxLeft = player2HurtboxX - m_player2.GetHurtboxWidth() * 0.5f;
            float player2HurtboxRight = player2HurtboxX + m_player2.GetHurtboxWidth() * 0.5f;
            float player2HurtboxTop = player2HurtboxY + m_player2.GetHurtboxHeight() * 0.5f;
            float player2HurtboxBottom = player2HurtboxY - m_player2.GetHurtboxHeight() * 0.5f;
            
            bool collisionX2 = lightning.hitboxRight >= player2HurtboxLeft && lightning.hitboxLeft <= player2HurtboxRight;
            bool collisionY2 = lightning.hitboxTop >= player2HurtboxBottom && lightning.hitboxBottom <= player2HurtboxTop;
            
            if (collisionX2 && collisionY2) {
                if (!IsCharacterInvincible(m_player2)) {
                    m_player2.TakeDamage(100);
                    lightning.hasDealtDamage = true;
                }
            }
        }
    }
}

void GSPlay::InitializeRespawnSlots() {
    const Vector3 kRespawnPositions[11] = {
        Vector3(-3.5f,  1.55f, 0.0f),
        Vector3(-3.25f, -1.1f, 0.0f),
        Vector3(-2.6f, -0.25f, 0.0f),
        Vector3(-2.3f, -1.2f, 0.0f),
        Vector3(-1.5f, -1.31f, 0.0f),
        Vector3(-0.35f, -0.22f, 0.0f),
        Vector3( 0.2f, -0.84f, 0.0f),
        Vector3( 1.25f, -1.2f, 0.0f),
        Vector3( 1.73f, -0.36f, 0.0f),
        Vector3( 2.4f, -1.2f, 0.0f),
        Vector3( 3.2f, -0.36f, 0.0f)
    };
    
    m_respawnSlots.clear();
    m_respawnSlots.resize(11);
    for (int i = 0; i < 11; ++i) {
        m_respawnSlots[i].pos = kRespawnPositions[i];
        m_respawnSlots[i].occupied = false;
    }
}

void GSPlay::UpdateCharacterRespawn(float deltaTime) {
    if (m_player.IsDead() && !m_p1Respawned) {
        RespawnCharacter(m_player);
        m_p1Respawned = true;
    }
    
    if (m_player2.IsDead() && !m_p2Respawned) {
        RespawnCharacter(m_player2);
        m_p2Respawned = true;
    }
    
    if (!m_player.IsDead()) {
        m_p1Respawned = false;
    }
    if (!m_player2.IsDead()) {
        m_p2Respawned = false;
    }
}

void GSPlay::RespawnCharacter(Character& character) {
    bool isPlayer1 = (&character == &m_player);
    
    int respawnIndex = ChooseRandomRespawnPosition();
    if (respawnIndex >= 0 && respawnIndex < (int)m_respawnSlots.size()) {
        const Vector3& respawnPos = m_respawnSlots[respawnIndex].pos;
        
        character.SetPosition(respawnPos.x, respawnPos.y);
        ResetCharacterToInitialState(character, isPlayer1);
        
        if (isPlayer1) {
            m_p1Invincible = true;
            m_p1InvincibilityTimer = 0.0f;
        } else {
            m_p2Invincible = true;
            m_p2InvincibilityTimer = 0.0f;
        }
        
        std::cout << "Character respawned at position " << respawnIndex 
                  << " (" << respawnPos.x << ", " << respawnPos.y << ")" << std::endl;
    }
}

void GSPlay::ResetCharacterToInitialState(Character& character, bool isPlayer1) {
    character.ResetHealth();
    character.ResetStamina();
    
    character.SetWeapon(Character::WeaponType::None);
    
    character.SetGunMode(false);
    character.SetGrenadeMode(false);
    character.SetBatDemonMode(false);
    character.SetWerewolfMode(false);
    character.SetKitsuneMode(false);
    character.SetOrcMode(false);
    
    if (CharacterMovement* movement = character.GetMovement()) {
        movement->SetInputLocked(false);
        movement->SetNoClipNoGravity(false);
        movement->SetMoveSpeedMultiplier(1.0f);
        movement->SetLadderDoubleTapEnabled(true);
        movement->SetLadderEnabled(true);
        
        movement->ResetDieState();
    }
    
    character.CancelAllCombos();
    
    if (isPlayer1) {
        m_player1GunTexId = 40;
        m_p1Ammo40 = 15; m_p1Ammo41 = 30; m_p1Ammo42 = 60; m_p1Ammo43 = 3;
        m_p1Ammo45 = 12; m_p1Ammo46 = 6; m_p1Ammo47 = 30;
        m_p1Bombs = 3;
        m_p1SpecialItemTexId = -1;
        m_p1SpecialExpireTime = -1.0f;
        m_p1SpecialType = 0;
        
        if (CharacterAnimation* anim = character.GetAnimation()) {
            anim->SetGunByTextureId(40);
        }
    } else {
        m_player2GunTexId = 40;
        m_p2Ammo40 = 15; m_p2Ammo41 = 30; m_p2Ammo42 = 60; m_p2Ammo43 = 3;
        m_p2Ammo45 = 12; m_p2Ammo46 = 6; m_p2Ammo47 = 30;
        m_p2Bombs = 3;
        m_p2SpecialItemTexId = -1;
        m_p2SpecialExpireTime = -1.0f;
        m_p2SpecialType = 0;
        
        if (CharacterAnimation* anim = character.GetAnimation()) {
            anim->SetGunByTextureId(40);
        }
    }
    
    UpdateHudWeapons();
    UpdateHudAmmoDigits();
    RefreshHudAmmoInstant(true);
    RefreshHudAmmoInstant(false);
    UpdateHudBombDigits();
    UpdateHudSpecialIcon(true);
    UpdateHudSpecialIcon(false);
}

int GSPlay::ChooseRandomRespawnPosition() {
    if (m_respawnSlots.empty()) return -1;
    
    std::vector<int> availablePositions;
    for (int i = 0; i < (int)m_respawnSlots.size(); ++i) {
        if (!m_respawnSlots[i].occupied) {
            availablePositions.push_back(i);
        }
    }
    
    if (availablePositions.empty()) {
        for (int i = 0; i < (int)m_respawnSlots.size(); ++i) {
            availablePositions.push_back(i);
        }
    }
    
    if (!availablePositions.empty()) {
        static std::mt19937 rng{ std::random_device{}() };
        std::uniform_int_distribution<int> dist(0, (int)availablePositions.size() - 1);
        int randomIndex = dist(rng);
        return availablePositions[randomIndex];
    }
    
    return -1;
}

void GSPlay::UpdateRespawnInvincibility(float deltaTime) {
    if (m_p1Invincible) {
        m_p1InvincibilityTimer += deltaTime;
        
        bool shouldBeVisible = fmodf(m_p1InvincibilityTimer, RESPAWN_BLINK_INTERVAL * 2.0f) < RESPAWN_BLINK_INTERVAL;
        if (CharacterAnimation* anim = m_player.GetAnimation()) {
            if (Object* obj = anim->GetCharacterObject()) {
                obj->SetVisible(shouldBeVisible);
            }
        }
        
        if (m_p1InvincibilityTimer >= RESPAWN_INVINCIBILITY_DURATION) {
            m_p1Invincible = false;
            m_p1InvincibilityTimer = 0.0f;
            if (CharacterAnimation* anim = m_player.GetAnimation()) {
                if (Object* obj = anim->GetCharacterObject()) {
                    obj->SetVisible(true);
                }
            }
        }
    }
    
    if (m_p2Invincible) {
        m_p2InvincibilityTimer += deltaTime;
        
        bool shouldBeVisible = fmodf(m_p2InvincibilityTimer, RESPAWN_BLINK_INTERVAL * 2.0f) < RESPAWN_BLINK_INTERVAL;
        if (CharacterAnimation* anim = m_player2.GetAnimation()) {
            if (Object* obj = anim->GetCharacterObject()) {
                obj->SetVisible(shouldBeVisible);
            }
        }
        
        if (m_p2InvincibilityTimer >= RESPAWN_INVINCIBILITY_DURATION) {
            m_p2Invincible = false;
            m_p2InvincibilityTimer = 0.0f;
            if (CharacterAnimation* anim = m_player2.GetAnimation()) {
                if (Object* obj = anim->GetCharacterObject()) {
                    obj->SetVisible(true);
                }
            }
        }
    }
}

void GSPlay::UpdateGameStartBlink(float deltaTime) {
    if (m_gameStartBlinkActive) {
        m_gameStartBlinkTimer += deltaTime;
        
        bool shouldBeVisible = fmodf(m_gameStartBlinkTimer, GAME_START_BLINK_INTERVAL * 2.0f) < GAME_START_BLINK_INTERVAL;
        
        if (CharacterAnimation* anim = m_player.GetAnimation()) {
            if (Object* obj = anim->GetCharacterObject()) {
                obj->SetVisible(shouldBeVisible);
            }
        }
        
        if (CharacterAnimation* anim = m_player2.GetAnimation()) {
            if (Object* obj = anim->GetCharacterObject()) {
                obj->SetVisible(shouldBeVisible);
            }
        }
        
        if (m_gameStartBlinkTimer >= GAME_START_BLINK_DURATION) {
            m_gameStartBlinkActive = false;
            m_gameStartBlinkTimer = 0.0f;
            
            if (CharacterAnimation* anim = m_player.GetAnimation()) {
                if (Object* obj = anim->GetCharacterObject()) {
                    obj->SetVisible(true);
                }
            }
            if (CharacterAnimation* anim = m_player2.GetAnimation()) {
                if (Object* obj = anim->GetCharacterObject()) {
                    obj->SetVisible(true);
                }
            }
        }
    }
}

bool GSPlay::IsCharacterInvincible(const Character& character) const {
    return (&character == &m_player && m_p1Invincible) || 
           (&character == &m_player2 && m_p2Invincible);
}