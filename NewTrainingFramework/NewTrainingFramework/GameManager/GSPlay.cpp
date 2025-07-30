#include "stdafx.h"
#include "GSPlay.h"
#include "GameStateMachine.h"
#include "../Core/Globals.h"
#include <iostream>
#include "../GameObject/Object.h"
#include "../GameObject/Texture2D.h"
#include <memory>
#include "../GameObject/AnimationManager.h"
#include "../GameObject/Character.h"
#include "../GameObject/InputManager.h"
#include "ResourceManager.h"

// Character and Input Manager
static Character m_player;
static Character m_player2;
static InputManager* m_inputManager = nullptr;

GSPlay::GSPlay() 
    : GameStateBase(StateType::PLAY), m_gameTime(0.0f) {
}

GSPlay::~GSPlay() {
}

void GSPlay::Init() {
    std::cout << "=== GAMEPLAY MODE ===" << std::endl;
    std::cout << "Game started! Press ESC or M to return to menu" << std::endl;
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    SceneManager* sceneManager = SceneManager::GetInstance();
    if (!sceneManager->LoadSceneForState(StateType::PLAY)) {
        std::cout << "Failed to load play scene!" << std::endl;
        sceneManager->Clear();
        
        // Create default 2D camera
        Camera* camera = sceneManager->CreateCamera();
        if (camera) {
            float aspect = (float)Globals::screenWidth / (float)Globals::screenHeight;
            camera->SetOrthographic(-aspect, aspect, -1.0f, 1.0f, 0.1f, 100.0f);
            camera->SetLookAt(Vector3(0.0f, 0.0f, 1.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
            sceneManager->SetActiveCamera(0);
        }
    } else {
        // Nếu đã có camera, vẫn chỉnh lại tỉ lệ aspect cho đúng
        Camera* camera = sceneManager->GetActiveCamera();
        if (camera) {
            float aspect = (float)Globals::screenWidth / (float)Globals::screenHeight;
            camera->SetOrthographic(-aspect, aspect, -1.0f, 1.0f, 0.1f, 100.0f);
        }
    }    
    m_gameTime = 0.0f;

    // Khởi tạo InputManager
    m_inputManager = InputManager::GetInstance();

    // Khởi tạo AnimationManager với dữ liệu từ ResourceManager cho Player 1
    m_animManager = std::make_shared<AnimationManager>();
    
    // Lấy animation data từ texture ID 10 (Player1_Body.tga)
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
    
    // Khởi tạo Character Player 1
    m_player.Initialize(m_animManager, 1000);
    
    // Khởi tạo AnimationManager cho Player 2
    auto animManager2 = std::make_shared<AnimationManager>();
    
    // Lấy animation data từ texture ID 11 (Player2_Body.tga)
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
    
    // Khởi tạo Character Player 2
    m_player2.Initialize(animManager2, 1001);
    // Vị trí sẽ được đọc từ file scene (POS -1.0 0.0 0.0)
    
    std::cout << "Gameplay initialized" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "- ESC or M: Return to menu" << std::endl;
    std::cout << "- P: Quick play (from menu)" << std::endl;
    std::cout << "- Q: Help/Question (from menu)" << std::endl;
    std::cout << "- X: Exit game (from menu)" << std::endl;
    std::cout << "=== MOVEMENT CONTROLS ===" << std::endl;
    std::cout << "- A: Walk left (Animation 1: Walk)" << std::endl;
    std::cout << "- D: Walk right (Animation 1: Walk)" << std::endl;
    std::cout << "- Shift + A: Run left (Animation 2: Run)" << std::endl;
    std::cout << "- Shift + D: Run right (Animation 2: Run)" << std::endl;
    std::cout << "- S: Sit down (Animation 3: Sit)" << std::endl;
    std::cout << "- W: Jump (Animation 15: Jump)" << std::endl;
    std::cout << "- A + Space: Roll left (Animation 4: Roll)" << std::endl;
    std::cout << "- D + Space: Roll right (Animation 4: Roll)" << std::endl;
    std::cout << "- Space: Roll in current direction (Animation 4: Roll)" << std::endl;
    std::cout << "- Release keys: Idle (Animation 0: Idle)" << std::endl;
    std::cout << "=== COMBO SYSTEM ===" << std::endl;
    std::cout << "- J: Start/Continue Punch Combo" << std::endl;
    std::cout << "  * Press J once: Punch1 (Animation 9: Punch1)" << std::endl;
    std::cout << "  * Press J twice: Punch2 (Animation 10: Punch2)" << std::endl;
    std::cout << "  * Press J three times: Punch3 (Animation 11: Punch3)" << std::endl;
    std::cout << "  * Combo window: 0.5 seconds" << std::endl;
    std::cout << "- L: Start/Continue Axe Combo" << std::endl;
    std::cout << "  * Press L once: Axe1 (Animation 19: Axe1)" << std::endl;
    std::cout << "  * Press L twice: Axe2 (Animation 20: Axe2)" << std::endl;
    std::cout << "  * Press L three times: Axe3 (Animation 21: Axe3)" << std::endl;
    std::cout << "  * Combo window: 0.5 seconds" << std::endl;
    std::cout << "- K: Kick (Animation 18: Kick)" << std::endl;

    
    Camera* cam = SceneManager::GetInstance()->GetActiveCamera();
    std::cout << "Camera actual: left=" << cam->GetLeft() << ", right=" << cam->GetRight()
              << ", bottom=" << cam->GetBottom() << ", top=" << cam->GetTop() << std::endl;
}

void GSPlay::Update(float deltaTime) {
    m_gameTime += deltaTime;
    
    // Update scene
    SceneManager::GetInstance()->Update(deltaTime);
    
    // Handle input and update character
    if (m_inputManager) {
        const bool* keyStates = m_inputManager->GetKeyStates();
        
        // Handle movement and jump
        m_player.HandleMovement(deltaTime, keyStates);
        m_player.HandleJump(deltaTime, keyStates);
        
        // Handle combat input
        m_inputManager->HandleInput(m_player);
        
        // Update input manager (reset key press events)
        m_inputManager->Update();
    }
    
    // Update characters
    m_player.Update(deltaTime);
    m_player2.Update(deltaTime);
    
    Object* menuButton = SceneManager::GetInstance()->GetObject(MENU_BUTTON_ID);
    if (menuButton) {
        menuButton->SetScale(Vector3(0.2f, 0.1f, 1.0f));
    }
    
    static float lastTimeShow = 0.0f;
    if (m_gameTime - lastTimeShow > 5.0f) {
        lastTimeShow = m_gameTime;
    }
}

void GSPlay::Draw() {
    SceneManager::GetInstance()->Draw();
    
    // Draw characters
    Camera* cam = SceneManager::GetInstance()->GetActiveCamera();
    if (cam) {
        m_player.Draw(cam);
        m_player2.Draw(cam);
    }
    
    // Debug info
    static float lastPosX = m_player.GetPosition().x;
    static int lastAnim = m_player.GetCurrentAnimation();
        static bool wasMoving = false;
        
    const bool* keyStates = m_inputManager ? m_inputManager->GetKeyStates() : nullptr;
    bool isMoving = keyStates ? (keyStates['A'] || keyStates['a'] || keyStates['D'] || keyStates['d']) : false;
        
    if (abs(m_player.GetPosition().x - lastPosX) > 0.01f || 
        lastAnim != m_player.GetCurrentAnimation() ||
            (isMoving && !wasMoving)) {
            
            std::cout << "=== CHARACTER STATUS ===" << std::endl;
        std::cout << "Position: (" << m_player.GetPosition().x << ", " << m_player.GetPosition().y << ")" << std::endl;
        std::cout << "State: " << (int)m_player.GetState() << std::endl;
        std::cout << "Animation: " << m_player.GetCurrentAnimation() << std::endl;
        std::cout << "Facing: " << (m_player.IsFacingLeft() ? "LEFT" : "RIGHT") << std::endl;
            std::cout << "Movement: " << (isMoving ? "ACTIVE" : "IDLE") << std::endl;
        
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
        } else if (m_player.GetCurrentAnimation() == 18) {
            std::cout << "Action: KICK [Animation 18]" << std::endl;
            }
            std::cout << "=========================" << std::endl;
            
        lastPosX = m_player.GetPosition().x;
        lastAnim = m_player.GetCurrentAnimation();
            wasMoving = isMoving;
    }
}

void GSPlay::HandleKeyEvent(unsigned char key, bool bIsPressed) {
    if (m_inputManager) {
        m_inputManager->UpdateKeyState(key, bIsPressed);
    }
    
    if (!bIsPressed) {
        if (key == 27 || key == 'M' || key == 'm') {
            std::cout << "=== Returning to Menu ===" << std::endl;
            std::cout << "Game paused. Returning to main menu..." << std::endl;
            std::cout << "Calling ChangeState(MENU)..." << std::endl;
            GameStateMachine::GetInstance()->ChangeState(StateType::MENU);
            std::cout << "ChangeState() called successfully!" << std::endl;
        }
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