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
#include "../GameObject/CharacterMovement.h"
#include "../GameObject/InputManager.h"
#include "ResourceManager.h"
#include <fstream>
#include <sstream>

#define MENU_BUTTON_ID 301

static Character m_player;
static Character m_player2;
static InputManager* m_inputManager = nullptr;

bool GSPlay::s_showHitboxHurtbox = false;
bool GSPlay::s_showPlatformBoxes = true;
bool GSPlay::s_showWallBoxes = true;
bool GSPlay::s_showLadderBoxes = true;
bool GSPlay::s_showTeleportBoxes = true;

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
    
    m_player.SetHurtbox(0.088f, 0.13f, -0.0088f, -0.038f);
    
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
    
    // Setup hurtbox for Player 2
    m_player2.SetHurtbox(0.088f, 0.13f, -0.0088f, -0.038f); // Width, Height, OffsetX, OffsetY
    
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

    // Initialize lifetime tracking for item objects
    m_itemLives.clear();
    auto tryAdd = [&](int id){ if (sceneManager->GetObject(id)) m_itemLives.push_back({id, 0.0f}); };
    tryAdd(AXE_OBJECT_ID);
    tryAdd(SWORD_OBJECT_ID);
    tryAdd(PIPE_OBJECT_ID);

    // Initialize HUD weapons: cache base scales and hide by default
    if (Object* hudWeapon1 = sceneManager->GetObject(918)) {
        m_hudWeapon1BaseScale = hudWeapon1->GetScale();
        hudWeapon1->SetScale(0.0f, 0.0f, m_hudWeapon1BaseScale.z);
    }
    if (Object* hudWeapon2 = sceneManager->GetObject(919)) {
        m_hudWeapon2BaseScale = hudWeapon2->GetScale();
        hudWeapon2->SetScale(0.0f, 0.0f, m_hudWeapon2BaseScale.z);
    }

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
    UpdateHudWeapons();
}

void GSPlay::Update(float deltaTime) {
    m_gameTime += deltaTime;
    
    SceneManager::GetInstance()->Update(deltaTime);
    
    if (m_inputManager) {
        m_player.ProcessInput(deltaTime, m_inputManager);
        m_player2.ProcessInput(deltaTime, m_inputManager);
        HandleItemPickup();
        m_inputManager->Update();
    } else {
        m_inputManager = InputManager::GetInstance();
    }
    
    m_player.Update(deltaTime);
    m_player2.Update(deltaTime);
    UpdateBullets(deltaTime);
    TryCompletePendingShots();
    UpdateHudWeapons();
    
    if (m_player.CheckHitboxCollision(m_player2)) {
        m_player2.TriggerGetHit(m_player);
    }
    
    if (m_player2.CheckHitboxCollision(m_player)) {
        m_player.TriggerGetHit(m_player2);
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

    // Update item lifetimes (20s total; blink after 12s)
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
        if (m_player.IsFacingLeft()) {
            std::swap(u0, u1);
        }
        hud1->SetCustomUV(u0, v0, u1, v1);
        hud1->Draw(uiView, uiProj);
    }

    // HUD Player 2 (ID 917)
    if (Object* hud2 = scene->GetObject(917)) {
        float u0, v0, u1, v1;
        m_player2.GetCurrentFrameUV(u0, v0, u1, v1);
        if (m_player2.IsFacingLeft()) {
            std::swap(u0, u1);
        }
        hud2->SetCustomUV(u0, v0, u1, v1);
        hud2->Draw(uiView, uiProj);
    }
}

