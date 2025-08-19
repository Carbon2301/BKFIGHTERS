#include "stdafx.h"
#include "SoundManager.h"
#include <SDL_mixer.h>
#include <SDL.h>
#include <iostream>
#include <fstream>
#include <sstream>

SoundManager::SoundManager() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cout << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
    }

    int numChannels = Mix_AllocateChannels(32);
    ReserveChannels(4);
}

SoundManager::~SoundManager() {
    StopAllChannels();
    StopMusic();
    SDL_Delay(100);
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
            m_audioPathByName[name] = path;
        } else {
            std::cout << "Failed to load music: " << path << " Error: " << Mix_GetError() << std::endl;
        }
    }
}

void SoundManager::PlayMusic(const std::string& name, int loop) {
    Mix_Music* music = nullptr;
    auto it = m_musicMap.find(name);
    if (it != m_musicMap.end()) {
        music = it->second;
    } else {
        auto itPath = m_audioPathByName.find(name);
        if (itPath != m_audioPathByName.end()) {
            music = Mix_LoadMUS(itPath->second.c_str());
            if (music) {
                m_musicMap[name] = music;
            }
        }
    }
    if (music) {
        m_currentMusic = music;
        if (Mix_PlayMusic(m_currentMusic, loop) == -1) {
            std::cout << "Failed to play music: " << Mix_GetError() << std::endl;
        }
    } else {
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
            m_audioPathByID[id] = path;
        } else {
            std::cout << "Failed to load music: " << path << " Error: " << Mix_GetError() << std::endl;
        }
    }
}

void SoundManager::PlayMusicByID(int id, int loop) {
    Mix_Music* music = nullptr;
    auto it = m_musicMapByID.find(id);
    if (it != m_musicMapByID.end()) {
        music = it->second;
    } else {
        auto itPath = m_audioPathByID.find(id);
        if (itPath != m_audioPathByID.end()) {
            music = Mix_LoadMUS(itPath->second.c_str());
            if (music) {
                m_musicMapByID[id] = music;
            }
        }
    }
    if (music) {
        m_currentMusic = music;
        if (Mix_PlayMusic(m_currentMusic, loop) == -1) {
            std::cout << "Failed to play music: " << Mix_GetError() << std::endl;
        }
    } else {
    }
}

void SoundManager::LoadSFX(const std::string& name, const std::string& path) {
    if (m_sfxMap.count(name) == 0) {
        Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
        if (chunk) {
            m_sfxMap[name] = chunk;
            m_audioPathByName[name] = path;
        } else {
            std::cout << "Failed to load SFX: " << path << " Error: " << Mix_GetError() << std::endl;
        }
    }
}

void SoundManager::LoadSFXByID(int id, const std::string& path) {
    if (m_sfxMapByID.count(id) == 0) {
        Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
        if (chunk) {
            m_sfxMapByID[id] = chunk;
            m_audioPathByID[id] = path;
        } else {
            std::cout << "Failed to load SFX: " << path << " Error: " << Mix_GetError() << std::endl;
        }
    }
}

void SoundManager::PlaySFX(const std::string& name, int loops) {
    Mix_Chunk* chunk = nullptr;
    auto it = m_sfxMap.find(name);
    if (it != m_sfxMap.end()) {
        chunk = it->second;
    } else {
        auto itPath = m_audioPathByName.find(name);
        if (itPath != m_audioPathByName.end()) {
            chunk = Mix_LoadWAV(itPath->second.c_str());
            if (chunk) {
                m_sfxMap[name] = chunk;
            }
        }
    }
    if (chunk) {
        int channel = Mix_PlayChannel(-1, chunk, loops);
    } else {
        std::cout << "SFX not found: " << name << std::endl;
    }
}

