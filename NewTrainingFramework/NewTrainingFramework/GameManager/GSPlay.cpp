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
static bool m_facingLeft = false; // Nhớ hướng cuối cùng

// Combo system variables
static int m_comboCount = 0; // 0, 1, 2, 3
static float m_comboTimer = 0.0f; // Thời gian để combo tiếp tục
static const float COMBO_WINDOW = 0.5f; // 0.5 giây để nhấn J tiếp
static bool m_isInCombo = false; // Đang trong combo hay không
static bool m_comboCompleted = false; // Combo đã hoàn thành chưa

// Axe combo system variables
static int m_axeComboCount = 0; // 0, 1, 2, 3
static float m_axeComboTimer = 0.0f; // Thời gian để axe combo tiếp tục
static bool m_isInAxeCombo = false; // Đang trong axe combo hay không
static bool m_axeComboCompleted = false; // Axe combo đã hoàn thành chưa

// Jump system variables
static bool m_isJumping = false; // Đang nhảy hay không
static float m_jumpVelocity = 0.0f; // Vận tốc nhảy
static float m_jumpStartY = 0.0f; // Vị trí Y bắt đầu nhảy
static const float JUMP_FORCE = 3.0f; // Lực nhảy
static const float GRAVITY = 8.0f; // Trọng lực
static const float GROUND_Y = 0.0f; // Vị trí mặt đất

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
        m_animManager->Play(0, true); // Play idle animation (0)
        std::cout << "Animation system initialized with " << animations.size() << " animations" << std::endl;
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
    std::cout << "  * Combo window: " << COMBO_WINDOW << " seconds" << std::endl;
    std::cout << "- L: Start/Continue Axe Combo" << std::endl;
    std::cout << "  * Press L once: Axe1 (Animation 19: Axe1)" << std::endl;
    std::cout << "  * Press L twice: Axe2 (Animation 20: Axe2)" << std::endl;
    std::cout << "  * Press L three times: Axe3 (Animation 21: Axe3)" << std::endl;
    std::cout << "  * Combo window: " << COMBO_WINDOW << " seconds" << std::endl;
    std::cout << "- K: Kick (Animation 18: Kick)" << std::endl;

    
    Camera* cam = SceneManager::GetInstance()->GetActiveCamera();
    std::cout << "Camera actual: left=" << cam->GetLeft() << ", right=" << cam->GetRight()
              << ", bottom=" << cam->GetBottom() << ", top=" << cam->GetTop() << std::endl;
}

