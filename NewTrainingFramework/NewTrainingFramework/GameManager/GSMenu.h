#pragma once
#include "GameStateBase.h"
#include "SceneManager.h"
#include "../GameObject/Object.h"
#include "../GameObject/Texture2D.h"

class GSMenu : public GameStateBase {
private:
    float m_buttonTimer;
    bool m_isSettingsVisible;
    
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
    static const int BUTTON_ID_SETTINGS = 203;
    static const int BUTTON_ID_EXIT = 205;
    
    static const int SETTINGS_UI_ID = 206;
    static const int SETTINGS_MUSIC_ID = 207;
    static const int SETTINGS_SFX_ID = 208;
    static const int SETTINGS_APPLY_ID = 209;
    static const int SETTINGS_MUSIC_SLIDER_ID = 210;
    static const int SETTINGS_SFX_SLIDER_ID = 211;

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
    
    void HandleMusicSliderDrag(int x, int y, bool bIsPressed);
    void UpdateMusicSliderPosition();
    void ApplyMusicVolume();
    
    void HandleSFXSliderDrag(int x, int y, bool bIsPressed);
    void UpdateSFXSliderPosition();
    void ApplySFXVolume();
}; 