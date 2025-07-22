#include "stdafx.h"
#include "GSPlay.h"
#include "GameStateMachine.h"
#include "../Core/Globals.h"
#include <iostream>
#include "../GameObject/Object.h"
#include "../GameObject/Texture2D.h"
#include <memory>

// Thêm include cho Animation2D
#include "../GameObject/Animation2D.h"

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
        
        // Create default gameplay background
        Object* backgroundObj = sceneManager->CreateObject(300);
        if (backgroundObj) {
            backgroundObj->SetModel(0);
            backgroundObj->SetTexture(1, 0); // bg_play1.tga
            backgroundObj->SetShader(0);
            backgroundObj->SetPosition(Vector3(0.0f, 0.0f, -0.1f));
            backgroundObj->SetScale(Vector3(2.5f, 2.5f, 1.0f));
        }
        
        // Create menu button
        Object* menuButton = sceneManager->CreateObject(MENU_BUTTON_ID);
        if (menuButton) {
            menuButton->SetModel(0);
            menuButton->SetTexture(4, 0); // btn_close.tga
            menuButton->SetShader(0);
            menuButton->SetPosition(Vector3(1.2f, 0.8f, 0.0f));
            menuButton->SetScale(Vector3(0.2f, 0.1f, 1.0f));
        }
    }
    
    m_gameTime = 0.0f;

    // Tạo object animation nhân vật qua SceneManager
    Object* animObj = SceneManager::GetInstance()->CreateObject(ANIM_OBJECT_ID);
    animObj->SetModel(0); // Sprite2D
    animObj->SetTexture(10); // spritesheet.tga (ID=10 trong RM.txt)
    animObj->SetShader(0);
    animObj->Set2DPosition(0.0f, 0.0f);
    animObj->SetSize(0.3f, 0.45f); // tuỳ chỉnh

    m_anim = std::make_shared<Animation2D>(12, 4, 0.1f); // 12 cột, 4 hàng, 0.1s/frame
    m_anim->SetRow(0); // Hàng đầu tiên (tuỳ chỉnh theo nhân vật)
    
    std::cout << "Gameplay initialized" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "- ESC or M: Return to menu" << std::endl;
    std::cout << "- P: Quick play (from menu)" << std::endl;
    std::cout << "- Q: Help/Question (from menu)" << std::endl;
    std::cout << "- X: Exit game (from menu)" << std::endl;
}

void GSPlay::Update(float deltaTime) {
    m_gameTime += deltaTime;
    
    // Update scene
    SceneManager::GetInstance()->Update(deltaTime);
    
    // Update animation
    if (m_anim) m_anim->Update(deltaTime);
    
    Object* menuButton = SceneManager::GetInstance()->GetObject(MENU_BUTTON_ID);
    if (menuButton) {
        menuButton->SetScale(Vector3(0.2f, 0.1f, 1.0f));
    }
    
    static float lastTimeShow = 0.0f;
    if (m_gameTime - lastTimeShow > 5.0f) {
        std::cout << "Game time: " << (int)m_gameTime << " seconds" << std::endl;
        lastTimeShow = m_gameTime;
    }
}

void GSPlay::Draw() {
    SceneManager::GetInstance()->Draw();
    // Vẽ animation object qua SceneManager
    Object* animObj = SceneManager::GetInstance()->GetObject(ANIM_OBJECT_ID);
    if (animObj && m_anim && animObj->GetModelId() >= 0 && animObj->GetModelPtr()) {
        float u0, v0, u1, v1;
        m_anim->GetUV(u0, v0, u1, v1);
        animObj->SetCustomUV(u0, v0, u1, v1);
        Camera* cam = SceneManager::GetInstance()->GetActiveCamera();
        if (cam) animObj->Draw(cam->GetViewMatrix(), cam->GetProjectionMatrix());
    }
}

void GSPlay::HandleKeyEvent(unsigned char key, bool bIsPressed) {
    std::cout << "GSPlay: Key " << (int)key << " pressed=" << bIsPressed << std::endl;
    
    if (!bIsPressed) return;
    
    switch (key) {
        case 27: // ESC
        case 'M':
        case 'm':
            std::cout << "=== Returning to Menu ===" << std::endl;
            std::cout << "Game paused. Returning to main menu..." << std::endl;
            std::cout << "Calling ChangeState(MENU)..." << std::endl;
            GameStateMachine::GetInstance()->ChangeState(StateType::MENU); // Direct change to menu
            std::cout << "ChangeState() called successfully!" << std::endl;
            break;
            
        case 'W':
        case 'w':
            std::cout << "Move up (demo)" << std::endl;
            break;
        case 'S':
        case 's':
            std::cout << "Move down (demo)" << std::endl;
            break;
        case 'A':
        case 'a':
            std::cout << "Move left (demo)" << std::endl;
            break;
        case 'D':
        case 'd':
            std::cout << "Move right (demo)" << std::endl;
            break;
            
        case ' ': // Space
            std::cout << "Action button pressed! (demo)" << std::endl;
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
    return top - (top - bottom) * ((float)y / Globals::screenHeight);
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
    // std::cout << "GSPlay: Mouse moved to (" << x << ", " << y << ")" << std::endl;
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
    m_anim = nullptr;
} 