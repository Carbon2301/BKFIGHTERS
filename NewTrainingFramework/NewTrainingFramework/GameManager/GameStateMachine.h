#pragma once
#include "GameStateBase.h"
#include "StateType.h"
#include <stack>
#include <memory>

class GameStateMachine {
private:
    static GameStateMachine* s_instance;
    
    std::stack<std::unique_ptr<GameStateBase>> m_stateStack;
    std::unique_ptr<GameStateBase> m_pActiveState;
    std::unique_ptr<GameStateBase> m_pNextState;

    GameStateMachine();

public:
    static GameStateMachine* GetInstance();
    static void DestroyInstance();
    
    ~GameStateMachine();

    // State management 
    void ChangeState(StateType stateType);
    void PushState(StateType stateType);
    void PopState();
    void PerformStateChange();

    // Main loop functions
    void Update(float deltaTime);
    void Draw();
    void HandleKeyEvent(unsigned char key, bool bIsPressed);
    void HandleMouseEvent(int x, int y, bool bIsPressed);
    void HandleMouseMove(int x, int y);

    // Utilities
    GameStateBase* CurrentState();
    bool HasState();
    void Cleanup();

private:
    std::unique_ptr<GameStateBase> CreateState(StateType stateType);
}; 