#pragma once
#include <vector>
#include <string>
#include <memory>

struct AnimationData {
    int startFrame;
    int numFrames;
    int duration;
    float frameTime;
};

class AnimationManager {
private:
    int m_spriteWidth;
    int m_spriteHeight;
    std::vector<AnimationData> m_animations;
    
    int m_currentAnimation;
    int m_currentFrame;
    float m_timer;
    bool m_isLooping;
    bool m_isPlaying;

public:
    AnimationManager();
    ~AnimationManager();
    
    void Initialize(int spriteWidth, int spriteHeight, const std::vector<AnimationData>& animations);
    
    void Play(int animationIndex, bool loop = true);
    void Stop();
    void Pause();
    void Resume();
    void SetPlaying(bool playing) { m_isPlaying = playing; }
    
    void Update(float deltaTime);
    
    int GetCurrentAnimation() const { return m_currentAnimation; }
    int GetCurrentFrame() const { return m_currentFrame; }
    void SetCurrentFrame(int frame) { m_currentFrame = frame; }
    bool IsPlaying() const { return m_isPlaying; }
    
    void GetUV(float& u0, float& v0, float& u1, float& v1) const;
    
    int GetAnimationCount() const { return m_animations.size(); }
    const AnimationData* GetAnimation(int index) const;
}; 