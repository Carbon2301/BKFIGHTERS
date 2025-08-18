#include "stdafx.h"
#include "GameButton.h"
#include "../GameManager/ResourceManager.h"
#include "Model.h"
#include "Texture2D.h" 
#include "Shaders.h"

GameButton::GameButton() : Object(), m_onClick(nullptr), m_isHolding(false) {
}

GameButton::GameButton(int id) : Object(id), m_onClick(nullptr), m_isHolding(false) {
}

GameButton::GameButton(std::shared_ptr<Model> model, std::shared_ptr<Shaders> shader, std::shared_ptr<Texture2D> texture)
    : Object(-1), m_onClick(nullptr), m_isHolding(false) {

}

GameButton::~GameButton() {
}

void GameButton::SetOnClick(ButtonClickCallback callback) {
    m_onClick = callback;
}

bool GameButton::HandleTouchEvents(float x, float y, bool isPressed) {
    bool isHandled = false;
    
    if (isPressed) {
        if (IsPointInside(x, y)) {
            m_isHolding = true;
            isHandled = true;
        }
    } else {
        if (IsPointInside(x, y) && m_isHolding) {
            if (m_onClick != nullptr) {
                m_onClick();
            }
            isHandled = true;
        }
        m_isHolding = false;
    }
    
    return isHandled;
}

bool GameButton::IsHolding() const {
    return m_isHolding;
}

bool GameButton::IsPointInside(float x, float y) const {
    const Vector3& position = GetPosition();
    const Vector3& scale = GetScale();
    
    // Use scale as size
    float width = scale.x;
    float height = scale.y;
    
    float left = position.x - width / 2.0f;
    float right = position.x + width / 2.0f;
    float bottom = position.y - height / 2.0f;
    float top = position.y + height / 2.0f;
    
    // Check if point (x, y) is inside button bounds
    bool inside = (x >= left && x <= right && y >= bottom && y <= top);
    
    return inside;
} 