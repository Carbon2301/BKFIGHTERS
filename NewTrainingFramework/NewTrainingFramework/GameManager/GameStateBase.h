#pragma once
#include "StateType.h"

class GameStateBase {
protected:
    StateType m_stateType;

public:
    GameStateBase(StateType stateType);
    virtual ~GameStateBase();

    // State lifecycle
    virtual void Init() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Draw() = 0;
    virtual void HandleKeyEvent(unsigned char key, bool bIsPressed) = 0;
    virtual void Resume() = 0;
    virtual void Pause() = 0;
    virtual void Exit() = 0;
    virtual void Cleanup() = 0;

    // Getters
    StateType GetStateType() const { return m_stateType; }
}; 