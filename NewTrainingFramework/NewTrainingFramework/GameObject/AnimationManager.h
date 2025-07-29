#pragma once
#include <vector>
#include <string>
#include <memory>

struct AnimationData {
    int startFrame;
    int numFrames;
    int duration; // milliseconds
    float frameTime; // calculated from duration
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
    
    // Initialize with animation data
    void Initialize(int spriteWidth, int spriteHeight, const std::vector<AnimationData>& animations);
    
    // Animation control
    void Play(int animationIndex, bool loop = true);
    void Stop();
    void Pause();
    void Resume();
    
    // Update animation
    void Update(float deltaTime);
    
    // Get current frame info
    int GetCurrentAnimation() const { return m_currentAnimation; }
    int GetCurrentFrame() const { return m_currentFrame; }
    bool IsPlaying() const { return m_isPlaying; }
    
    // Get UV coordinates for current frame
    void GetUV(float& u0, float& v0, float& u1, float& v1) const;
    
    // Get animation info
    int GetAnimationCount() const { return m_animations.size(); }
    const AnimationData* GetAnimation(int index) const;
}; 