#include "stdafx.h"
#include "ResourceManager.h"
#include "../GameObject/Texture2D.h"
#include <fstream>
#include <sstream>
#include <iostream>

ResourceManager* ResourceManager::s_instance = nullptr;

ResourceManager* ResourceManager::GetInstance() {
    if (!s_instance) {
        s_instance = new ResourceManager();
    }
    return s_instance;
}

void ResourceManager::DestroyInstance() {
    if (s_instance) {
        delete s_instance;
        s_instance = nullptr;
    }
}

ResourceManager::~ResourceManager() {
    Clear();
}

bool ResourceManager::LoadFromFile(const std::string& filepath) {
    char buffer[1000];
    if (GetCurrentDirectoryA(1000, buffer)) {
    }
    
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::ifstream testFile(filepath, std::ios::in);
        if (testFile.good()) {
        } else {
        }
        testFile.close();
        
        return false;
    }
    
    std::string line;
    std::string currentSection;
    
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        
        if (line[0] == '#') {
            if (line.find("#Model") != std::string::npos) {
                currentSection = "Model";
                continue;
            } else if (line.find("#2DTexture") != std::string::npos || line.find("# 2DTexture") != std::string::npos) {
                currentSection = "Texture";
                continue;
            } else if (line.find("#Shader") != std::string::npos || line.find("# Shader") != std::string::npos) {
                currentSection = "Shader";
                continue;
            }
            continue;
        }
        
        if (currentSection == "Model") {
            if (line.find("ID") == 0) {
                int id;
                sscanf_s(line.c_str(), "ID %d", &id);
                if (std::getline(file, line) && line.find("FILE") == 0) {
                    std::string filepath;
                    size_t start = line.find('"');
                    size_t end = line.rfind('"');
                    if (start != std::string::npos && end != std::string::npos && start < end) {
                        filepath = line.substr(start + 1, end - start - 1);
                        LoadModel(id, filepath);
                    }
                }
            }
        }
        else if (currentSection == "Texture") {
            if (line.find("ID") == 0) {
                int id;
                sscanf_s(line.c_str(), "ID %d", &id);
                
                std::string filepath, tiling = "GL_REPEAT";
                
                std::streampos currentPos = file.tellg();
                if (std::getline(file, line)) {
                    if (line.find("FILE") == 0) {
                        size_t start = line.find('"');
                        size_t end = line.rfind('"');
                        if (start != std::string::npos && end != std::string::npos && start < end) {
                            filepath = line.substr(start + 1, end - start - 1);
                        }
                    } else {
                        file.seekg(currentPos);
                    }
                }
                
                currentPos = file.tellg();
                if (std::getline(file, line)) {
                    if (line.find("TILING") == 0) {
                        std::istringstream ss(line);
                        std::string keyword;
                        ss >> keyword >> tiling;
                    } else {
                        file.seekg(currentPos);
                    }
                }
                
                int spriteWidth = 0, spriteHeight = 0;
                currentPos = file.tellg();
                if (std::getline(file, line)) {
                    if (line.find("SIZE") == 0) {
                        std::istringstream ss(line);
                        std::string keyword;
                        ss >> keyword >> spriteWidth >> spriteHeight;
                    } else {
                        file.seekg(currentPos);
                    }
                }
                
                std::vector<AnimationFrame> animations;
                currentPos = file.tellg();
                if (std::getline(file, line)) {
                    if (line.find("ANINUM") == 0) {
                        std::istringstream ss(line);
                        std::string keyword;
                        int numAnimations;
                        ss >> keyword >> numAnimations;
                        
                        for (int i = 0; i < numAnimations; i++) {
                            if (std::getline(file, line)) {
                                while (line.find("*") == 0 && std::getline(file, line)) {
                                }
                                
                                if (!line.empty() && line.find_first_of("0123456789") != std::string::npos) {
                                    int startFrame, numFrames, duration;
                                    std::istringstream animSS(line);
                                    animSS >> startFrame >> numFrames >> duration;
                                    animations.push_back({startFrame, numFrames, duration});
                                }
                            }
                        }
                    } else {
                        file.seekg(currentPos);
                    }
                }
                
                if (!filepath.empty()) {
                    LoadTexture(id, filepath, tiling, spriteWidth, spriteHeight, animations);
                }
                
                continue;
            }
        }
        else if (currentSection == "Shader") {
            if (line.find("ID") == 0) {
                int id;
                sscanf_s(line.c_str(), "ID %d", &id);
                
                std::string vsPath, fsPath;
                
                if (std::getline(file, line) && line.find("VS") == 0) {
                    size_t start = line.find('"');
                    size_t end = line.rfind('"');
                    if (start != std::string::npos && end != std::string::npos && start < end) {
                        vsPath = line.substr(start + 1, end - start - 1);
                    }
                }
                
                if (std::getline(file, line) && line.find("FS") == 0) {
                    size_t start = line.find('"');
                    size_t end = line.rfind('"');
                    if (start != std::string::npos && end != std::string::npos && start < end) {
                        fsPath = line.substr(start + 1, end - start - 1);
                    }
                }
                
                if (!vsPath.empty() && !fsPath.empty()) {
                    LoadShader(id, vsPath, fsPath);
                }
            }
        }
    }
    
    file.close();
    return true;
}

