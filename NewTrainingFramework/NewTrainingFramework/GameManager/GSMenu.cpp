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
#include <SDL_mixer.h>
#include "SoundManager.h"
#include <algorithm>
#include <windows.h>

#ifndef MIX_MAX_VOLUME
#define MIX_MAX_VOLUME 128
#endif

float GSMenu::s_savedMusicVolume = 0.5f;
float GSMenu::s_savedSFXVolume = 0.5f;

GSMenu::GSMenu() 
    : GameStateBase(StateType::MENU), m_buttonTimer(0.0f), m_isSettingsVisible(false), m_isCreditsVisible(false), 
      m_isDraggingMusicSlider(false), m_musicVolume(s_savedMusicVolume),
      m_isDraggingSFXSlider(false), m_sfxVolume(s_savedSFXVolume) {
}

GSMenu::~GSMenu() {
}

void GSMenu::Init() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    SceneManager* sceneManager = SceneManager::GetInstance();
    sceneManager->LoadSceneForState(StateType::MENU);

    SoundManager::Instance().PlayMusicByID(0, -1);
    
    HideSettingsUI();
    HideCreditsUI();
    
    SceneManager* scene = SceneManager::GetInstance();
    if (Object* musicSlider = scene->GetObject(SETTINGS_MUSIC_SLIDER_ID)) {
        m_musicSliderOriginalPos = musicSlider->GetPosition();
    }
    
    if (Object* sfxSlider = scene->GetObject(SETTINGS_SFX_SLIDER_ID)) {
        m_sfxSliderOriginalPos = sfxSlider->GetPosition();
    }
    
    UpdateMusicSliderPosition();
    UpdateSFXSliderPosition();
    
    ApplyMusicVolume();
    ApplySFXVolume();
 
}

void GSMenu::Update(float deltaTime) {
    m_buttonTimer += deltaTime;
    
    SceneManager::GetInstance()->Update(deltaTime);
    
}

void GSMenu::Draw() {
    SceneManager::GetInstance()->Draw();
}

