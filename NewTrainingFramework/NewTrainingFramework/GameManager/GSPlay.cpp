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

bool GSPlay_IsShowPlatformBoxes() {
    return GSPlay::IsShowPlatformBoxes();
}

GSPlay::GSPlay() 
    : GameStateBase(StateType::PLAY), m_gameTime(0.0f), m_player1Health(100.0f), m_player2Health(100.0f) {
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
    
    // Debug: Check if Health texture is loaded
    ResourceManager* resourceManager = ResourceManager::GetInstance();
    std::shared_ptr<Texture2D> healthTexture = resourceManager->GetTexture(12);
    if (healthTexture) {
        std::cout << "SUCCESS: Health texture (ID 12) loaded successfully!" << std::endl;
        std::cout << "  - Size: " << healthTexture->GetWidth() << "x" << healthTexture->GetHeight() << std::endl;
        std::cout << "  - Channels: " << healthTexture->GetChannels() << std::endl;
        std::cout << "  - Filepath: " << healthTexture->GetFilepath() << std::endl;
    } else {
        std::cout << "ERROR: Health texture (ID 12) failed to load!" << std::endl;
        // Try to load it manually
        if (resourceManager->LoadTexture(12, "../Resources/Fighter/UI/Health.tga", "GL_CLAMP_TO_EDGE")) {
            std::cout << "SUCCESS: Manually loaded Health texture!" << std::endl;
        } else {
            std::cout << "ERROR: Failed to manually load Health texture!" << std::endl;
        }
    }
    
    SceneManager* sceneManager = SceneManager::GetInstance();
    if (!sceneManager->LoadSceneForState(StateType::PLAY)) {
        std::cout << "Failed to load play scene!" << std::endl;
        sceneManager->Clear();
        
        Camera* camera = sceneManager->CreateCamera();
        if (camera) {
            float aspect = (float)Globals::screenWidth / (float)Globals::screenHeight;
            camera->SetOrthographic(-aspect, aspect, -1.0f, 1.0f, 0.1f, 100.0f);
            camera->SetLookAt(Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
            
            // Enable auto zoom for dynamic camera
            camera->EnableAutoZoom(true);
            camera->SetZoomRange(0.4f, 1.8f); // Min zoom (far view) to max zoom (close view)
            camera->SetZoomSpeed(3.0f); // Smooth zoom speed
            
            sceneManager->SetActiveCamera(0);
        }
    } else {
        Camera* camera = sceneManager->GetActiveCamera();
        if (camera) {
            float aspect = (float)Globals::screenWidth / (float)Globals::screenHeight;
            camera->SetOrthographic(-aspect, aspect, -1.0f, 1.0f, 0.1f, 100.0f);
            
            // Enable auto zoom for dynamic camera
            camera->EnableAutoZoom(true);
            camera->SetZoomRange(0.4f, 1.8f); // Min zoom (far view) to max zoom (close view)
            camera->SetZoomSpeed(3.0f); // Smooth zoom speed
        }
    }    
    
    // Debug: Check if health bar objects are created
    Object* healthBar1 = sceneManager->GetObject(2000);
    Object* healthBar2 = sceneManager->GetObject(2001);
    if (healthBar1) {
        std::cout << "SUCCESS: Health bar 1 (ID 2000) created!" << std::endl;
        std::cout << "  - Position: (" << healthBar1->GetPosition().x << ", " << healthBar1->GetPosition().y << ", " << healthBar1->GetPosition().z << ")" << std::endl;
        std::cout << "  - Scale: (" << healthBar1->GetScale().x << ", " << healthBar1->GetScale().y << ", " << healthBar1->GetScale().z << ")" << std::endl;
        std::cout << "  - Texture IDs: ";
        for (int texId : healthBar1->GetTextureIds()) {
            std::cout << texId << " ";
        }
        std::cout << std::endl;
    } else {
        std::cout << "ERROR: Health bar 1 (ID 2000) not found!" << std::endl;
    }
    
    if (healthBar2) {
        std::cout << "SUCCESS: Health bar 2 (ID 2001) created!" << std::endl;
        std::cout << "  - Position: (" << healthBar2->GetPosition().x << ", " << healthBar2->GetPosition().y << ", " << healthBar2->GetPosition().z << ")" << std::endl;
        std::cout << "  - Scale: (" << healthBar2->GetScale().x << ", " << healthBar2->GetScale().y << ", " << healthBar2->GetScale().z << ")" << std::endl;
        std::cout << "  - Texture IDs: ";
        for (int texId : healthBar2->GetTextureIds()) {
            std::cout << texId << " ";
        }
        std::cout << std::endl;
    } else {
        std::cout << "ERROR: Health bar 2 (ID 2001) not found!" << std::endl;
    }
    
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
        std::cout << "Animation system initialized for Player 1 with " << animations.size() << " animations" << std::endl;
    } else {
        std::cout << "Warning: No animation data found for texture ID 10" << std::endl;
    }
    
    m_player.Initialize(m_animManager, 1000);
    m_player.SetInputConfig(CharacterMovement::PLAYER1_INPUT);
    m_player.ResetHealth(); // Khởi tạo health cho Player 1
    
    // Setup hurtbox for Player 1
    m_player.SetHurtbox(0.16f, 0.24f, -0.01f, -0.08f); // Width, Height, OffsetX, OffsetY
    
    auto animManager2 = std::make_shared<AnimationManager>();
    
    const TextureData* textureData2 = ResourceManager::GetInstance()->GetTextureData(11);
    if (textureData2 && textureData2->spriteWidth > 0 && textureData2->spriteHeight > 0) {
        std::vector<AnimationData> animations2;
        for (const auto& anim : textureData2->animations) {
            animations2.push_back({anim.startFrame, anim.numFrames, anim.duration, 0.0f});
        }
        animManager2->Initialize(textureData2->spriteWidth, textureData2->spriteHeight, animations2);
        std::cout << "Animation system initialized for Player 2 with " << animations2.size() << " animations" << std::endl;
    } else {
        std::cout << "Warning: No animation data found for texture ID 11" << std::endl;
    }
    
    m_player2.Initialize(animManager2, 1001);
    m_player2.SetInputConfig(CharacterMovement::PLAYER2_INPUT);
    m_player2.ResetHealth(); // Khởi tạo health cho Player 2
    
    // Setup hurtbox for Player 2
    m_player2.SetHurtbox(0.16f, 0.24f, -0.01f, -0.08f); // Width, Height, OffsetX, OffsetY
    
    // Tự động đọc tất cả platform từ file GSPlay.txt
    m_player.GetMovement()->ClearPlatforms();
    m_player2.GetMovement()->ClearPlatforms();
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
    
    // Set character size for collision detection
    m_player.GetMovement()->SetCharacterSize(0.16f, 0.24f);
    m_player2.GetMovement()->SetCharacterSize(0.16f, 0.24f);
    
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
    std::cout << "- J: Start/Continue Punch Combo" << std::endl;
    std::cout << "  * Press J once: Punch1 (Animation 10: Punch1)" << std::endl;
    std::cout << "  * Press J twice: Punch2 (Animation 11: Punch2)" << std::endl;
    std::cout << "  * Press J three times: Punch3 (Animation 12: Punch3)" << std::endl;
    std::cout << "  * Combo window: 0.5 seconds" << std::endl;
    std::cout << "- L: Start/Continue Axe Combo" << std::endl;
    std::cout << "  * Press L once: Axe1 (Animation 20: Axe1)" << std::endl;
    std::cout << "  * Press L twice: Axe2 (Animation 21: Axe2)" << std::endl;
    std::cout << "  * Press L three times: Axe3 (Animation 22: Axe3)" << std::endl;
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
    std::cout << "- 1: Start/Continue Punch Combo" << std::endl;
    std::cout << "  * Press 1 once: Punch1 (Animation 10: Punch1)" << std::endl;
    std::cout << "  * Press 1 twice: Punch2 (Animation 11: Punch2)" << std::endl;
    std::cout << "  * Press 1 three times: Punch3 (Animation 12: Punch3)" << std::endl;
    std::cout << "  * Combo window: 0.5 seconds" << std::endl;
    std::cout << "- 3: Start/Continue Axe Combo" << std::endl;
    std::cout << "  * Press 3 once: Axe1 (Animation 20: Axe1)" << std::endl;
    std::cout << "  * Press 3 twice: Axe2 (Animation 21: Axe2)" << std::endl;
    std::cout << "  * Press 3 three times: Axe3 (Animation 22: Axe3)" << std::endl;
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
}

