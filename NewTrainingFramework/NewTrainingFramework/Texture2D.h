#pragma once
#include "../Utilities/utilities.h"
#include <string>

class Texture2D {
private:
    GLuint m_textureId;
    int m_width;
    int m_height;
    int m_channels;
    std::string m_filepath;
    
public:
    Texture2D();
    ~Texture2D();
    
    // Load texture from file
    bool LoadFromFile(const std::string& filepath, const std::string& tiling = "GL_REPEAT");
    
    // Bind/Unbind texture
    void Bind(int textureUnit = 0) const;
    void Unbind() const;
    
    // Getters
    GLuint GetTextureId() const { return m_textureId; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    int GetChannels() const { return m_channels; }
    const std::string& GetFilepath() const { return m_filepath; }
    
    // Cleanup
    void Cleanup();
}; 