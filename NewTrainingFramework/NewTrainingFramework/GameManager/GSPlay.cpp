#include "stdafx.h"
#include "GSPlay.h"
#include "GameStateMachine.h"
#include "../Core/Globals.h"
#include "../../Utilities/Math.h"
#include <iostream>
#include <cmath>
#include <algorithm>
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

bool GSPlay_IsShowPlatformBoxes() {
    return GSPlay::IsShowPlatformBoxes();
}

GSPlay::GSPlay() 
    : GameStateBase(StateType::PLAY), m_gameTime(0.0f), m_player1Health(100.0f), m_player2Health(100.0f), m_cloudSpeed(0.5f) {
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
    
    m_player.GetMovement()->ClearPlatforms();
    m_player2.GetMovement()->ClearPlatforms();
    m_player.GetMovement()->ClearMovingPlatforms();
    m_player2.GetMovement()->ClearMovingPlatforms();
    {
        std::ifstream sceneFile("Resources/Scenes/GSPlay.txt");
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
            0.49f, -0.862f, 0.0f,  // Start position
            0.49f, 0.33f, 0.0f,   // End position
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

    // Apply glint shader to world weapons (IDs 1100/1101/1102)
    auto applyGlint = [&](int objId){ if (Object* o = sceneManager->GetObject(objId)) { o->SetShader(1); } };
    applyGlint(AXE_OBJECT_ID);
    applyGlint(SWORD_OBJECT_ID);
    applyGlint(PIPE_OBJECT_ID);
    int glintPickupIds[] = {1200,1201,1202,1203,1204,1205,1206,1207,1502};
    for (int gid : glintPickupIds) {
        applyGlint(gid);
    }

    // Initialize lifetime tracking for item objects
    m_itemLives.clear();
    auto tryAdd = [&](int id){ if (sceneManager->GetObject(id)) m_itemLives.push_back({id, 0.0f}); };
    tryAdd(AXE_OBJECT_ID);
    tryAdd(SWORD_OBJECT_ID);
    tryAdd(PIPE_OBJECT_ID);
    int pickupIds[] = {1200,1201,1202,1203,1204,1205,1206,1207,1502};
    for (int pid : pickupIds) { tryAdd(pid); }

    // Initialize HUD weapons: cache base scales and hide by default
    if (Object* hudWeapon1 = sceneManager->GetObject(918)) {
        m_hudWeapon1BaseScale = hudWeapon1->GetScale();
        hudWeapon1->SetScale(0.0f, 0.0f, m_hudWeapon1BaseScale.z);
    }
    if (Object* hudWeapon2 = sceneManager->GetObject(919)) {
        m_hudWeapon2BaseScale = hudWeapon2->GetScale();
        hudWeapon2->SetScale(0.0f, 0.0f, m_hudWeapon2BaseScale.z);
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
            case 44: return isP1 ? m_p1Ammo44 : m_p2Ammo44;
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
        case 44: return isPlayer1 ? m_p1Ammo44 : m_p2Ammo44;
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
        case 44: // Flamegun
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
        Vector3 base = m_hudBombIcon1BaseScale.x != 0.0f || m_hudBombIcon1BaseScale.y != 0.0f ? m_hudBombIcon1BaseScale : bombIcon1->GetScale();
        bombIcon1->SetScale(m_p1Bombs > 0 ? base : Vector3(0.0f, 0.0f, base.z));
    }
    if (Object* bombIcon2 = scene->GetObject(923)) {
        Vector3 base = m_hudBombIcon2BaseScale.x != 0.0f || m_hudBombIcon2BaseScale.y != 0.0f ? m_hudBombIcon2BaseScale : bombIcon2->GetScale();
        bombIcon2->SetScale(m_p2Bombs > 0 ? base : Vector3(0.0f, 0.0f, base.z));
    }
    if (m_p1Bombs > 0) setTwoDigits(m_p1Bombs, 928, 929);
    if (m_p2Bombs > 0) setTwoDigits(m_p2Bombs, 930, 931);
    TTF_CloseFont(font);
}

int GSPlay::AmmoCapacityFor(int texId) const {
    switch (texId) {
        case 40: return 15; // Pistol
        case 41: return 30; // M4A1
        case 42: return 60; // Shotgun
        case 43: return 3;  // Bazoka
        case 44: return 30; // Flamegun
        case 45: return 12; // Deagle
        case 46: return 6;  // Sniper
        case 47: return 30; // Uzi
        default: return 0;
    }
}

void GSPlay::Update(float deltaTime) {
    m_gameTime += deltaTime;
    
    SceneManager::GetInstance()->Update(deltaTime);
    
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

    if (m_player.GetMovement() && m_player.GetMovement()->ConsumeJustStartedUpwardJump()) {
        SoundManager::Instance().PlaySFXByID(18, 0);
    }
    if (m_player2.GetMovement() && m_player2.GetMovement()->ConsumeJustStartedUpwardJump()) {
        SoundManager::Instance().PlaySFXByID(18, 0);
    }
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
    
    if (m_player2.CheckHitboxCollision(m_player)) {
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
    
    Camera* camera = SceneManager::GetInstance()->GetActiveCamera();
    if (camera && camera->IsAutoZoomEnabled()) {
        Vector3 player1Pos = m_player.GetPosition();
        Vector3 player2Pos = m_player2.GetPosition();
        camera->UpdateCameraForCharacters(player1Pos, player2Pos, deltaTime);
    }
    
    m_player1Health = m_player.GetHealth();
    m_player2Health = m_player2.GetHealth();
    
    UpdateHealthBars();
    
    Object* menuButton = SceneManager::GetInstance()->GetObject(MENU_BUTTON_ID);
    if (menuButton) {
        menuButton->SetScale(Vector3(0.2f, 0.1f, 1.0f));
    }
    
    // Update cloud movement
    UpdateCloudMovement(deltaTime);
    
    // Update fan rotation
    UpdateFanRotation(deltaTime);

    {
        SceneManager* scene = SceneManager::GetInstance();
        const float LIFETIME = 20.0f;
        const float BLINK_START = 12.0f;
        static float blinkTimer = 0.0f;
        blinkTimer += deltaTime;
        bool blinkVisible = fmodf(blinkTimer, 0.3f) < 0.15f; // toggle every 0.15s

        for (auto it = m_itemLives.begin(); it != m_itemLives.end(); ) {
            it->timer += deltaTime;
            Object* obj = scene->GetObject(it->id);
            if (!obj) { it = m_itemLives.erase(it); continue; }
            if (it->timer >= LIFETIME) {
                scene->RemoveObject(it->id);
                it = m_itemLives.erase(it);
                continue;
            }
            if (it->timer >= BLINK_START) {
                obj->SetVisible(blinkVisible);
            }
            ++it;
        }
    }

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
    if (cam) { DrawBullets(cam); }
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
            SpawnExplosionAt(it->x, it->y);
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

void GSPlay::SpawnExplosionAt(float x, float y) {
    int idx = CreateOrAcquireExplosionObjectFromProto(m_explosionObjectId);
    Explosion e{}; e.x = x; e.y = y; e.objIdx = idx; e.cols = 11; e.rows = 1; e.frameIndex = 0; e.frameCount = 11; e.frameTimer = 0.0f; e.frameDuration = EXPLOSION_FRAME_DURATION;
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
                halfW = fabsf(sc.x) * 0.5f;
                halfH = fabsf(sc.y) * 0.5f;
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
            case 44: return {0.1275f, 0.0675f}; // Flamegun (17x9)
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

    // HUD Player 1 (ID 916)
    if (Object* hud1 = scene->GetObject(916)) {
        float u0, v0, u1, v1;
        m_player.GetCurrentFrameUV(u0, v0, u1, v1);
        // Flip horizontally if player is facing left so HUD mirrors in the same direction
        bool flip = m_player.IsFacingLeft();
        if (flip) {
            std::swap(u0, u1);
        }
        hud1->SetCustomUV(u0, v0, u1, v1);
        hud1->Draw(uiView, uiProj);

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
        float u0, v0, u1, v1;
        m_player2.GetCurrentFrameUV(u0, v0, u1, v1);
        bool flip2 = m_player2.IsFacingLeft();
        if (flip2) {
            std::swap(u0, u1);
        }
        hud2->SetCustomUV(u0, v0, u1, v1);
        hud2->Draw(uiView, uiProj);

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

void GSPlay::HandleKeyEvent(unsigned char key, bool bIsPressed) {
    if (m_inputManager) {
        m_inputManager->UpdateKeyState(key, bIsPressed);
    }
    
    if (key == 'M' || key == 'm') {
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

    if (bIsPressed && key == '5') {
        bool toBat = !m_player.IsBatDemon();
        m_player.SetGunMode(false);
        m_player.SetGrenadeMode(false);
        if (toBat) { m_player.SetWerewolfMode(false); }
        m_player.SetBatDemonMode(toBat);
        if (auto mv = m_player.GetMovement()) mv->SetInputLocked(false);
    }
    if (bIsPressed && (key == '?' || key == '/' || key == 0xBF)) {
        bool toBat2 = !m_player2.IsBatDemon();
        m_player2.SetGunMode(false);
        m_player2.SetGrenadeMode(false);
        if (toBat2) { m_player2.SetWerewolfMode(false); }
        m_player2.SetBatDemonMode(toBat2);
        if (auto mv2 = m_player2.GetMovement()) mv2->SetInputLocked(false);
    }

    if (bIsPressed && key == '4') {
        bool toWerewolf = !m_player.IsWerewolf();
        m_player.SetGunMode(false);
        m_player.SetGrenadeMode(false);
        if (toWerewolf) { m_player.SetBatDemonMode(false); }
        m_player.SetWerewolfMode(toWerewolf);
        if (auto mv = m_player.GetMovement()) mv->SetInputLocked(false);
    }
    if (bIsPressed && (key == '.' || key == 0xBE)) { 
        bool toWerewolf2 = !m_player2.IsWerewolf();
        m_player2.SetGunMode(false);
        m_player2.SetGrenadeMode(false);
        if (toWerewolf2) { m_player2.SetBatDemonMode(false); }
        m_player2.SetWerewolfMode(toWerewolf2);
        if (auto mv2 = m_player2.GetMovement()) mv2->SetInputLocked(false);
    }

    if (bIsPressed && key == '1') { 
        if (m_player.IsBatDemon()) {
            m_player.TriggerBatDemonSlash();
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
        if (m_player2.IsBatDemon()) {
            m_player2.TriggerBatDemonSlash();
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
        if (m_player.IsBatDemon()) {
            return;
        }
        if (m_player.IsWerewolf()) {
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
                    if (m_p1Bombs > 0) { SpawnBombFromCharacter(m_player, remain); m_p1Bombs -= 1; UpdateHudBombDigits(); }
                }
                m_p1GrenadePressTime = -1.0f;
                m_p1GrenadeExplodedInHand = false;
            }
        }
    }
    if (key == ',' || key == 0xBC) { 
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
                    if (m_p2Bombs > 0) { SpawnBombFromCharacter(m_player2, remain); m_p2Bombs -= 1; UpdateHudBombDigits(); }
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
    {
        float len = dir.Length();
        if (len > 1e-6f) {
            dir = dir / len;
        } else {
            dir = Vector3(faceSign, 0.0f, 0.0f);
        }
    }

    int slot = CreateOrAcquireBulletObject();
    bool isP1 = (&ch == &m_player);
    int gunTex = isP1 ? m_player1GunTexId : m_player2GunTexId;
    float speedMul = 1.0f;
    float damage = 10.0f;
    if (gunTex == 45) { speedMul = 1.5f; damage = 20.0f; }
    else if (gunTex == 46) { speedMul = 2.0f; damage = 50.0f; }
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
    if (gunTex == 45) { speedMul = 1.5f; damage = 20.0f; }
    else if (gunTex == 46) { speedMul = 2.0f; damage = 50.0f; }
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

void GSPlay::SpawnFlamegunBulletFromCharacter(const Character& ch, float jitterDeg) {
    // Reuse bazoka visual (bullet + trail), but slower and with gravity after a distance
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
    b.vx = dir.x * BULLET_SPEED * FLAMEGUN_SPEED_MUL; b.vy = dir.y * BULLET_SPEED * FLAMEGUN_SPEED_MUL;
    b.life = FLAMEGUN_LIFETIME; b.objIndex = slot; b.angleRad = angleWorld; b.faceSign = faceSign;
    b.ownerId = isP1 ? 1 : 2; b.damage = FLAMEGUN_DAMAGE; b.isBazoka = true; b.trailTimer = 0.0f;
    b.isFlamegun = true; b.distanceTraveled = 0.0f; b.dropAfterDistance = FLAMEGUN_DROP_DISTANCE; b.gravityAccel = FLAMEGUN_GRAVITY;
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

        if (it->isFlamegun) {
            it->distanceTraveled += std::sqrt(dx*dx + dy*dy);
            if (it->distanceTraveled >= it->dropAfterDistance) {
                it->vy -= it->gravityAccel * dt;
                it->angleRad = atan2f(it->vy, it->vx);
            }
        }

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
                if (it->isBazoka && !it->isFlamegun) {
                    SpawnExplosionAt(it->x, it->y);
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
                if (it->isBazoka && !it->isFlamegun) {
                    SpawnExplosionAt(it->x, it->y);
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
            b.vx = dir.x * BULLET_SPEED * 0.8f; b.vy = dir.y * BULLET_SPEED * 0.8f;
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
        } else if (currentGunTex == 44) { // FlameGun
            int& ammoFlame = isP1 ? m_p1Ammo44 : m_p2Ammo44;
            if (ammoFlame < 5) { pendingFlag = false; ch.SetGunMode(false); if (ch.GetMovement()) ch.GetMovement()->SetInputLocked(false); return; }
            const int count = FLAMEGUN_BULLET_COUNT;
            for (int i = 0; i < count; ++i) {
                float r1 = (float)rand() / (float)RAND_MAX;
                float r2 = (float)rand() / (float)RAND_MAX;
                float r = r1 - r2;
                float jitter = r * (FLAMEGUN_SPREAD_DEG * 0.6f);
                SpawnFlamegunBulletFromCharacter(ch, jitter);
            }
            ammoFlame -= 5; UpdateHudAmmoDigits(); StartHudAmmoAnimation(isP1); TryUnequipIfEmpty(44, isP1);
            ch.MarkGunShotFired();
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
            SpawnBulletFromCharacter(ch);
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
    
    int fanIds[] = {800, 801, 802, 803};
    const float FAN_ROTATION_SPEED = 90.0f;
    
    for (int fanId : fanIds) {
        Object* fan = sceneManager->GetObject(fanId);
        if (fan) {
            const Vector3& currentRotation = fan->GetRotation();
            float newZRotation = currentRotation.z + FAN_ROTATION_SPEED * deltaTime;
            
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
    Object* gun_flame   = scene->GetObject(1204);
    Object* gun_deagle  = scene->GetObject(1205);
    Object* gun_sniper  = scene->GetObject(1206);
    Object* gun_uzi     = scene->GetObject(1207);
    Object* bomb_pickup = scene->GetObject(1502);

    const bool* keys = m_inputManager->GetKeyStates();
    if (!keys) return;

    const PlayerInputConfig& cfg1 = m_player.GetMovement()->GetInputConfig();
    bool p1Sit = keys[cfg1.sitKey];
    bool p1PickupJust = m_inputManager->IsKeyJustPressed('1');

    const PlayerInputConfig& cfg2 = m_player2.GetMovement()->GetInputConfig();
    bool p2Sit = keys[cfg2.sitKey];
    bool p2PickupJust = m_inputManager->IsKeyJustPressed('N') || m_inputManager->IsKeyJustPressed('n');

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
            gunObj = nullptr;
            if (isPlayer1) m_player1GunTexId = texId; else m_player2GunTexId = texId;
            // Also switch top overlay animations to this gun
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
            bombObj = nullptr;
            if (isPlayer1) { m_p1Bombs = 3; } else { m_p2Bombs = 3; }
            UpdateHudBombDigits();
            std::cout << "Picked up bomb ID " << removedId << " -> refill to 3\n";
            SoundManager::Instance().PlaySFXByID(7, 0);
            return true;
        }
        return false;
    };

    if ( tryPickupBomb(bomb_pickup, m_player,  p1Sit, p1PickupJust, true) ) { return; }

    // Check Player 1 melee
    if ( tryPickup(m_player,  p1Sit, p1PickupJust, axe,   m_isAxeAvailable,   Character::WeaponType::Axe)   ||
         tryPickup(m_player,  p1Sit, p1PickupJust, sword, m_isSwordAvailable, Character::WeaponType::Sword) ||
         tryPickup(m_player,  p1Sit, p1PickupJust, pipe,  m_isPipeAvailable,  Character::WeaponType::Pipe) ) { return; }
    // Check Player 1 guns
    if ( tryPickupGun(40, gun_pistol,  m_player, p1Sit, p1PickupJust, true)  ||
         tryPickupGun(41, gun_m4a1,    m_player, p1Sit, p1PickupJust, true)  ||
         tryPickupGun(42, gun_shotgun, m_player, p1Sit, p1PickupJust, true)  ||
         tryPickupGun(43, gun_bazoka,  m_player, p1Sit, p1PickupJust, true)  ||
         tryPickupGun(44, gun_flame,   m_player, p1Sit, p1PickupJust, true)  ||
         tryPickupGun(45, gun_deagle,  m_player, p1Sit, p1PickupJust, true)  ||
         tryPickupGun(46, gun_sniper,  m_player, p1Sit, p1PickupJust, true)  ||
         tryPickupGun(47, gun_uzi,     m_player, p1Sit, p1PickupJust, true) ) { return; }

    // Try pick up bomb (Player 2)
    if ( tryPickupBomb(bomb_pickup, m_player2, p2Sit, p2PickupJust, false) ) { return; }

    // Check Player 2
    if ( tryPickup(m_player2, p2Sit, p2PickupJust, axe,   m_isAxeAvailable,   Character::WeaponType::Axe)   ||
         tryPickup(m_player2, p2Sit, p2PickupJust, sword, m_isSwordAvailable, Character::WeaponType::Sword) ||
         tryPickup(m_player2, p2Sit, p2PickupJust, pipe,  m_isPipeAvailable,  Character::WeaponType::Pipe) ) { return; }
    // Check Player 2 guns
    if ( tryPickupGun(40, gun_pistol,  m_player2, p2Sit, p2PickupJust, false) ||
         tryPickupGun(41, gun_m4a1,    m_player2, p2Sit, p2PickupJust, false) ||
         tryPickupGun(42, gun_shotgun, m_player2, p2Sit, p2PickupJust, false) ||
         tryPickupGun(43, gun_bazoka,  m_player2, p2Sit, p2PickupJust, false) ||
         tryPickupGun(44, gun_flame,   m_player2, p2Sit, p2PickupJust, false) ||
         tryPickupGun(45, gun_deagle,  m_player2, p2Sit, p2PickupJust, false) ||
         tryPickupGun(46, gun_sniper,  m_player2, p2Sit, p2PickupJust, false) ||
         tryPickupGun(47, gun_uzi,     m_player2, p2Sit, p2PickupJust, false) ) { return; }
}