void GSPlay::HandleKeyEvent(unsigned char key, bool bIsPressed) {
    if (m_inputManager) {
        m_inputManager->UpdateKeyState(key, bIsPressed);
    }
    
    if (key == 'M' || key == 'm') {
        bool was = m_player2.IsGunMode();
        if (bIsPressed) {
            if (!m_player2.IsJumping()) {
                m_player2.SetGunMode(true);
                m_player2.GetMovement()->SetInputLocked(true);
                if (!was) { m_p2ShotPending = false; m_p2GunStartTime = m_gameTime; }
            }
        } else {
            if (was) { m_p2ShotPending = true; }
        }
    }
    if (key == '2') {
        bool was = m_player.IsGunMode();
        if (bIsPressed) {
            if (!m_player.IsJumping()) {
                m_player.SetGunMode(true);
                m_player.GetMovement()->SetInputLocked(true);
                if (!was) { m_p1ShotPending = false; m_p1GunStartTime = m_gameTime; }
            }
        } else {
            if (was) { m_p1ShotPending = true; }
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
    Vector3 spawn = ch.GetPosition();
    spawn.x += ch.IsFacingLeft() ? -BULLET_SPAWN_OFFSET_X : BULLET_SPAWN_OFFSET_X;
    spawn.y += BULLET_SPAWN_OFFSET_Y;
    bool left = ch.IsFacingLeft();
    Vector3 dir = left ? Vector3(-1.0f, 0.0f, 0.0f) : Vector3(1.0f, 0.0f, 0.0f);
    int slot = CreateOrAcquireBulletObject();
    Bullet b; b.x = spawn.x; b.y = spawn.y; b.vx = dir.x * BULLET_SPEED; b.vy = dir.y * BULLET_SPEED; b.life = BULLET_LIFETIME; b.objIndex = slot;
    m_bullets.push_back(b);
}

void GSPlay::UpdateBullets(float dt) {
    // Update all bullets
    for (auto it = m_bullets.begin(); it != m_bullets.end(); ) {
        it->life -= dt;
        it->x += it->vx * dt;
        it->y += it->vy * dt;
        if (it->life <= 0.0f) {
            // release object back to pool
            if (it->objIndex >= 0 && it->objIndex < (int)m_bulletObjs.size() && m_bulletObjs[it->objIndex]) {
                m_freeBulletSlots.push_back(it->objIndex);
                m_bulletObjs[it->objIndex]->SetVisible(false);
            }
            it = m_bullets.erase(it);
        } else {
            ++it;
        }
    }
}

int GSPlay::CreateOrAcquireBulletObject() {
    // reuse slot if available
    if (!m_freeBulletSlots.empty()) {
        int idx = m_freeBulletSlots.back();
        m_freeBulletSlots.pop_back();
        if (m_bulletObjs[idx]) {
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

void GSPlay::DrawBullets(Camera* cam) {
    for (const Bullet& b : m_bullets) {
        int idx = b.objIndex;
        if (idx >= 0 && idx < (int)m_bulletObjs.size() && m_bulletObjs[idx]) {
            m_bulletObjs[idx]->SetPosition(b.x, b.y, 0.0f);
            m_bulletObjs[idx]->Draw(cam->GetViewMatrix(), cam->GetProjectionMatrix());
        }
    }
}

void GSPlay::TryCompletePendingShots() {
    auto tryFinish = [&](Character& ch, bool& pendingFlag, float& startTime){
        if (!pendingFlag) return;
        // Require minimum time for anim0 + anim1 display
        float elapsed = m_gameTime - startTime;
        if (elapsed < GetGunRequiredTime()) return;
        // Fire then exit gun mode
        SpawnBulletFromCharacter(ch);
        pendingFlag = false;
        ch.SetGunMode(false);
        ch.GetMovement()->SetInputLocked(false);
    };
    tryFinish(m_player,  m_p1ShotPending, m_p1GunStartTime);
    tryFinish(m_player2, m_p2ShotPending, m_p2GunStartTime);
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

// Item pickup: sit + kick
void GSPlay::HandleItemPickup() {
    if (!m_inputManager) return;
    SceneManager* scene = SceneManager::GetInstance();
    Object* axe   = scene->GetObject(AXE_OBJECT_ID);
    Object* sword = scene->GetObject(SWORD_OBJECT_ID);
    Object* pipe  = scene->GetObject(PIPE_OBJECT_ID);

    const bool* keys = m_inputManager->GetKeyStates();
    if (!keys) return;

    // Player 1 input gates: sit + kick
    const PlayerInputConfig& cfg1 = m_player.GetMovement()->GetInputConfig();
    bool p1Sit = keys[cfg1.sitKey];
    bool p1KickJust = m_inputManager->IsKeyJustPressed(cfg1.kickKey);

    // Player 2 input gates: sit + kick
    const PlayerInputConfig& cfg2 = m_player2.GetMovement()->GetInputConfig();
    bool p2Sit = keys[cfg2.sitKey];
    bool p2KickJust = m_inputManager->IsKeyJustPressed(cfg2.kickKey);

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

    auto tryPickup = [&](Character& player, bool sitHeld, bool kickJust, Object*& objRef, bool& availFlag, Character::WeaponType weaponType){
        if (!sitHeld || !kickJust || !objRef) return false;
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
            player.SetWeapon(weaponType);
            std::cout << "Picked up weapon ID " << removedId << " (type=" << (int)weaponType << ")" << std::endl;
            return true;
        }
        return false;
    };

    // Check Player 1
    if ( tryPickup(m_player,  p1Sit, p1KickJust, axe,   m_isAxeAvailable,   Character::WeaponType::Axe)   ||
         tryPickup(m_player,  p1Sit, p1KickJust, sword, m_isSwordAvailable, Character::WeaponType::Sword) ||
         tryPickup(m_player,  p1Sit, p1KickJust, pipe,  m_isPipeAvailable,  Character::WeaponType::Pipe) ) { return; }

    // Check Player 2
    if ( tryPickup(m_player2, p2Sit, p2KickJust, axe,   m_isAxeAvailable,   Character::WeaponType::Axe)   ||
         tryPickup(m_player2, p2Sit, p2KickJust, sword, m_isSwordAvailable, Character::WeaponType::Sword) ||
         tryPickup(m_player2, p2Sit, p2KickJust, pipe,  m_isPipeAvailable,  Character::WeaponType::Pipe) ) { return; }
}