#pragma once
#include "../GameObject/Model.h"
#include "../GameObject/Shaders.h"
#include "../../Utilities/utilities.h"
#include <vector>
#include <string>
#include <memory>

class Texture2D;

struct ModelData {
    int id;
    std::string filepath;
    std::shared_ptr<Model> model;
};

struct TextureData {
    int id;
    std::string filepath;
    std::string tiling;
    std::shared_ptr<Texture2D> texture;
};

struct ShaderData {
    int id;
    std::string vertexShaderPath;
    std::string fragmentShaderPath;
    std::shared_ptr<Shaders> shader;
};

class ResourceManager {
private:
    static ResourceManager* s_instance;
    
    std::vector<ModelData> m_models;
    std::vector<TextureData> m_textures;
    std::vector<ShaderData> m_shaders;
    
    ResourceManager() = default;
    
public:
    // Singleton access
    static ResourceManager* GetInstance();
    static void DestroyInstance();

    ~ResourceManager();
    
    // Load resources from RM.txt file
    bool LoadFromFile(const std::string& filepath);
    
    bool LoadModel(int id, const std::string& filepath);
    std::shared_ptr<Model> GetModel(int id);
    void ClearModels();

    bool LoadTexture(int id, const std::string& filepath, const std::string& tiling = "GL_REPEAT");
    std::shared_ptr<Texture2D> GetTexture(int id);
    void ClearTextures();

    bool LoadShader(int id, const std::string& vsPath, const std::string& fsPath);
    std::shared_ptr<Shaders> GetShader(int id);
    void ClearShaders();

    void Clear();

    void PrintLoadedResources();
}; 