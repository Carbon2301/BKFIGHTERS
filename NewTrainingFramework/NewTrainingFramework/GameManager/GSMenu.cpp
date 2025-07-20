#include "stdafx.h"
#include "GSMenu.h"
#include "GameStateMachine.h"
#include "../Core/Globals.h"
#include <iostream>

GSMenu::GSMenu() 
    : GameStateBase(StateType::MENU), m_buttonTimer(0.0f) {
}

GSMenu::~GSMenu() {
}

void GSMenu::Init() {
    std::cout << "=== MAIN MENU ===" << std::endl;
    std::cout << "Welcome to the Game!" << std::endl;
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    SceneManager* sceneManager = SceneManager::GetInstance();
    sceneManager->LoadSceneForState(StateType::MENU);
    
    std::cout << "Use P for Play, Q for Question/Help, X for Exit" << std::endl;
    std::cout << "ESC to exit game" << std::endl;
}

void GSMenu::Update(float deltaTime) {
    m_buttonTimer += deltaTime;
    
    // Update scene
    SceneManager::GetInstance()->Update(deltaTime);
    
}

void GSMenu::Draw() {
    SceneManager::GetInstance()->Draw();
}

void GSMenu::HandleKeyEvent(unsigned char key, bool bIsPressed) {
    if (!bIsPressed) return;
    
    switch (key) {
        case 'P':
        case 'p':
            // Direct play button
            std::cout << "=== Starting Game ===" << std::endl;
            GameStateMachine::GetInstance()->PushState(StateType::PLAY);
            break;
            
        case 'Q':
        case 'q':
            // Direct question/help button
            std::cout << "=== HELP ===" << std::endl;
            std::cout << "Game Controls:" << std::endl;
            std::cout << "- P: Play game" << std::endl;
            std::cout << "- Q: Help/Question" << std::endl;
            std::cout << "- X: Exit game" << std::endl;
            std::cout << "- ESC: Exit game" << std::endl;
            break;
            
        case 'X':
        case 'x':
            // Direct exit button
            std::cout << "=== Closing Game ===" << std::endl;
            std::cout << "Thanks for playing!" << std::endl;
            break;
            
        case 13: // Enter
        case ' ': // Space
            HandleButtonSelection();
            break;
            
        case 27: // ESC
            std::cout << "Exit game requested" << std::endl;
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

void GSMenu::HandleMouseEvent(int x, int y, bool bIsPressed) {
    if (!bIsPressed) return;

    SceneManager* sceneManager = SceneManager::GetInstance();
    Camera* camera = sceneManager->GetActiveCamera();
    if (!camera) return;

    float worldX = MousePixelToWorldX(x, camera);
    float worldY = MousePixelToWorldY(y, camera);

    std::cout << "Mouse pixel: (" << x << ", " << y << "), world: (" << worldX << ", " << worldY << ")\n";
    std::cout << "Camera: left=" << camera->GetLeft() << ", right=" << camera->GetRight()
              << ", top=" << camera->GetTop() << ", bottom=" << camera->GetBottom() << std::endl;

    const auto& objects = sceneManager->GetObjects();
    for (const auto& objPtr : objects) {
        int id = objPtr->GetId();
        if (id == BUTTON_ID_PLAY || id == BUTTON_ID_HELP || id == BUTTON_ID_CLOSE) {
            const Vector3& pos = objPtr->GetPosition();
            const Vector3& scale = objPtr->GetScale();
            float width = scale.x;
            float height = scale.y;
            float leftBtn = pos.x - width / 2.0f;
            float rightBtn = pos.x + width / 2.0f;
            float bottomBtn = pos.y - height / 2.0f;
            float topBtn = pos.y + height / 2.0f;
            std::cout << "Button ID " << id << " region: left=" << leftBtn << ", right=" << rightBtn
                      << ", top=" << topBtn << ", bottom=" << bottomBtn << std::endl;
            if (worldX >= leftBtn && worldX <= rightBtn && worldY >= bottomBtn && worldY <= topBtn) {
                std::cout << "HIT button ID " << id << std::endl;
                if (id == BUTTON_ID_PLAY) {
                    std::cout << "[Mouse] Play button clicked!" << std::endl;
                    GameStateMachine::GetInstance()->PushState(StateType::PLAY);
                } else if (id == BUTTON_ID_HELP) {
                    std::cout << "[Mouse] Help button clicked!" << std::endl;
                    std::cout << "Game Controls:" << std::endl;
                    std::cout << "- P: Play game" << std::endl;
                    std::cout << "- Q: Help/Question" << std::endl;
                    std::cout << "- X: Exit game" << std::endl;
                    std::cout << "- ESC: Exit game" << std::endl;
                } else if (id == BUTTON_ID_CLOSE) {
                    std::cout << "[Mouse] Close button clicked!" << std::endl;
                    std::cout << "Thanks for playing!" << std::endl;
                    exit(0);
                }
                break;
            }
        }
    }
}

void GSMenu::HandleMouseMove(int x, int y) {
    // std::cout << "Mouse moved to (" << x << ", " << y << ")" << std::endl;
}

void GSMenu::HandleButtonSelection() {
    std::cout << "=== Available Actions ===" << std::endl;
    std::cout << "Game Controls:" << std::endl;
    std::cout << "- P: Play game" << std::endl;
    std::cout << "- Q: Help/Question" << std::endl;
    std::cout << "- X: Exit game" << std::endl;
    std::cout << "- ESC: Exit game" << std::endl;
}

void GSMenu::Resume() {
    std::cout << "GSMenu: Resume (Back to Main Menu)" << std::endl;
    std::cout << "Welcome back to the main menu!" << std::endl;
}

void GSMenu::Pause() {
    std::cout << "GSMenu: Pause" << std::endl;
}

void GSMenu::Exit() {
    std::cout << "GSMenu: Exit" << std::endl;
}

void GSMenu::Cleanup() {
    std::cout << "GSMenu: Cleanup" << std::endl;
} 