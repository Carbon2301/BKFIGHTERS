#include "stdafx.h"
#include "Texture2D.h"
#include "../../Utilities/TGA.h"
#include <iostream>
#include <SDL_surface.h>

#ifndef GL_BGRA
#define GL_BGRA 0x80E1
#endif

Texture2D::Texture2D() 
    : m_textureId(0), m_width(0), m_height(0), m_channels(0) {
}

Texture2D::~Texture2D() {
    Cleanup();
}

bool Texture2D::LoadFromFile(const std::string& filepath, const std::string& tiling) {
    // Cleanup existing texture
    Cleanup();
    
    // Load texture data using existing TGA loader
    char* textureData = LoadTGA(filepath.c_str(), &m_width, &m_height, &m_channels);
    if (!textureData) {
        std::cout << "Failed to load texture: " << filepath << std::endl;
        return false;
    }
    
    // Generate OpenGL texture
    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    
    // Determine format based on channels
    GLenum format = (m_channels == 24) ? GL_RGB : GL_RGBA;
    
    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 
                 0, format, GL_UNSIGNED_BYTE, textureData);
    
    // Set texture parameters based on tiling mode
    GLenum wrapMode = GL_REPEAT;
    if (tiling == "GL_CLAMP_TO_EDGE") {
        wrapMode = GL_CLAMP_TO_EDGE;
    } else if (tiling == "GL_MIRRORED_REPEAT") {
        wrapMode = GL_MIRRORED_REPEAT;
    }
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Store filepath and cleanup data
    m_filepath = filepath;
    delete[] textureData;
    
    std::cout << "Loaded texture: " << filepath << " (" << m_width << "x" << m_height << ", " << m_channels << " channels)" << std::endl;
    return true;
}

void Texture2D::Bind(int textureUnit) const {
    if (m_textureId) {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, m_textureId);
    }
}

void Texture2D::Unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::Cleanup() {
    if (m_textureId) {
        glDeleteTextures(1, &m_textureId);
        m_textureId = 0;
    }
    m_width = m_height = m_channels = 0;
    m_filepath.clear();
} 

// Tạo texture từ SDL_Surface
bool Texture2D::LoadFromSDLSurface(void* surfacePtr) {
    Cleanup();
    if (!surfacePtr) return false;
    SDL_Surface* surface = (SDL_Surface*)surfacePtr;

    // Convert surface về RGBA32
    SDL_Surface* converted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    if (!converted) return false;

    m_width = converted->w;
    m_height = converted->h;
    m_channels = 4;

    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, converted->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    SDL_FreeSurface(converted);
    return true;
}

bool Texture2D::CreateColorTexture(int width, int height, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    Cleanup();
    
    m_width = width;
    m_height = height;
    m_channels = 4; // RGBA
    
    // Create color data
    std::vector<unsigned char> textureData(width * height * 4);
    for (int i = 0; i < width * height; ++i) {
        textureData[i * 4 + 0] = r; // Red
        textureData[i * 4 + 1] = g; // Green
        textureData[i * 4 + 2] = b; // Blue
        textureData[i * 4 + 3] = a; // Alpha
    }
    
    // Generate OpenGL texture
    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    
    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 
                 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData.data());
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    std::cout << "Created color texture: " << width << "x" << height << " RGBA(" << (int)r << "," << (int)g << "," << (int)b << "," << (int)a << ")" << std::endl;
    return true;
} 