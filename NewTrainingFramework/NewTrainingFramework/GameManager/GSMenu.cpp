#include "stdafx.h"
#include "GSMenu.h"
#include "GameStateMachine.h"
#include "../Core/Globals.h"
#include <iostream>
#include <SDL_ttf.h>
#include "../GameObject/Texture2D.h"
#include "../GameObject/Object.h"
#include "../GameManager/ResourceManager.h"
#include "../GameObject/Model.h"
#include "../GameObject/Shaders.h"

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

    //Tạo texture text động bằng SDL_ttf
    if (TTF_WasInit() == 0) TTF_Init();
    TTF_Font* font = TTF_OpenFont("../Resources/Font/Roboto-Regular.ttf", 32);
    if (!font) {
        std::cout << "Failed to load font! TTF_Error: " << TTF_GetError() << std::endl;
        return;
    }
    SDL_Color color = {255, 255, 255, 255};
    SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, "Hello", color);
    if (!textSurface) {
        std::cout << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
        TTF_CloseFont(font);
        return;
    }
    m_textTexture = std::make_shared<Texture2D>();
    m_textTexture->LoadFromSDLSurface(textSurface);
    SDL_FreeSurface(textSurface);
    TTF_CloseFont(font);

    //Tạo object vẽ text
    m_textObject = std::make_shared<Object>();
    m_textObject->SetId(-100); // ID đặc biệt cho text động
    m_textObject->SetModel(0);
    m_textObject->SetShader(0);
    m_textObject->SetDynamicTexture(m_textTexture);
    float x = -3.7f;
    float y = 8.5f;
    m_textObject->Set2DPosition(x, y);
    m_textObject->SetSize(0.3f, 0.1f);
}

void GSMenu::Update(float deltaTime) {
    m_buttonTimer += deltaTime;
    
    SceneManager::GetInstance()->Update(deltaTime);
    
}

void GSMenu::Draw() {
    SceneManager::GetInstance()->Draw();
    // --- Vẽ text động ---
    if (m_textObject) {
        Camera* cam = SceneManager::GetInstance()->GetActiveCamera();
        if (cam) {
            m_textObject->Draw(cam->GetViewMatrix(), cam->GetProjectionMatrix());
        }
    }
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