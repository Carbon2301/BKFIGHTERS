#include "stdafx.h"
#include "GSPlay.h"
#include "GameStateMachine.h"
#include "../Core/Globals.h"
#include <iostream>
#include "../GameObject/Object.h"
#include "../GameObject/Texture2D.h"
#include <memory>
#include "../GameObject/AnimationManager.h"
#include "ResourceManager.h"

// Thêm enum trạng thái nhân vật
enum class CharState { Idle, MoveLeft, MoveRight, MoveBack, MoveFront };
static CharState m_charState = CharState::Idle;
static int m_facingRow = 0; // 0: trước, 1: trái, 2: phải, 3: sau

// Thêm mảng lưu trạng thái phím
static bool keyStates[256] = { false };

static float m_charPosX = 0.0f;
static float m_charPosY = 0.0f;

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
    m_charPosX = 0.0f; // Vị trí giữa màn hình 
    m_charPosY = 0.0f;

    // Khởi tạo AnimationManager với dữ liệu từ ResourceManager
    m_animManager = std::make_shared<AnimationManager>();
    
    // Lấy animation data từ texture ID 10 (spritesheet)
    const TextureData* textureData = ResourceManager::GetInstance()->GetTextureData(10);
    if (textureData && textureData->spriteWidth > 0 && textureData->spriteHeight > 0) {
        std::vector<AnimationData> animations;
        for (const auto& anim : textureData->animations) {
            animations.push_back({anim.startFrame, anim.numFrames, anim.duration, 0.0f});
        }
        m_animManager->Initialize(textureData->spriteWidth, textureData->spriteHeight, animations);
        m_animManager->Play(0, true); // Play first animation (idle)
    } else {
        std::cout << "Warning: No animation data found for texture ID 10" << std::endl;
    }
    
    m_charState = CharState::Idle;
    m_facingRow = 0;
    
    std::cout << "Gameplay initialized" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "- ESC or M: Return to menu" << std::endl;
    std::cout << "- P: Quick play (from menu)" << std::endl;
    std::cout << "- Q: Help/Question (from menu)" << std::endl;
    std::cout << "- X: Exit game (from menu)" << std::endl;
    std::cout << "=== ANIMATION TEST MODE ===" << std::endl;
    std::cout << "- 0: Animation 0 (Idle)" << std::endl;
    std::cout << "- 1: Animation 1" << std::endl;
    std::cout << "- 2: Animation 2" << std::endl;
    std::cout << "- 3: Animation 3" << std::endl;
    std::cout << "- 4: Animation 4" << std::endl;
    std::cout << "- 5: Animation 5" << std::endl;
    std::cout << "- 6: Animation 6" << std::endl;
    std::cout << "- 7: Animation 7" << std::endl;
    std::cout << "- 8: Animation 8" << std::endl;
    std::cout << "- 9: Animation 9" << std::endl;
    std::cout << "- Q: Animation 10" << std::endl;
    std::cout << "- E: Animation 11" << std::endl;
    std::cout << "- R: Animation 12" << std::endl;
    
    Camera* cam = SceneManager::GetInstance()->GetActiveCamera();
    std::cout << "Camera actual: left=" << cam->GetLeft() << ", right=" << cam->GetRight()
              << ", bottom=" << cam->GetBottom() << ", top=" << cam->GetTop() << std::endl;
}

void GSPlay::Update(float deltaTime) {
    m_gameTime += deltaTime;
    float moveSpeed = 0.5f; // pixel/giây
    
    // Update scene
    SceneManager::GetInstance()->Update(deltaTime);
    
    // Tạm thời bỏ qua di chuyển, chỉ test animation
    // Animation sẽ được control bằng phím 1-9, 0
    m_charState = CharState::Idle;
    
    // Update animation
    if (m_animManager) {
        m_animManager->Update(deltaTime);
    }
    
    Object* menuButton = SceneManager::GetInstance()->GetObject(MENU_BUTTON_ID);
    if (menuButton) {
        menuButton->SetScale(Vector3(0.2f, 0.1f, 1.0f));
    }
    
    static float lastTimeShow = 0.0f;
    if (m_gameTime - lastTimeShow > 5.0f) {
        // std::cout << "Game time: " << (int)m_gameTime << " seconds" << std::endl;
        lastTimeShow = m_gameTime;
    }
}

void GSPlay::Draw() {
    SceneManager::GetInstance()->Draw();
    // Vẽ animation object qua SceneManager
    Object* animObj = SceneManager::GetInstance()->GetObject(ANIM_OBJECT_ID);
    if (animObj && m_animManager && animObj->GetModelId() >= 0 && animObj->GetModelPtr()) {
        float u0, v0, u1, v1;
        m_animManager->GetUV(u0, v0, u1, v1);
        animObj->SetCustomUV(u0, v0, u1, v1);
        // Truyền vị trí mới cho object nhân vật
        animObj->SetPosition(Vector3(m_charPosX, m_charPosY, 0.0f));
        Camera* cam = SceneManager::GetInstance()->GetActiveCamera();
        if (cam) animObj->Draw(cam->GetViewMatrix(), cam->GetProjectionMatrix());
    }

}

void GSPlay::HandleKeyEvent(unsigned char key, bool bIsPressed) {
    keyStates[key] = bIsPressed;
    
    if (bIsPressed) {
        // Test animation keys
        if (key >= '0' && key <= '9') {
            int animIndex = key - '0';
            if (m_animManager && animIndex < m_animManager->GetAnimationCount()) {
                m_animManager->Play(animIndex, true);
                std::cout << "Playing animation " << animIndex << std::endl;
            } else {
                std::cout << "Animation " << animIndex << " not available" << std::endl;
            }
        } else if (key == 'Q' || key == 'q') {
            if (m_animManager && 10 < m_animManager->GetAnimationCount()) {
                m_animManager->Play(10, true);
                std::cout << "Playing animation 10" << std::endl;
            }
        } else if (key == 'E' || key == 'e') {
            if (m_animManager && 11 < m_animManager->GetAnimationCount()) {
                m_animManager->Play(11, true);
                std::cout << "Playing animation 11" << std::endl;
            }
        } else if (key == 'R' || key == 'r') {
            if (m_animManager && 12 < m_animManager->GetAnimationCount()) {
                m_animManager->Play(12, true);
                std::cout << "Playing animation 12" << std::endl;
            }
        }
    }
    
    if (!bIsPressed) {
        if (key == 27 || key == 'M' || key == 'm') {
            std::cout << "=== Returning to Menu ===" << std::endl;
            std::cout << "Game paused. Returning to main menu..." << std::endl;
            std::cout << "Calling ChangeState(MENU)..." << std::endl;
            GameStateMachine::GetInstance()->ChangeState(StateType::MENU); // Direct change to menu
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
} 