void GSPlay::Update(float deltaTime) {
    m_gameTime += deltaTime;
    
    SceneManager::GetInstance()->Update(deltaTime);
    
    if (m_inputManager) {
        // Unified input processing - Character handles all input logic
        m_player.ProcessInput(deltaTime, m_inputManager);
        m_player2.ProcessInput(deltaTime, m_inputManager);
        
        // Update input manager (reset key press events)
        m_inputManager->Update();
    } else {
        std::cout << "Warning: m_inputManager is null in GSPlay::Update" << std::endl;
        // Initialize input manager if it's null
        m_inputManager = InputManager::GetInstance();
    }
    
    // Update characters
    m_player.Update(deltaTime);
    m_player2.Update(deltaTime);
    
    // Check for hitbox-hurtbox collisions
    if (m_player.CheckHitboxCollision(m_player2)) {
        m_player2.TriggerGetHit(m_player);
    }
    
    if (m_player2.CheckHitboxCollision(m_player)) {
        m_player.TriggerGetHit(m_player2);
    }
    
    // Update camera with auto zoom based on character positions
    Camera* camera = SceneManager::GetInstance()->GetActiveCamera();
    if (camera && camera->IsAutoZoomEnabled()) {
        Vector3 player1Pos = m_player.GetPosition();
        Vector3 player2Pos = m_player2.GetPosition();
        camera->UpdateCameraForCharacters(player1Pos, player2Pos, deltaTime);
    }
    
    // Sync health from characters to health bars
    m_player1Health = m_player.GetHealth();
    m_player2Health = m_player2.GetHealth();
    
    UpdateHealthBars();
    
    Object* menuButton = SceneManager::GetInstance()->GetObject(MENU_BUTTON_ID);
    if (menuButton) {
        menuButton->SetScale(Vector3(0.2f, 0.1f, 1.0f));
    }
}

