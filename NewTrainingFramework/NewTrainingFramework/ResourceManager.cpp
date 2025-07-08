#include "stdafx.h"
#include "ResourceManager.h"
#include "Texture2D.h"
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
        std::cout << "Current working directory: " << buffer << std::endl;
    }
    std::cout << "Looking for config file: " << filepath << std::endl;
    
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cout << "Cannot open ResourceManager file: " << filepath << std::endl;
        
        std::ifstream testFile(filepath, std::ios::in);
        if (testFile.good()) {
            std::cout << "File exists but cannot be opened properly" << std::endl;
        } else {
            std::cout << "File does not exist or is not accessible" << std::endl;
        }
        testFile.close();
        
        return false;
    }
    
    std::cout << "Loading resources from: " << filepath << std::endl;
    
    std::string line;
    std::string currentSection;
    
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
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
        
        // Parse based on current section
        if (currentSection == "Model") {
            if (line.find("ID") == 0) {
                int id;
                sscanf(line.c_str(), "ID %d", &id);
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
                sscanf(line.c_str(), "ID %d", &id);
                
                std::string filepath, tiling = "GL_REPEAT";
                if (std::getline(file, line) && line.find("FILE") == 0) {
                    size_t start = line.find('"');
                    size_t end = line.rfind('"');
                    if (start != std::string::npos && end != std::string::npos && start < end) {
                        filepath = line.substr(start + 1, end - start - 1);
                    }
                }
                if (std::getline(file, line) && line.find("TILING") == 0) {
                    std::istringstream ss(line);
                    std::string keyword;
                    ss >> keyword >> tiling;
                }
                if (!filepath.empty()) {
                    LoadTexture(id, filepath, tiling);
                }
            }
        }
        else if (currentSection == "Shader") {
            if (line.find("ID") == 0) {
                int id;
                sscanf(line.c_str(), "ID %d", &id);
                
                std::string vsPath, fsPath;
                
                // Read VS line
                if (std::getline(file, line) && line.find("VS") == 0) {
                    size_t start = line.find('"');
                    size_t end = line.rfind('"');
                    if (start != std::string::npos && end != std::string::npos && start < end) {
                        vsPath = line.substr(start + 1, end - start - 1);
                    }
                }
                
                // Read FS line
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
    PrintLoadedResources();
    return true;
}

bool ResourceManager::LoadModel(int id, const std::string& filepath) {
    // Check if model with this ID already exists
    for (const auto& modelData : m_models) {
        if (modelData.id == id) {
            std::cout << "Model with ID " << id << " already exists!" << std::endl;
            return false;
        }
    }
    
    // Create new model
    auto model = std::make_shared<Model>();
    if (!model->LoadFromNFG(filepath.c_str())) {
        std::cout << "Failed to load model: " << filepath << std::endl;
        return false;
    }
    
    model->CreateBuffers();
    
    // Store model data
    ModelData modelData;
    modelData.id = id;
    modelData.filepath = filepath;
    modelData.model = model;
    m_models.push_back(modelData);
    
    std::cout << "Loaded model ID " << id << ": " << filepath << std::endl;
    return true;
}

std::shared_ptr<Model> ResourceManager::GetModel(int id) {
    for (const auto& modelData : m_models) {
        if (modelData.id == id) {
            return modelData.model;
        }
    }
    std::cout << "Model with ID " << id << " not found!" << std::endl;
    return nullptr;
}

bool ResourceManager::LoadTexture(int id, const std::string& filepath, const std::string& tiling) {
    // Check if texture with this ID already exists
    for (const auto& textureData : m_textures) {
        if (textureData.id == id) {
            std::cout << "Texture with ID " << id << " already exists!" << std::endl;
            return false;
        }
    }
    
    // Create new texture
    auto texture = std::make_shared<Texture2D>();
    if (!texture->LoadFromFile(filepath, tiling)) {
        std::cout << "Failed to load texture: " << filepath << std::endl;
        return false;
    }
    
    // Store texture data
    TextureData textureData;
    textureData.id = id;
    textureData.filepath = filepath;
    textureData.tiling = tiling;
    textureData.texture = texture;
    m_textures.push_back(textureData);
    
    std::cout << "Loaded texture ID " << id << ": " << filepath << std::endl;
    return true;
}

std::shared_ptr<Texture2D> ResourceManager::GetTexture(int id) {
    for (const auto& textureData : m_textures) {
        if (textureData.id == id) {
            return textureData.texture;
        }
    }
    std::cout << "Texture with ID " << id << " not found!" << std::endl;
    return nullptr;
}

bool ResourceManager::LoadShader(int id, const std::string& vsPath, const std::string& fsPath) {
    // Check if shader with this ID already exists
    for (const auto& shaderData : m_shaders) {
        if (shaderData.id == id) {
            std::cout << "Shader with ID " << id << " already exists!" << std::endl;
            return false;
        }
    }
    
    // Create new shader
    auto shader = std::make_shared<Shaders>();
    if (shader->Init((char*)vsPath.c_str(), (char*)fsPath.c_str()) != 0) {
        std::cout << "Failed to load shader: " << vsPath << ", " << fsPath << std::endl;
        return false;
    }
    
    // Store shader data
    ShaderData shaderData;
    shaderData.id = id;
    shaderData.vertexShaderPath = vsPath;
    shaderData.fragmentShaderPath = fsPath;
    shaderData.shader = shader;
    m_shaders.push_back(shaderData);
    
    std::cout << "Loaded shader ID " << id << ": " << vsPath << ", " << fsPath << std::endl;
    return true;
}

std::shared_ptr<Shaders> ResourceManager::GetShader(int id) {
    for (const auto& shaderData : m_shaders) {
        if (shaderData.id == id) {
            return shaderData.shader;
        }
    }
    std::cout << "Shader with ID " << id << " not found!" << std::endl;
    return nullptr;
}

void ResourceManager::ClearModels() {
    m_models.clear();
    std::cout << "Cleared all models" << std::endl;
}

void ResourceManager::ClearTextures() {
    m_textures.clear();
    std::cout << "Cleared all textures" << std::endl;
}

void ResourceManager::ClearShaders() {
    m_shaders.clear();
    std::cout << "Cleared all shaders" << std::endl;
}

void ResourceManager::Clear() {
    ClearModels();
    ClearTextures();
    ClearShaders();
}

void ResourceManager::PrintLoadedResources() {
    std::cout << "\n=== Resource Manager Status ===" << std::endl;
    std::cout << "Models: " << m_models.size() << std::endl;
    for (const auto& model : m_models) {
        std::cout << "  - ID " << model.id << ": " << model.filepath << std::endl;
    }
    
    std::cout << "Textures: " << m_textures.size() << std::endl;
    for (const auto& texture : m_textures) {
        std::cout << "  - ID " << texture.id << ": " << texture.filepath << " (" << texture.tiling << ")" << std::endl;
    }
    
    std::cout << "Shaders: " << m_shaders.size() << std::endl;
    for (const auto& shader : m_shaders) {
        std::cout << "  - ID " << shader.id << ": " << shader.vertexShaderPath << ", " << shader.fragmentShaderPath << std::endl;
    }
    std::cout << "==============================\n" << std::endl;
} 