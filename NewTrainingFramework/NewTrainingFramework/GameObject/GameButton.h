#pragma once
#include "Object.h"

class Model;
class Shaders; 
class Texture2D;

typedef void (*ButtonClickCallback)();

class GameButton : public Object {
private:
    ButtonClickCallback m_onClick;
    bool m_isHolding;
    
public:
    // Constructors
    GameButton();
    GameButton(int id);
    GameButton(std::shared_ptr<Model> model, std::shared_ptr<Shaders> shader, std::shared_ptr<Texture2D> texture);
    ~GameButton();
    
    // Button functionality
    void SetOnClick(ButtonClickCallback callback);
    bool HandleTouchEvents(float x, float y, bool isPressed);
    bool IsHolding() const;
    
private:
    bool IsPointInside(float x, float y) const;
}; 