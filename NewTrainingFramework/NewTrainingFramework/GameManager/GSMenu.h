#pragma once
#include "GameStateBase.h"
#include "SceneManager.h"
#include "../GameObject/Object.h"
#include "../GameObject/Texture2D.h"

class GSMenu : public GameStateBase {
private:
    float m_buttonTimer;
    bool m_isSettingsVisible;
    bool m_isCreditsVisible;
    bool m_isTutorialVisible;
    int m_currentTutorialPage;
    
    // Music slider
    bool m_isDraggingMusicSlider;
    float m_musicVolume;
    Vector3 m_musicSliderOriginalPos;
    
    // SFX slider
    bool m_isDraggingSFXSlider;
    float m_sfxVolume;
    Vector3 m_sfxSliderOriginalPos;
    
    static float s_savedMusicVolume;
    static float s_savedSFXVolume;
    
    enum ButtonType {
        BUTTON_PLAY = 0,
        BUTTON_SETTINGS = 1,
        BUTTON_EXIT = 2,
        BUTTON_COUNT = 3
    };
    
    static const int BUTTON_ID_PLAY = 201;
    static const int BUTTON_ID_HELP = 202;
    static const int BUTTON_ID_SETTINGS = 203;
    static const int BUTTON_ID_CREDITS = 204;
    static const int BUTTON_ID_EXIT = 205;
    
    static const int SETTINGS_UI_ID = 206;
    static const int SETTINGS_MUSIC_ID = 207;
    static const int SETTINGS_SFX_ID = 208;
    static const int SETTINGS_APPLY_ID = 209;
    static const int SETTINGS_MUSIC_SLIDER_ID = 210;
    static const int SETTINGS_SFX_SLIDER_ID = 211;
    static const int CREDITS_UI_ID = 212;
    static const int CREDITS_BACK_ID = 213;
    
    static const int TUTORIAL_FRAME_ID = 214;
    static const int TUTORIAL_PREV_BUTTON_ID = 215;
    static const int TUTORIAL_NEXT_BUTTON_ID = 216;
    static const int TUTORIAL_BACK_BUTTON_ID = 217;
    static const int TUTORIAL_PAGE_START_ID = 218;
    static const int TUTORIAL_PAGE_END_ID = 224;
    static const int TUTORIAL_PAGE_COUNT = 7;

public:
    GSMenu();
    ~GSMenu();

    void Init() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void HandleKeyEvent(unsigned char key, bool bIsPressed) override;
    void HandleMouseEvent(int x, int y, bool bIsPressed) override;
    void HandleMouseMove(int x, int y) override;
    void Resume() override;
    void Pause() override;
    void Exit() override;
    void Cleanup() override;

private:
    
    void ShowSettingsUI();
    void HideSettingsUI();
    void ToggleSettingsUI();
    void ShowCreditsUI();
    void HideCreditsUI();
    void ShowTutorialUI();
    void HideTutorialUI();
    void UpdateTutorialPage();
    
    void HandleMusicSliderDrag(int x, int y, bool bIsPressed);
    void UpdateMusicSliderPosition();
    void ApplyMusicVolume();
    
    void HandleSFXSliderDrag(int x, int y, bool bIsPressed);
    void UpdateSFXSliderPosition();
    void ApplySFXVolume();
}; 