void GSPlay::Update(float deltaTime) {
    m_gameTime += deltaTime;
    float moveSpeed = 0.5f; // units/giây - di chuyển chậm hơn
    
    // Update scene
    SceneManager::GetInstance()->Update(deltaTime);
    
    // Update combo timer
    if (m_isInCombo) {
        m_comboTimer -= deltaTime;
        if (m_comboTimer <= 0.0f) {
            // Combo timeout - reset về idle
            m_isInCombo = false;
            m_comboCount = 0;
            m_comboCompleted = false;
            if (m_animManager) {
                m_animManager->Play(0, true); // Return to idle
            }
            std::cout << "Combo timeout - returning to idle" << std::endl;
        }
    }
    
    // Update axe combo timer
    if (m_isInAxeCombo) {
        m_axeComboTimer -= deltaTime;
        if (m_axeComboTimer <= 0.0f) {
            // Axe combo timeout - reset về idle
            m_isInAxeCombo = false;
            m_axeComboCount = 0;
            m_axeComboCompleted = false;
            if (m_animManager) {
                m_animManager->Play(0, true); // Return to idle
            }
            std::cout << "Axe combo timeout - returning to idle" << std::endl;
        }
    }
    
    // Xử lý di chuyển dựa trên trạng thái phím
    static int lastAnimation = -1; // Để tránh gọi Play() liên tục
    
    // Kiểm tra Shift key (VK_SHIFT = 16)
    bool isShiftPressed = keyStates[16];
    
    // Reset combo chỉ khi thực hiện hành động khác (không phải di chuyển)
    bool isMoving = (keyStates['A'] || keyStates['a'] || keyStates['D'] || keyStates['d']);
    bool isOtherAction = (keyStates['S'] || keyStates['s'] || keyStates[' '] || keyStates['W'] || keyStates['w']);
    
    if (isOtherAction && m_isInCombo) {
        m_isInCombo = false;
        m_comboCount = 0;
        m_comboTimer = 0.0f;
        m_comboCompleted = false;
        std::cout << "Combo cancelled due to other action (Sit/Roll/Jump)" << std::endl;
    }
    
    if (isOtherAction && m_isInAxeCombo) {
        m_isInAxeCombo = false;
        m_axeComboCount = 0;
        m_axeComboTimer = 0.0f;
        m_axeComboCompleted = false;
        std::cout << "Axe combo cancelled due to other action (Sit/Roll/Jump)" << std::endl;
    }
    
    // Xử lý Jump
    if (keyStates['W'] || keyStates['w']) {
        if (!m_isJumping) {
            // Bắt đầu nhảy
            m_isJumping = true;
            m_jumpVelocity = JUMP_FORCE;
            m_jumpStartY = m_charPosY;
            if (m_animManager) {
                m_animManager->Play(15, false); // Jump animation (không loop)
            }
            std::cout << "=== JUMP START ===" << std::endl;
        }
    }
    
    // Cập nhật vị trí nhảy
    if (m_isJumping) {
        m_jumpVelocity -= GRAVITY * deltaTime;
        m_charPosY += m_jumpVelocity * deltaTime;
        
        // Cho phép di chuyển trong không khí khi đang nhảy
        bool isMovingLeft = (keyStates['A'] || keyStates['a']);
        bool isMovingRight = (keyStates['D'] || keyStates['d']);
        bool isShiftPressed = keyStates[16];
        
        if (isMovingLeft) {
            m_facingLeft = true;
            m_charState = CharState::MoveLeft;
            if (isShiftPressed) {
                m_charPosX -= moveSpeed * 1.5f * deltaTime; // Chạy trong không khí
            } else {
                m_charPosX -= moveSpeed * 0.8f * deltaTime; // Đi bộ trong không khí (chậm hơn)
            }
        } else if (isMovingRight) {
            m_facingLeft = false;
            m_charState = CharState::MoveRight;
            if (isShiftPressed) {
                m_charPosX += moveSpeed * 1.5f * deltaTime; // Chạy trong không khí
            } else {
                m_charPosX += moveSpeed * 0.8f * deltaTime; // Đi bộ trong không khí (chậm hơn)
            }
        }
        
        // Kiểm tra chạm đất
        if (m_charPosY <= GROUND_Y) {
            m_charPosY = GROUND_Y;
            m_isJumping = false;
            m_jumpVelocity = 0.0f;
            
            // Kiểm tra xem có đang giữ phím di chuyển không trước khi chuyển về idle
            if (isMovingLeft || isMovingRight) {
                // Cập nhật hướng di chuyển
                if (isMovingLeft) {
                    m_facingLeft = true;
                    m_charState = CharState::MoveLeft;
                } else if (isMovingRight) {
                    m_facingLeft = false;
                    m_charState = CharState::MoveRight;
                }
                
                // Nếu đang giữ phím di chuyển, chuyển về animation di chuyển tương ứng
                if (isShiftPressed) {
                    if (m_animManager) {
                        m_animManager->Play(2, true); // Run animation
                    }
                } else {
                    if (m_animManager) {
                        m_animManager->Play(1, true); // Walk animation
                    }
                }
            } else {
                // Nếu không di chuyển, chuyển về idle
                m_charState = CharState::Idle;
                if (m_animManager) {
                    m_animManager->Play(0, true); // Return to idle
                }
            }
            std::cout << "=== JUMP END ===" << std::endl;
        }
    }
    
    // Kiểm tra Roll trước (priority cao nhất)
    if ((keyStates['A'] || keyStates['a']) && (keyStates[' '])) {
        // Roll Left (animation 4) - lăn sang trái
        m_charState = CharState::MoveLeft;
        m_facingLeft = true;
        m_charPosX -= moveSpeed * 1.5f * deltaTime; // Roll nhanh hơn walk
        if (m_animManager && lastAnimation != 4) {
            m_animManager->Play(4, true); // Roll animation (sẽ đảo ngược UV)
            lastAnimation = 4;
        }
    } else if ((keyStates['D'] || keyStates['d']) && (keyStates[' '])) {
        // Roll Right (animation 4) - lăn sang phải
        m_charState = CharState::MoveRight;
        m_facingLeft = false; // Nhớ hướng phải
        m_charPosX += moveSpeed * 1.5f * deltaTime; // Roll nhanh hơn walk
        if (m_animManager && lastAnimation != 4) {
            m_animManager->Play(4, true); // Roll animation
            lastAnimation = 4;
        }
    } else if (keyStates[' ']) {
        // Roll theo hướng cuối cùng (animation 4)
        if (m_facingLeft) {
            m_charPosX -= moveSpeed * 1.5f * deltaTime; // Roll trái
        } else {
            m_charPosX += moveSpeed * 1.5f * deltaTime; // Roll phải
        }
        if (m_animManager && lastAnimation != 4) {
            m_animManager->Play(4, true); // Roll animation
            lastAnimation = 4;
        }
    } else if (keyStates['D'] || keyStates['d']) {
        // Di chuyển sang phải
        m_charState = CharState::MoveRight;
        m_facingLeft = false; // Nhớ hướng phải
        
        if (isShiftPressed) {
            // Chạy sang phải
            m_charPosX += moveSpeed * 2.0f * deltaTime; // Chạy nhanh gấp đôi
        } else {
            // Đi bộ sang phải
            m_charPosX += moveSpeed * deltaTime;
        }
        
        // Chỉ thay đổi animation nếu không đang trong combo
        if (!m_isInCombo && !m_isInAxeCombo) {
            if (isShiftPressed) {
                if (m_animManager && lastAnimation != 2) {
                    m_animManager->Play(2, true); // Run animation
                    lastAnimation = 2;
                }
            } else {
                if (m_animManager && lastAnimation != 1) {
                    m_animManager->Play(1, true); // Walk animation
                    lastAnimation = 1;
                }
            }
        }
    } else if (keyStates['A'] || keyStates['a']) {
        // Di chuyển sang trái
        m_charState = CharState::MoveLeft;
        m_facingLeft = true; // Nhớ hướng trái
        
        if (isShiftPressed) {
            // Chạy sang trái
            m_charPosX -= moveSpeed * 2.0f * deltaTime; // Chạy nhanh gấp đôi
        } else {
            // Đi bộ sang trái
            m_charPosX -= moveSpeed * deltaTime;
        }
        
        // Chỉ thay đổi animation nếu không đang trong combo
        if (!m_isInCombo && !m_isInAxeCombo) {
            if (isShiftPressed) {
                if (m_animManager && lastAnimation != 2) {
                    m_animManager->Play(2, true); // Run animation (sẽ đảo ngược UV)
                    lastAnimation = 2;
                }
            } else {
                if (m_animManager && lastAnimation != 1) {
                    m_animManager->Play(1, true); // Walk animation (sẽ đảo ngược UV)
                    lastAnimation = 1;
                }
            }
        }
    } else if (keyStates['S'] || keyStates['s']) {
        // Sit animation (animation 3) - ngồi xuống
        m_charState = CharState::Idle;
        if (m_animManager && lastAnimation != 3) {
            m_animManager->Play(3, true); // Sit animation
            lastAnimation = 3;
        }
    } else {
        // Idle animation (animation 0) - giữ hướng cuối cùng
        // Chỉ chuyển về idle nếu không đang trong combo
        if (!m_isInCombo && !m_isInAxeCombo) {
            m_charState = CharState::Idle;
            if (m_animManager && lastAnimation != 0) {
                m_animManager->Play(0, true); // Idle animation
                lastAnimation = 0;
            }
        }
    }
    
    // Update animation
    if (m_animManager) {
        m_animManager->Update(deltaTime);
        
        // Check if combo animation finished
        if (m_isInCombo && !m_animManager->IsPlaying()) {
            // Animation finished, check if we need to continue combo or return to idle
            if (m_comboCompleted) {
                // Combo completed, return to idle
                m_isInCombo = false;
                m_comboCount = 0;
                m_comboCompleted = false;
                m_animManager->Play(0, true); // Return to idle
                std::cout << "Combo completed - returning to idle" << std::endl;
            } else if (m_comboTimer <= 0.0f) {
                // Combo timeout, return to idle
                m_isInCombo = false;
                m_comboCount = 0;
                m_comboCompleted = false;
                m_animManager->Play(0, true); // Return to idle
                std::cout << "Combo timeout - returning to idle" << std::endl;
            }
        }
        
        // Kiểm tra animation đá đã kết thúc chưa
        if (!m_isInCombo && !m_isInAxeCombo && m_animManager->GetCurrentAnimation() == 18 && !m_animManager->IsPlaying()) {
            // Animation đá kết thúc, trở về trạng thái idle
            m_animManager->Play(0, true); // Trở về idle
            std::cout << "Animation đá kết thúc - trở về trạng thái idle" << std::endl;
        }
        
        // Check if axe combo animation finished
        if (m_isInAxeCombo && !m_animManager->IsPlaying()) {
            // Animation finished, check if we need to continue combo or return to idle
            if (m_axeComboCompleted) {
                // Combo completed, return to idle
                m_isInAxeCombo = false;
                m_axeComboCount = 0;
                m_axeComboCompleted = false;
                m_animManager->Play(0, true); // Return to idle
                std::cout << "Axe combo completed - returning to idle" << std::endl;
            } else if (m_axeComboTimer <= 0.0f) {
                // Combo timeout, return to idle
                m_isInAxeCombo = false;
                m_axeComboCount = 0;
                m_axeComboCompleted = false;
                m_animManager->Play(0, true); // Return to idle
                std::cout << "Axe combo timeout - returning to idle" << std::endl;
            }
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
    if (animObj && m_animManager && animObj->GetModelId() >= 0 && animObj->GetModelPtr()) {
        float u0, v0, u1, v1;
        m_animManager->GetUV(u0, v0, u1, v1);
        
        // Đảo ngược UV coordinates khi nhìn sang trái
        if (m_facingLeft) {
            // Đảo ngược U coordinates để flip sprite
            float temp = u0;
            u0 = u1;
            u1 = temp;
        }
        
        animObj->SetCustomUV(u0, v0, u1, v1);
        // Truyền vị trí mới cho object nhân vật
        animObj->SetPosition(Vector3(m_charPosX, m_charPosY, 0.0f));
        Camera* cam = SceneManager::GetInstance()->GetActiveCamera();
        if (cam) animObj->Draw(cam->GetViewMatrix(), cam->GetProjectionMatrix());
        
        // Debug info - hiển thị vị trí hiện tại và sau khi nhấn phím
        static float lastPosX = m_charPosX;
        static int lastAnim = m_animManager->GetCurrentAnimation();
        static bool wasMoving = false;
        
        bool isMoving = (keyStates['A'] || keyStates['a'] || keyStates['D'] || keyStates['d']);
        bool isOtherAction = (keyStates['S'] || keyStates['s'] || keyStates[' ']);
        
        if (abs(m_charPosX - lastPosX) > 0.01f || 
            lastAnim != m_animManager->GetCurrentAnimation() ||
            (isMoving && !wasMoving)) {
            
            std::cout << "=== CHARACTER STATUS ===" << std::endl;
            std::cout << "Position: (" << m_charPosX << ", " << m_charPosY << ")" << std::endl;
            std::cout << "State: " << (int)m_charState << std::endl;
            std::cout << "Animation: " << m_animManager->GetCurrentAnimation() << std::endl;
            std::cout << "Facing: " << (m_facingLeft ? "LEFT" : "RIGHT") << std::endl;
            std::cout << "Movement: " << (isMoving ? "ACTIVE" : "IDLE") << std::endl;
            if (m_isInCombo) {
                std::cout << "Combo: " << m_comboCount << "/3 (Timer: " << m_comboTimer << "s)";
                if (m_comboCompleted) {
                    std::cout << " [COMPLETED]";
                }
                if (isMoving) {
                    std::cout << " [MOVING DURING COMBO]";
                }
                std::cout << std::endl;
            } else if (m_isInAxeCombo) {
                std::cout << "Axe Combo: " << m_axeComboCount << "/3 (Timer: " << m_axeComboTimer << "s)";
                if (m_axeComboCompleted) {
                    std::cout << " [COMPLETED]";
                }
                if (isMoving) {
                    std::cout << " [MOVING DURING COMBO]";
                }
                std::cout << std::endl;
            } else if (m_animManager->GetCurrentAnimation() == 18) {
                std::cout << "Action: KICK [Animation 18]" << std::endl;
            }
            std::cout << "=========================" << std::endl;
            
            lastPosX = m_charPosX;
            lastAnim = m_animManager->GetCurrentAnimation();
            wasMoving = isMoving;
        }
    }
}

void GSPlay::HandleKeyEvent(unsigned char key, bool bIsPressed) {
    keyStates[key] = bIsPressed;
    
    if (bIsPressed) {
        // Combo system - J key
        if (key == 'J' || key == 'j') {
            if (!m_isInCombo) {
                // Bắt đầu combo mới
                m_comboCount = 1;
                m_isInCombo = true;
                m_comboTimer = COMBO_WINDOW;
                if (m_animManager) {
                    m_animManager->Play(9, false); // Punch1 - không loop
                }
                std::cout << "=== COMBO START ===" << std::endl;
                std::cout << "Combo " << m_comboCount << ": Punch1!" << std::endl;
                std::cout << "Press J again within " << COMBO_WINDOW << " seconds for next punch!" << std::endl;
            } else if (m_comboTimer > 0.0f) {
                // Tiếp tục combo
                m_comboCount++;
                m_comboTimer = COMBO_WINDOW; // Reset timer
                
                if (m_comboCount == 2) {
                    if (m_animManager) {
                        m_animManager->Play(10, false); // Punch2 - không loop
                    }
                    std::cout << "=== COMBO CONTINUE ===" << std::endl;
                    std::cout << "Combo " << m_comboCount << ": Punch2!" << std::endl;
                    std::cout << "Press J again within " << COMBO_WINDOW << " seconds for final punch!" << std::endl;
                } else if (m_comboCount == 3) {
                    if (m_animManager) {
                        m_animManager->Play(11, false); // Punch3 - không loop
                    }
                    std::cout << "=== COMBO FINISH ===" << std::endl;
                    std::cout << "Combo " << m_comboCount << ": Punch3! COMBO COMPLETE!" << std::endl;
                    // Combo hoàn thành, set flag để logic Update xử lý
                    m_comboCompleted = true;
                }
            }
        }
        
        // Kick system - K key
        if (key == 'K' || key == 'k') {
            // Hủy combo đang chạy
            if (m_isInCombo) {
                m_isInCombo = false;
                m_comboCount = 0;
                m_comboTimer = 0.0f;
                m_comboCompleted = false;
                std::cout << "Combo cancelled due to kick" << std::endl;
            }
            
            // Hủy axe combo đang chạy
            if (m_isInAxeCombo) {
                m_isInAxeCombo = false;
                m_axeComboCount = 0;
                m_axeComboTimer = 0.0f;
                m_axeComboCompleted = false;
                std::cout << "Axe combo cancelled due to kick" << std::endl;
            }
            
            // Play kick animation
            if (m_animManager) {
                m_animManager->Play(18, false); // Kick animation (Animation 18) - không loop
                std::cout << "=== KICK EXECUTED ===" << std::endl;
                std::cout << "Playing Kick animation (Animation 18)" << std::endl;
            }
        }
        
        // Axe combo system - L key
        if (key == 'L' || key == 'l') {
            if (!m_isInAxeCombo) {
                // Bắt đầu axe combo mới
                m_axeComboCount = 1;
                m_isInAxeCombo = true;
                m_axeComboTimer = COMBO_WINDOW;
                if (m_animManager) {
                    m_animManager->Play(19, false); // Axe1 - không loop
                }
                std::cout << "=== AXE COMBO START ===" << std::endl;
                std::cout << "Axe Combo " << m_axeComboCount << ": Axe1!" << std::endl;
                std::cout << "Press L again within " << COMBO_WINDOW << " seconds for next axe attack!" << std::endl;
            } else if (m_axeComboTimer > 0.0f) {
                // Tiếp tục axe combo
                m_axeComboCount++;
                m_axeComboTimer = COMBO_WINDOW; // Reset timer
                
                if (m_axeComboCount == 2) {
                    if (m_animManager) {
                        m_animManager->Play(20, false); // Axe2 - không loop
                    }
                    std::cout << "=== AXE COMBO CONTINUE ===" << std::endl;
                    std::cout << "Axe Combo " << m_axeComboCount << ": Axe2!" << std::endl;
                    std::cout << "Press L again within " << COMBO_WINDOW << " seconds for final axe attack!" << std::endl;
                } else if (m_axeComboCount == 3) {
                    if (m_animManager) {
                        m_animManager->Play(21, false); // Axe3 - không loop
                    }
                    std::cout << "=== AXE COMBO FINISH ===" << std::endl;
                    std::cout << "Axe Combo " << m_axeComboCount << ": Axe3! AXE COMBO COMPLETE!" << std::endl;
                    // Combo hoàn thành, set flag để logic Update xử lý
                    m_axeComboCompleted = true;
                }
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