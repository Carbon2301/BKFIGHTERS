#pragma once
#include <string>
#include <map>
struct _Mix_Music;
typedef struct _Mix_Music Mix_Music;
struct Mix_Chunk;
typedef struct Mix_Chunk Mix_Chunk;

class SoundManager {
public:
    static SoundManager& Instance();

    void LoadMusic(const std::string& name, const std::string& path);
    void PlayMusic(const std::string& name, int loop = -1);
    void StopMusic();
    void Clear();
    void LoadMusicFromFile(const std::string& rmFilePath);
    void LoadMusicByID(int id, const std::string& path);
    void PlayMusicByID(int id, int loop = -1);

    void LoadSFX(const std::string& name, const std::string& path);
    void LoadSFXByID(int id, const std::string& path);
    void PlaySFX(const std::string& name, int loops = 0);
    void PlaySFXByID(int id, int loops = 0);
    void PlaySFXOnChannel(int channel, const std::string& name, int loops = 0);
    void PlaySFXByIDOnChannel(int channel, int id, int loops = 0);

    void PreloadMusicByID(int id);
    void PreloadSFXByID(int id);
    void PreloadAllAudio(bool includeMusic = true, bool includeSfx = true);
    std::vector<int> GetAllAudioIDs() const;
    
    void StopAllChannels();
    int GetActiveChannelCount() const;
    int GetTotalChannelCount() const;
    void ReserveChannels(int numChannels);
    
    void Shutdown();

private:
    SoundManager();
    ~SoundManager();
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    std::map<std::string, Mix_Music*> m_musicMap;
    std::map<int, Mix_Music*> m_musicMapByID;
    std::map<std::string, Mix_Chunk*> m_sfxMap;
    std::map<int, Mix_Chunk*> m_sfxMapByID;
    std::map<int, std::string> m_audioPathByID;
    std::map<std::string, std::string> m_audioPathByName;
    Mix_Music* m_currentMusic = nullptr;
}; 