void SoundManager::PlaySFXByID(int id, int loops) {
    Mix_Chunk* chunk = nullptr;
    auto it = m_sfxMapByID.find(id);
    if (it != m_sfxMapByID.end()) {
        chunk = it->second;
    } else {
        auto itPath = m_audioPathByID.find(id);
        if (itPath != m_audioPathByID.end()) {
            chunk = Mix_LoadWAV(itPath->second.c_str());
            if (chunk) {
                m_sfxMapByID[id] = chunk;
            }
        }
    }
    if (chunk) {
        int channel = Mix_PlayChannel(-1, chunk, loops);
    } else {
        std::cout << "SFX not found with ID: " << id << std::endl;
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
    for (auto& pair : m_sfxMap) {
        if (pair.second) {
            Mix_FreeChunk(pair.second);
        }
    }
    m_sfxMap.clear();
    for (auto& pair : m_sfxMapByID) {
        if (pair.second) {
            Mix_FreeChunk(pair.second);
        }
    }
    m_sfxMapByID.clear();
    m_audioPathByID.clear();
    m_audioPathByName.clear();
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
                m_audioPathByID[id] = path;
                if (!name.empty()) m_audioPathByName[name] = path;
                id = -1;
                path.clear();
                name.clear();
            }
        }
    }
} 

void SoundManager::PreloadMusicByID(int id) {
    if (m_musicMapByID.count(id)) return;
    auto itPath = m_audioPathByID.find(id);
    if (itPath == m_audioPathByID.end()) return;
    Mix_Music* music = Mix_LoadMUS(itPath->second.c_str());
    if (music) {
        m_musicMapByID[id] = music;
    }
}

void SoundManager::PreloadSFXByID(int id) {
    if (m_sfxMapByID.count(id)) return;
    auto itPath = m_audioPathByID.find(id);
    if (itPath == m_audioPathByID.end()) return;
    Mix_Chunk* chunk = Mix_LoadWAV(itPath->second.c_str());
    if (chunk) {
        m_sfxMapByID[id] = chunk;
    }
}

std::vector<int> SoundManager::GetAllAudioIDs() const {
    std::vector<int> ids;
    ids.reserve(m_audioPathByID.size());
    for (const auto& kv : m_audioPathByID) ids.push_back(kv.first);
    std::sort(ids.begin(), ids.end());
    return ids;
}
void SoundManager::PreloadAllAudio(bool includeMusic, bool includeSfx) {
    for (const auto& kv : m_audioPathByID) {
        int id = kv.first;
        if (includeMusic) PreloadMusicByID(id);
        if (includeSfx) PreloadSFXByID(id);
    }
}

void SoundManager::StopAllChannels() {
    Mix_HaltChannel(-1);
    SDL_Delay(50);

    int activeChannels = Mix_Playing(-1);
    if (activeChannels > 0) {
        Mix_HaltChannel(-1);
        SDL_Delay(50);
    }
}

int SoundManager::GetActiveChannelCount() const {
    return Mix_Playing(-1);
}

int SoundManager::GetTotalChannelCount() const {
    return Mix_AllocateChannels(-1);
}

void SoundManager::ReserveChannels(int numChannels) {
    Mix_ReserveChannels(numChannels);
}

void SoundManager::PlaySFXOnChannel(int channel, const std::string& name, int loops) {
    Mix_Chunk* chunk = nullptr;
    auto it = m_sfxMap.find(name);
    if (it != m_sfxMap.end()) {
        chunk = it->second;
    } else {
        auto itPath = m_audioPathByName.find(name);
        if (itPath != m_audioPathByName.end()) {
            chunk = Mix_LoadWAV(itPath->second.c_str());
            if (chunk) {
                m_sfxMap[name] = chunk;
            }
        }
    }
}

void SoundManager::PlaySFXByIDOnChannel(int channel, int id, int loops) {
    Mix_Chunk* chunk = nullptr;
    auto it = m_sfxMapByID.find(id);
    if (it != m_sfxMapByID.end()) {
        chunk = it->second;
    } else {
        auto itPath = m_audioPathByID.find(id);
        if (itPath != m_audioPathByID.end()) {
            chunk = Mix_LoadWAV(itPath->second.c_str());
            if (chunk) {
                m_sfxMapByID[id] = chunk;
            }
        }
    }
}

void SoundManager::Shutdown() {
    StopAllChannels();
    StopMusic();
    SDL_Delay(200);
    Clear();
    Mix_CloseAudio();
}

