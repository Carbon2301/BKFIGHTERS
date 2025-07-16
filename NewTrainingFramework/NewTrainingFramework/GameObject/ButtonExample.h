#pragma once
#include "GameButton.h"
#include "../GameManager/GameStateMachine.h"

class ButtonExample {
public:
    static void CreatePlayButton() {

    }
    
private:
    // Example callback function
    static void OnPlayButtonClicked() {
        GameStateMachine::GetInstance()->ChangeState(StateType::STATE_PLAY);
    }
}; 