bool ResourceManager::LoadModel(int id, const std::string& filepath) {
    for (const auto& modelData : m_models) {
        if (modelData.id == id) {
            return false;
        }
    }
    
    auto model = std::make_shared<Model>();
    if (!model->LoadFromNFG(filepath.c_str())) {
        return false;
    }
    
    model->CreateBuffers();
    
    ModelData modelData;
    modelData.id = id;
    modelData.filepath = filepath;
    modelData.model = model;
    m_models.push_back(modelData);
    
    return true;
}

std::shared_ptr<Model> ResourceManager::GetModel(int id) {
    for (const auto& modelData : m_models) {
        if (modelData.id == id) {
            return modelData.model;
        }
    }
    return nullptr;
}

bool ResourceManager::LoadTexture(int id, const std::string& filepath, const std::string& tiling, 
                                int spriteWidth, int spriteHeight, const std::vector<AnimationFrame>& animations) {
    for (const auto& textureData : m_textures) {
        if (textureData.id == id) {
            return false;
        }
    }
    
    auto texture = std::make_shared<Texture2D>();
    if (!texture->LoadFromFile(filepath, tiling)) {
        return false;
    }
    
    if (id == 0 || id == 1) {
        texture->SetMixedFiltering();
    } else if (id >= 10 && id <= 11) {
        texture->SetSharpFiltering();
    } else if (id == 12) {
        texture->SetMixedFiltering();
    } else {
        texture->SetMixedFiltering();
    }
    
    TextureData textureData;
    textureData.id = id;
    textureData.filepath = filepath;
    textureData.tiling = tiling;
    textureData.texture = texture;
    textureData.spriteWidth = spriteWidth;
    textureData.spriteHeight = spriteHeight;
    textureData.animations = animations;
    m_textures.push_back(textureData);
    
    return true;
}

std::shared_ptr<Texture2D> ResourceManager::GetTexture(int id) {
    for (const auto& textureData : m_textures) {
        if (textureData.id == id) {
            return textureData.texture;
        }
    }
    return nullptr;
}

const TextureData* ResourceManager::GetTextureData(int id) {
    for (const auto& textureData : m_textures) {
        if (textureData.id == id) {
            return &textureData;
        }
    }
    return nullptr;
}

bool ResourceManager::LoadShader(int id, const std::string& vsPath, const std::string& fsPath) {
    for (const auto& shaderData : m_shaders) {
        if (shaderData.id == id) {
            return false;
        }
    }
    
    auto shader = std::make_shared<Shaders>();
    if (shader->Init((char*)vsPath.c_str(), (char*)fsPath.c_str()) != 0) {
        return false;
    }
    
    ShaderData shaderData;
    shaderData.id = id;
    shaderData.vertexShaderPath = vsPath;
    shaderData.fragmentShaderPath = fsPath;
    shaderData.shader = shader;
    m_shaders.push_back(shaderData);
    
    return true;
}

std::shared_ptr<Shaders> ResourceManager::GetShader(int id) {
    for (const auto& shaderData : m_shaders) {
        if (shaderData.id == id) {
            return shaderData.shader;
        }
    }
    return nullptr;
}

void ResourceManager::ClearModels() {
    m_models.clear();
}

void ResourceManager::ClearTextures() {
    m_textures.clear();
}

void ResourceManager::ClearShaders() {
    m_shaders.clear();
}

void ResourceManager::Clear() {
    ClearModels();
    ClearTextures();
    ClearShaders();
}

void ResourceManager::PrintLoadedResources() {
} 