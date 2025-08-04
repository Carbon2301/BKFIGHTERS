#include "stdafx.h"
#include "SoundManager.h"
#include <SDL_mixer.h>
#include <iostream>
#include <fstream>
#include <sstream>

SoundManager::SoundManager() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cout << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
    }
}

SoundManager::~SoundManager() {
    Clear();
    Mix_CloseAudio();
}

SoundManager& SoundManager::Instance() {
    static SoundManager instance;
    return instance;
}

void SoundManager::LoadMusic(const std::string& name, const std::string& path) {
    if (m_musicMap.count(name) == 0) {
        Mix_Music* music = Mix_LoadMUS(path.c_str());
        if (music) {
            m_musicMap[name] = music;
        } else {
            std::cout << "Failed to load music: " << path << " Error: " << Mix_GetError() << std::endl;
        }
    }
}

void SoundManager::PlayMusic(const std::string& name, int loop) {
    auto it = m_musicMap.find(name);
    if (it != m_musicMap.end()) {
        m_currentMusic = it->second;
        if (Mix_PlayMusic(m_currentMusic, loop) == -1) {
            std::cout << "Failed to play music: " << Mix_GetError() << std::endl;
        }
    } else {
        std::cout << "Music not found: " << name << std::endl;
    }
}

void SoundManager::StopMusic() {
    Mix_HaltMusic();
    m_currentMusic = nullptr;
}

void SoundManager::LoadMusicByID(int id, const std::string& path) {
    if (m_musicMapByID.count(id) == 0) {
        Mix_Music* music = Mix_LoadMUS(path.c_str());
        if (music) {
            m_musicMapByID[id] = music;
        } else {
            std::cout << "Failed to load music: " << path << " Error: " << Mix_GetError() << std::endl;
        }
    }
}

void SoundManager::PlayMusicByID(int id, int loop) {
    auto it = m_musicMapByID.find(id);
    if (it != m_musicMapByID.end()) {
        m_currentMusic = it->second;
        if (Mix_PlayMusic(m_currentMusic, loop) == -1) {
            std::cout << "Failed to play music: " << Mix_GetError() << std::endl;
        }
    } else {
        std::cout << "Music not found with ID: " << id << std::endl;
    }
}

void SoundManager::Clear() {
    for (auto& pair : m_musicMap) {
        if (pair.second) {
            Mix_FreeMusic(pair.second);
        }
    }
    m_musicMap.clear();
    for (auto& pair : m_musicMapByID) {
        if (pair.second) {
            Mix_FreeMusic(pair.second);
        }
    }
    m_musicMapByID.clear();
    m_currentMusic = nullptr;
}

void SoundManager::LoadMusicFromFile(const std::string& rmFilePath) {
    std::ifstream file(rmFilePath);
    if (!file.is_open()) {
        std::cout << "Failed to open RM.txt for music loading\n";
        return;
    }
    std::string line;
    int id = -1;
    std::string path;
    std::string name;
    bool inMusicSection = false;
    
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            if (line.find("# Music") != std::string::npos) {
                inMusicSection = true;
            } else if (line.find("# ") != std::string::npos) {
                inMusicSection = false;
            }
            continue;
        }
        
        if (!inMusicSection) continue;
        
        if (line.find("ID") == 0) {
            id = std::stoi(line.substr(3));
        }
        if (line.find("NAME") == 0) {
            size_t firstQuote = line.find('"');
            size_t lastQuote = line.find_last_of('"');
            if (firstQuote != std::string::npos && lastQuote != std::string::npos && lastQuote > firstQuote) {
                name = line.substr(firstQuote + 1, lastQuote - firstQuote - 1);
            }
        }
        if (line.find("FILE") == 0) {
            size_t firstQuote = line.find('"');
            size_t lastQuote = line.find_last_of('"');
            if (firstQuote != std::string::npos && lastQuote != std::string::npos && lastQuote > firstQuote) {
                path = line.substr(firstQuote + 1, lastQuote - firstQuote - 1);
            }
            if (id != -1 && !path.empty()) {
                LoadMusicByID(id, path);
                if (!name.empty()) {
                    LoadMusic(name, path);
                }
                id = -1;
                path.clear();
                name.clear();
            }
        }
    }
} 