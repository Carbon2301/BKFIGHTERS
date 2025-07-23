#pragma once
#include <string>
#include <map>
struct _Mix_Music;
typedef struct _Mix_Music Mix_Music;

class SoundManager {
public:
    static SoundManager& Instance();

    void LoadMusic(const std::string& name, const std::string& path);
    void PlayMusic(const std::string& name, int loop = -1);
    void StopMusic();
    void Clear();
    void LoadMusicFromFile(const std::string& rmFilePath);

private:
    SoundManager();
    ~SoundManager();
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    std::map<std::string, Mix_Music*> m_musicMap;
    Mix_Music* m_currentMusic = nullptr;
}; 