void GSMenu::HandleKeyEvent(unsigned char key, bool bIsPressed) {
    if (!bIsPressed) return;
    
    switch (key) {
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

void GSMenu::HandleMouseEvent(int x, int y, bool bIsPressed) {
    if (!bIsPressed && m_isDraggingMusicSlider) {
        m_isDraggingMusicSlider = false;
        return;
    }
    if (!bIsPressed && m_isDraggingSFXSlider) {
        m_isDraggingSFXSlider = false;
        return;
    }
    
    if (!bIsPressed) return;

    SceneManager* sceneManager = SceneManager::GetInstance();
    Camera* camera = sceneManager->GetActiveCamera();
    if (!camera) return;

    float worldX = MousePixelToWorldX(x, camera);
    float worldY = MousePixelToWorldY(y, camera);

    if (m_isSettingsVisible) {
        HandleMusicSliderDrag(x, y, bIsPressed);
        HandleSFXSliderDrag(x, y, bIsPressed);
        
        if (m_isDraggingMusicSlider || m_isDraggingSFXSlider) {
            return;
        }
        if (bIsPressed) {
            const auto& objects = sceneManager->GetObjects();
            for (const auto& objPtr : objects) {
                int id = objPtr->GetId();
                if (id == SETTINGS_APPLY_ID) {
                    const Vector3& pos = objPtr->GetPosition();
                    const Vector3& scale = objPtr->GetScale();
                    float width = scale.x;
                    float height = scale.y;
                    float leftBtn = pos.x - width / 2.0f;
                    float rightBtn = pos.x + width / 2.0f;
                    float bottomBtn = pos.y - height / 2.0f;
                    float topBtn = pos.y + height / 2.0f;
                    
                    if (bottomBtn > topBtn) {
                        std::swap(bottomBtn, topBtn);
                    }
                    
                    if (worldX >= leftBtn && worldX <= rightBtn && worldY >= bottomBtn && worldY <= topBtn) {
                        HideSettingsUI();
                        return;
                    }
                }
            }
        }
        return;
    }
    
    if (m_isCreditsVisible) {
        const auto& objects = sceneManager->GetObjects();
        for (const auto& objPtr : objects) {
            if (objPtr->GetId() == CREDITS_BACK_ID) {
                const Vector3& pos = objPtr->GetPosition();
                const Vector3& scale = objPtr->GetScale();
                float width = scale.x;
                float height = scale.y;
                float leftBtn = pos.x - width / 2.0f;
                float rightBtn = pos.x + width / 2.0f;
                float bottomBtn = pos.y - height / 2.0f;
                float topBtn = pos.y + height / 2.0f;
                if (bottomBtn > topBtn) std::swap(bottomBtn, topBtn);
                if (worldX >= leftBtn && worldX <= rightBtn && worldY >= bottomBtn && worldY <= topBtn) {
                    HideCreditsUI();
                }
                return;
            }
        }
        return;
    }

    const auto& objects = sceneManager->GetObjects();
    for (const auto& objPtr : objects) {
        int id = objPtr->GetId();
        if (id == BUTTON_ID_PLAY || id == BUTTON_ID_SETTINGS || id == BUTTON_ID_CREDITS || id == BUTTON_ID_EXIT) {
            const Vector3& pos = objPtr->GetPosition();
            const Vector3& scale = objPtr->GetScale();
            float width = scale.x;
            float height = scale.y;
            float leftBtn = pos.x - width / 2.0f;
            float rightBtn = pos.x + width / 2.0f;
            float bottomBtn = pos.y - height / 2.0f;
            float topBtn = pos.y + height / 2.0f;
            
            if (bottomBtn > topBtn) {
                std::swap(bottomBtn, topBtn);
            }
            
            if (worldX >= leftBtn && worldX <= rightBtn && worldY >= bottomBtn && worldY <= topBtn) {                
                if (id == BUTTON_ID_PLAY) {
                    GameStateMachine::GetInstance()->PushState(StateType::PLAY);
                } else if (id == BUTTON_ID_SETTINGS) {
                    ToggleSettingsUI();
                } else if (id == BUTTON_ID_CREDITS) {
                    ShowCreditsUI();
                } else if (id == BUTTON_ID_EXIT) {
                    Cleanup();
                    HWND hwnd = GetActiveWindow();
                    if (hwnd) {
                        PostMessage(hwnd, WM_CLOSE, 0, 0);
                    } else {
                        exit(0);
                    }
                }
                break;
            }
        }
    }
}

void GSMenu::HandleMouseMove(int x, int y) {
    if (m_isSettingsVisible) {
        if (m_isDraggingMusicSlider) {
            HandleMusicSliderDrag(x, y, false);
        }
        if (m_isDraggingSFXSlider) {
            HandleSFXSliderDrag(x, y, false);
        }
    }
}

void GSMenu::Resume() {
    
    m_musicVolume = s_savedMusicVolume;
    m_sfxVolume = s_savedSFXVolume;
    
    UpdateMusicSliderPosition();
    UpdateSFXSliderPosition();
    ApplyMusicVolume();
    ApplySFXVolume();
}

void GSMenu::Pause() {
    SoundManager::Instance().StopMusic();
}

void GSMenu::Exit() {
}

void GSMenu::Cleanup() {
}

void GSMenu::ShowSettingsUI() {
    SceneManager* scene = SceneManager::GetInstance();
    
    if (Object* settingsUI = scene->GetObject(SETTINGS_UI_ID)) {
        settingsUI->SetVisible(true);
    }
    if (Object* settingsMusic = scene->GetObject(SETTINGS_MUSIC_ID)) {
        settingsMusic->SetVisible(true);
    }
    if (Object* settingsSFX = scene->GetObject(SETTINGS_SFX_ID)) {
        settingsSFX->SetVisible(true);
    }
    if (Object* settingsApply = scene->GetObject(SETTINGS_APPLY_ID)) {
        settingsApply->SetVisible(false);
    }
    if (Object* settingsMusicSlider = scene->GetObject(SETTINGS_MUSIC_SLIDER_ID)) {
        settingsMusicSlider->SetVisible(true);
    }
    if (Object* settingsSFXSlider = scene->GetObject(SETTINGS_SFX_SLIDER_ID)) {
        settingsSFXSlider->SetVisible(true);
    }
    m_isSettingsVisible = true;
    
}

void GSMenu::HideSettingsUI() {
    SceneManager* scene = SceneManager::GetInstance();
    
    if (Object* settingsUI = scene->GetObject(SETTINGS_UI_ID)) {
        settingsUI->SetVisible(false);
    }
    if (Object* settingsMusic = scene->GetObject(SETTINGS_MUSIC_ID)) {
        settingsMusic->SetVisible(false);
    }
    if (Object* settingsSFX = scene->GetObject(SETTINGS_SFX_ID)) {
        settingsSFX->SetVisible(false);
    }
    if (Object* settingsApply = scene->GetObject(SETTINGS_APPLY_ID)) {
        settingsApply->SetVisible(false);
    }
    if (Object* settingsMusicSlider = scene->GetObject(SETTINGS_MUSIC_SLIDER_ID)) {
        settingsMusicSlider->SetVisible(false);
    }
    if (Object* settingsSFXSlider = scene->GetObject(SETTINGS_SFX_SLIDER_ID)) {
        settingsSFXSlider->SetVisible(false);
    }
    m_isSettingsVisible = false;
}

void GSMenu::ToggleSettingsUI() {
    ShowSettingsUI();
}

void GSMenu::ShowCreditsUI() {
    SceneManager* scene = SceneManager::GetInstance();
    if (Object* credits = scene->GetObject(CREDITS_UI_ID)) {
        credits->SetVisible(true);
    }
    if (Object* back = scene->GetObject(CREDITS_BACK_ID)) {
        back->SetVisible(true);
    }
    m_isCreditsVisible = true;
}

void GSMenu::HideCreditsUI() {
    SceneManager* scene = SceneManager::GetInstance();
    if (Object* credits = scene->GetObject(CREDITS_UI_ID)) {
        credits->SetVisible(false);
    }
    if (Object* back = scene->GetObject(CREDITS_BACK_ID)) {
        back->SetVisible(false);
    }
    m_isCreditsVisible = false;
}

void GSMenu::HandleMusicSliderDrag(int x, int y, bool bIsPressed) {
    SceneManager* scene = SceneManager::GetInstance();
    Camera* camera = scene->GetActiveCamera();
    if (!camera) return;

    float worldX = MousePixelToWorldX(x, camera);
    float worldY = MousePixelToWorldY(y, camera);

    Object* sliderTrack = scene->GetObject(SETTINGS_MUSIC_ID);
    Object* sliderThumb = scene->GetObject(SETTINGS_MUSIC_SLIDER_ID);
    
    if (!sliderTrack || !sliderThumb) return;

    const Vector3& trackPos = sliderTrack->GetPosition();
    const Vector3& trackScale = sliderTrack->GetScale();
    const Vector3& thumbPos = sliderThumb->GetPosition();
    const Vector3& thumbScale = sliderThumb->GetScale();
    
    float trackWidth = trackScale.x;
    float trackHeight = trackScale.y;
    float leftTrack = trackPos.x - trackWidth / 2.0f;
    float rightTrack = trackPos.x + trackWidth / 2.0f;
    float bottomTrack = trackPos.y - trackHeight / 2.0f;
    float topTrack = trackPos.y + trackHeight / 2.0f;
    
    if (bottomTrack > topTrack) {
        std::swap(bottomTrack, topTrack);
    }

    float thumbWidth = thumbScale.x;
    float thumbHeight = thumbScale.y;
    float leftThumb = thumbPos.x - thumbWidth / 2.0f;
    float rightThumb = thumbPos.x + thumbWidth / 2.0f;
    float bottomThumb = thumbPos.y - thumbHeight / 2.0f;
    float topThumb = thumbPos.y + thumbHeight / 2.0f;
    
    if (bottomThumb > topThumb) {
        std::swap(bottomThumb, topThumb);
    }

    if (bIsPressed) {
        if (worldX >= leftThumb && worldX <= rightThumb && 
            worldY >= bottomThumb && worldY <= topThumb) {
            m_isDraggingMusicSlider = true;
            
            float trackRange = rightTrack - leftTrack;
            float relativeX = worldX - leftTrack;
            m_musicVolume = relativeX / trackRange;
            
            if (m_musicVolume < 0.0f) m_musicVolume = 0.0f;
            if (m_musicVolume > 1.0f) m_musicVolume = 1.0f;
            
            UpdateMusicSliderPosition();
            ApplyMusicVolume();
            
        }
        else if (worldX >= leftTrack && worldX <= rightTrack && 
                 worldY >= bottomTrack && worldY <= topTrack) {
            float trackRange = rightTrack - leftTrack;
            float relativeX = worldX - leftTrack;
            m_musicVolume = relativeX / trackRange;
            
            if (m_musicVolume < 0.0f) m_musicVolume = 0.0f;
            if (m_musicVolume > 1.0f) m_musicVolume = 1.0f;
            
            UpdateMusicSliderPosition();
            ApplyMusicVolume();
        }
    }

    if (m_isDraggingMusicSlider) {
        float trackRange = rightTrack - leftTrack;
        float relativeX = worldX - leftTrack;
        m_musicVolume = relativeX / trackRange;
        
        if (m_musicVolume < 0.0f) m_musicVolume = 0.0f;
        if (m_musicVolume > 1.0f) m_musicVolume = 1.0f;
        
        UpdateMusicSliderPosition();
        ApplyMusicVolume();
    }
}

void GSMenu::UpdateMusicSliderPosition() {
    SceneManager* scene = SceneManager::GetInstance();
    Object* sliderTrack = scene->GetObject(SETTINGS_MUSIC_ID);
    Object* sliderThumb = scene->GetObject(SETTINGS_MUSIC_SLIDER_ID);
    
    if (!sliderTrack || !sliderThumb) return;

    const Vector3& trackPos = sliderTrack->GetPosition();
    const Vector3& trackScale = sliderTrack->GetScale();
    
    float trackWidth = trackScale.x;
    float leftTrack = trackPos.x - trackWidth / 2.0f;
    float rightTrack = trackPos.x + trackWidth / 2.0f;
    float trackRange = rightTrack - leftTrack;
    
    float newX = leftTrack + (trackRange * m_musicVolume);
    
    const Vector3& currentPos = sliderThumb->GetPosition();
    Vector3 newPos(currentPos.x, currentPos.y, currentPos.z);
    newPos.x = newX;
    sliderThumb->SetPosition(newPos);
}

void GSMenu::ApplyMusicVolume() {
    int volume = static_cast<int>(m_musicVolume * MIX_MAX_VOLUME);
    Mix_VolumeMusic(volume);
    
    s_savedMusicVolume = m_musicVolume;
}

void GSMenu::HandleSFXSliderDrag(int x, int y, bool bIsPressed) {
    SceneManager* scene = SceneManager::GetInstance();
    Camera* camera = scene->GetActiveCamera();
    if (!camera) return;

    float worldX = MousePixelToWorldX(x, camera);
    float worldY = MousePixelToWorldY(y, camera);

    Object* sliderTrack = scene->GetObject(SETTINGS_SFX_ID);
    Object* sliderThumb = scene->GetObject(SETTINGS_SFX_SLIDER_ID);
    
    if (!sliderTrack || !sliderThumb) return;

    const Vector3& trackPos = sliderTrack->GetPosition();
    const Vector3& trackScale = sliderTrack->GetScale();
    const Vector3& thumbPos = sliderThumb->GetPosition();
    const Vector3& thumbScale = sliderThumb->GetScale();
    
    float trackWidth = trackScale.x;
    float trackHeight = trackScale.y;
    float leftTrack = trackPos.x - trackWidth / 2.0f;
    float rightTrack = trackPos.x + trackWidth / 2.0f;
    float bottomTrack = trackPos.y - trackHeight / 2.0f;
    float topTrack = trackPos.y + trackHeight / 2.0f;
    
    if (bottomTrack > topTrack) {
        std::swap(bottomTrack, topTrack);
    }

    float thumbWidth = thumbScale.x;
    float thumbHeight = thumbScale.y;
    float leftThumb = thumbPos.x - thumbWidth / 2.0f;
    float rightThumb = thumbPos.x + thumbWidth / 2.0f;
    float bottomThumb = thumbPos.y - thumbHeight / 2.0f;
    float topThumb = thumbPos.y + thumbHeight / 2.0f;
    
    if (bottomThumb > topThumb) {
        std::swap(bottomThumb, topThumb);
    }

    if (bIsPressed) {
        if (worldX >= leftThumb && worldX <= rightThumb && 
            worldY >= bottomThumb && worldY <= topThumb) {
            m_isDraggingSFXSlider = true;
            
            float trackRange = rightTrack - leftTrack;
            float relativeX = worldX - leftTrack;
            m_sfxVolume = relativeX / trackRange;
            
            if (m_sfxVolume < 0.0f) m_sfxVolume = 0.0f;
            if (m_sfxVolume > 1.0f) m_sfxVolume = 1.0f;
            
            UpdateSFXSliderPosition();
            ApplySFXVolume();
        }
        else if (worldX >= leftTrack && worldX <= rightTrack && 
                 worldY >= bottomTrack && worldY <= topTrack) {
            float trackRange = rightTrack - leftTrack;
            float relativeX = worldX - leftTrack;
            m_sfxVolume = relativeX / trackRange;
            
            if (m_sfxVolume < 0.0f) m_sfxVolume = 0.0f;
            if (m_sfxVolume > 1.0f) m_sfxVolume = 1.0f;
            UpdateSFXSliderPosition();
            ApplySFXVolume();
            }
    }

    if (m_isDraggingSFXSlider) {
        float trackRange = rightTrack - leftTrack;
        float relativeX = worldX - leftTrack;
        m_sfxVolume = relativeX / trackRange;
        
        if (m_sfxVolume < 0.0f) m_sfxVolume = 0.0f;
        if (m_sfxVolume > 1.0f) m_sfxVolume = 1.0f;
        
        UpdateSFXSliderPosition();
        ApplySFXVolume();
    }
}

void GSMenu::UpdateSFXSliderPosition() {
    SceneManager* scene = SceneManager::GetInstance();
    Object* sliderTrack = scene->GetObject(SETTINGS_SFX_ID);
    Object* sliderThumb = scene->GetObject(SETTINGS_SFX_SLIDER_ID);
    
    if (!sliderTrack || !sliderThumb) return;

    const Vector3& trackPos = sliderTrack->GetPosition();
    const Vector3& trackScale = sliderTrack->GetScale();
    
    float trackWidth = trackScale.x;
    float leftTrack = trackPos.x - trackWidth / 2.0f;
    float rightTrack = trackPos.x + trackWidth / 2.0f;
    float trackRange = rightTrack - leftTrack;
    
    float newX = leftTrack + (trackRange * m_sfxVolume);
    
    const Vector3& currentPos = sliderThumb->GetPosition();
    Vector3 newPos(currentPos.x, currentPos.y, currentPos.z);
    newPos.x = newX;
    sliderThumb->SetPosition(newPos);
}

void GSMenu::ApplySFXVolume() {
    int volume = static_cast<int>(m_sfxVolume * MIX_MAX_VOLUME);
    for (int channel = 0; channel < 8; channel++) {
        Mix_Volume(channel, volume);
    }
    
    s_savedSFXVolume = m_sfxVolume;
} 