void GSPlay::Draw() {
    SceneManager::GetInstance()->Draw();
    
    Camera* cam = SceneManager::GetInstance()->GetActiveCamera();
    if (cam) {
        m_player.Draw(cam);
        m_player2.Draw(cam);
    }
    
    static float lastPosX = m_player.GetPosition().x;
    static int lastAnim = m_player.GetCurrentAnimation();
    static float lastPosX2 = m_player2.GetPosition().x;
    static int lastAnim2 = m_player2.GetCurrentAnimation();
    static bool wasMoving = false;
    static bool wasMoving2 = false;
        
    const bool* keyStates = m_inputManager ? m_inputManager->GetKeyStates() : nullptr;
    if (!keyStates) {
        std::cout << "Warning: keyStates is null in GSPlay::Draw" << std::endl;
        return;
    }
    bool isMoving = keyStates ? (keyStates['A'] || keyStates['D']) : false;
    bool isMoving2 = keyStates ? (keyStates[0x25] || keyStates[0x27]) : false;
        
    if (abs(m_player.GetPosition().x - lastPosX) > 0.01f || 
        lastAnim != m_player.GetCurrentAnimation() ||
        (isMoving && !wasMoving)) {
            
        //std::cout << "=== PLAYER 1 STATUS ===" << std::endl;
        //std::cout << "Position: (" << m_player.GetPosition().x << ", " << m_player.GetPosition().y << ")" << std::endl;
        //std::cout << "State: " << (int)m_player.GetState() << std::endl;
        //std::cout << "Animation: " << m_player.GetCurrentAnimation() << std::endl;
        //std::cout << "Facing: " << (m_player.IsFacingLeft() ? "LEFT" : "RIGHT") << std::endl;
        //std::cout << "Movement: " << (isMoving ? "ACTIVE" : "IDLE") << std::endl;
        
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
        std::cout << "=========================" << std::endl;
            
        lastPosX = m_player.GetPosition().x;
        lastAnim = m_player.GetCurrentAnimation();
        wasMoving = isMoving;
    }
    
    if (abs(m_player2.GetPosition().x - lastPosX2) > 0.01f || 
        lastAnim2 != m_player2.GetCurrentAnimation() ||
        (isMoving2 && !wasMoving2)) {
            
        //std::cout << "=== PLAYER 2 STATUS ===" << std::endl;
        //std::cout << "Position: (" << m_player2.GetPosition().x << ", " << m_player2.GetPosition().y << ")" << std::endl;
        //std::cout << "State: " << (int)m_player2.GetState() << std::endl;
        //std::cout << "Animation: " << m_player2.GetCurrentAnimation() << std::endl;
        //std::cout << "Facing: " << (m_player2.IsFacingLeft() ? "LEFT" : "RIGHT") << std::endl;
        //std::cout << "Movement: " << (isMoving2 ? "ACTIVE" : "IDLE") << std::endl;
        
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
        std::cout << "=========================" << std::endl;
            
        lastPosX2 = m_player2.GetPosition().x;
        lastAnim2 = m_player2.GetCurrentAnimation();
        wasMoving2 = isMoving2;
    }
}

