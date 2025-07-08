#pragma once
#include "Model.h"
#include "Shaders.h"
#include "../Utilities/utilities.h"
#include <vector>
#include <string>
#include <memory>

// Forward declarations
class Texture2D;

// Resource data structures
struct ModelData {
    int id;
    std::string filepath;
    std::shared_ptr<Model> model;
};

struct TextureData {
    int id;
    std::string filepath;
    std::string tiling; // GL_REPEAT, GL_CLAMP_TO_EDGE, etc.
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
    // Singleton instance
    static ResourceManager* s_instance;
    
    // Resource containers
    std::vector<ModelData> m_models;
    std::vector<TextureData> m_textures;
    std::vector<ShaderData> m_shaders;
    
    // Private constructor for singleton
    ResourceManager() = default;
    
public:
    // Singleton access
    static ResourceManager* GetInstance();
    static void DestroyInstance();
    
    // Destructor
    ~ResourceManager();
    
    // Load resources from RM.txt file
    bool LoadFromFile(const std::string& filepath);
    
    // Model management
    bool LoadModel(int id, const std::string& filepath);
    std::shared_ptr<Model> GetModel(int id);
    void ClearModels();
    
    // Texture management  
    bool LoadTexture(int id, const std::string& filepath, const std::string& tiling = "GL_REPEAT");
    std::shared_ptr<Texture2D> GetTexture(int id);
    void ClearTextures();
    
    // Shader management
    bool LoadShader(int id, const std::string& vsPath, const std::string& fsPath);
    std::shared_ptr<Shaders> GetShader(int id);
    void ClearShaders();
    
    // Clear all resources
    void Clear();
    
    // Debug
    void PrintLoadedResources();
}; 