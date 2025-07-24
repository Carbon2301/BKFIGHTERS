#include "stdafx.h"
#include "GSPlay.h"
#include "GameStateMachine.h"
#include "../Core/Globals.h"
#include <iostream>
#include "../GameObject/Object.h"
#include "../GameObject/Texture2D.h"
#include <memory>
#include "../GameObject/Animation2D.h"

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
    // Luôn override lại camera aspect sau khi load scene
    sceneManager->AdjustCameraToWindow();
    
    m_gameTime = 0.0f;
    m_charPosX = 0.0f; // Vị trí giữa màn hình 
    m_charPosY = 0.0f;

    // Khởi tạo animation đúng với spritesheet gốc (12 cột, 4 hàng), chỉ lấy nhân vật vàng (col 0-2)
    m_anim = std::make_shared<Animation2D>(12, 4, 0.1f);
    m_anim->SetColRange(0, 2); // Chỉ chạy trong 3 cột đầu
    m_anim->SetRow(0); // Mặc định: action đầu tiên 
    m_anim->SetCol(1); // Đứng yên ở frame giữa
    m_charState = CharState::Idle;
    m_facingRow = 0;
    
    std::cout << "Gameplay initialized" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "- ESC or M: Return to menu" << std::endl;
    std::cout << "- P: Quick play (from menu)" << std::endl;
    std::cout << "- Q: Help/Question (from menu)" << std::endl;
    std::cout << "- X: Exit game (from menu)" << std::endl;
    std::cout << "- W/A/S/D: Doi action nhan vat mau vang" << std::endl;
    
    Camera* cam = SceneManager::GetInstance()->GetActiveCamera();
    std::cout << "Camera actual: left=" << cam->GetLeft() << ", right=" << cam->GetRight()
              << ", bottom=" << cam->GetBottom() << ", top=" << cam->GetTop() << std::endl;
}

void GSPlay::Update(float deltaTime) {
    m_gameTime += deltaTime;
    float moveSpeed = 0.5f; // pixel/giây
    
    // Update scene
    SceneManager::GetInstance()->Update(deltaTime);
    
    // Xử lý trạng thái nhân vật dựa trên keyStates (ưu tiên W > A > D > S)
    if (keyStates['W'] || keyStates['w']) {
        m_facingRow = 0;
        m_charState = CharState::MoveFront;
        if (m_anim) {
            m_anim->SetRow(0);
            m_anim->SetColRange(0,2);
        }
    } else if (keyStates['A'] || keyStates['a']) {
        m_facingRow = 2; //row 2 là trái
        m_charState = CharState::MoveLeft;
        if (m_anim) {
            m_anim->SetRow(2);
            m_anim->SetColRange(0,2);
        }
        m_charPosX -= moveSpeed * deltaTime; // Di chuyển sang trái
    } else if (keyStates['D'] || keyStates['d']) {
        m_facingRow = 1; //row 1 là phải
        m_charState = CharState::MoveRight;
        if (m_anim) {
            m_anim->SetRow(1);
            m_anim->SetColRange(0,2);
        }
        m_charPosX += moveSpeed * deltaTime; // Di chuyển sang phải
    } else if (keyStates['S'] || keyStates['s']) {
        m_facingRow = 3;
        m_charState = CharState::MoveBack;
        if (m_anim) {
            m_anim->SetRow(3);
            m_anim->SetColRange(0,2);
        }
    } else {
        m_charState = CharState::Idle;
        if (m_anim) {
            m_anim->SetRow(m_facingRow);
            m_anim->SetCol(1);
            m_anim->SetColRange(1,1);
        }
    }
    // Update animation
    if (m_anim) {
        if (m_charState == CharState::Idle) {
            m_anim->SetCol(1);
            m_anim->SetColRange(1,1);
        } else {
            m_anim->SetColRange(0,2);
            m_anim->Update(deltaTime);
        }
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
    if (animObj && m_anim && animObj->GetModelId() >= 0 && animObj->GetModelPtr()) {
        float u0, v0, u1, v1;
        m_anim->GetUV(u0, v0, u1, v1);
        animObj->SetCustomUV(u0, v0, u1, v1);
        // Truyền vị trí mới cho object nhân vật
        animObj->SetPosition(Vector3(m_charPosX, m_charPosY, 0.0f));
        Camera* cam = SceneManager::GetInstance()->GetActiveCamera();
        if (cam) animObj->Draw(cam->GetViewMatrix(), cam->GetProjectionMatrix());
    }

}

void GSPlay::HandleKeyEvent(unsigned char key, bool bIsPressed) {
    keyStates[key] = bIsPressed;
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