void GSPlay::HandleKeyEvent(unsigned char key, bool bIsPressed) {
    if (m_inputManager) {
        m_inputManager->UpdateKeyState(key, bIsPressed);
    }
    
    if (!bIsPressed) return; // Chỉ xử lý khi nhấn phím, không xử lý khi thả phím
    
    switch (key) {
        case 'C':
        case 'c':
            s_showHitboxHurtbox = !s_showHitboxHurtbox;
            std::cout << "Hitbox/Hurtbox display: " << (s_showHitboxHurtbox ? "ON" : "OFF") << std::endl;
            break;
            
        case 'Z':
        case 'z':
            {
                Camera* camera = SceneManager::GetInstance()->GetActiveCamera();
                if (camera) {
                    bool currentState = camera->IsAutoZoomEnabled();
                    camera->EnableAutoZoom(!currentState);
                    
                    if (!currentState) {
                        // Turning ON auto zoom
                        std::cout << "Camera auto-zoom: ON" << std::endl;
                    } else {
                        // Turning OFF auto zoom - reset to initial state
                        camera->ResetToInitialState();
                        std::cout << "Camera auto-zoom: OFF (reset to initial state)" << std::endl;
                    }
                }
            }
            break;
            
        case 'R':
        case 'r':
            // Reset health cho cả 2 player khi nhấn R
            m_player.ResetHealth();
            m_player2.ResetHealth();
            m_player1Health = m_player.GetHealth();
            m_player2Health = m_player2.GetHealth();
            UpdateHealthBars();
            std::cout << "=== HEALTH RESET ===" << std::endl;
            std::cout << "Player 1 Health: " << m_player.GetHealth() << "/" << m_player.GetMaxHealth() << std::endl;
            std::cout << "Player 2 Health: " << m_player2.GetHealth() << "/" << m_player2.GetMaxHealth() << std::endl;
            std::cout << "===================" << std::endl;
            break;
            
        case 'V':
        case 'v':
            s_showPlatformBoxes = !s_showPlatformBoxes;
            std::cout << "Platform boxes display: " << (s_showPlatformBoxes ? "ON" : "OFF") << std::endl;
            break;
    }
}

static float MousePixelToWorldX(int x, Camera* cam) {
    float left = cam->GetLeft();
    float right = cam->GetRight();
    return left + (right - left) * ((float)x / Globals::screenWidth);
}
static float MousePixelToWorldY(int y, Camera* cam) {
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
    std::cout << "GSPlay: Resume (Back to Game)" << std::endl;
    std::cout << "Welcome back to the game!" << std::endl;
}

void GSPlay::Pause() {
    std::cout << "GSPlay: Pause (Game Paused)" << std::endl;
    std::cout << "Game is now paused..." << std::endl;
}

void GSPlay::Exit() {
    std::cout << "GSPlay: Exit" << std::endl;
    std::cout << "Leaving gameplay mode..." << std::endl;
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
        Vector3 player1Pos = m_player.GetPosition();
        
        Object* player1Obj = sceneManager->GetObject(1000);
        float characterHeight = 0.24f;
        if (player1Obj) {
            characterHeight = player1Obj->GetScale().y;
        }
        //chỉnh độ cao
        float healthBarOffsetY = characterHeight / 6.0f;
        //chỉnh center
        float healthBarOffsetX = -0.13f;
        healthBar1->SetPosition(player1Pos.x + healthBarOffsetX, player1Pos.y + healthBarOffsetY, 0.0f);
        
        float healthRatio1 = m_player.GetHealth() / m_player.GetMaxHealth();
        const Vector3& scaleRef = healthBar1->GetScale();
        Vector3 currentScale(scaleRef.x, scaleRef.y, scaleRef.z);
        //chỉnh độ dài ngắn
        healthBar1->SetScale(healthRatio1 * 0.5f, currentScale.y, currentScale.z);
    }
    
    Object* healthBar2 = sceneManager->GetObject(2001);
    if (healthBar2) {
        Vector3 player2Pos = m_player2.GetPosition();
        
        Object* player2Obj = sceneManager->GetObject(1001);
        float characterHeight = 0.24f;
        if (player2Obj) {
            characterHeight = player2Obj->GetScale().y;
        }
        
        float healthBarOffsetY = characterHeight / 6.0f;
        float healthBarOffsetX = -0.13f;
        healthBar2->SetPosition(player2Pos.x + healthBarOffsetX, player2Pos.y + healthBarOffsetY, 0.0f);
        
        float healthRatio2 = m_player2.GetHealth() / m_player2.GetMaxHealth();
        const Vector3& scaleRef = healthBar2->GetScale();
        Vector3 currentScale(scaleRef.x, scaleRef.y, scaleRef.z);
        healthBar2->SetScale(healthRatio2 * 0.5f, currentScale.y, currentScale.z);
    }
} 