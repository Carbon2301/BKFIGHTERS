#include "stdafx.h"
#include "SceneManager.h"
#include "Globals.h"
#include <fstream>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Static member initialization
SceneManager* SceneManager::s_instance = nullptr;

SceneManager* SceneManager::GetInstance() {
    if (!s_instance) {
        s_instance = new SceneManager();
    }
    return s_instance;
}

void SceneManager::DestroyInstance() {
    if (s_instance) {
        delete s_instance;
        s_instance = nullptr;
    }
}

SceneManager::SceneManager() 
    : m_activeCameraIndex(-1) {
    // Create default camera
    CreateCamera();
    m_activeCameraIndex = 0;
}

SceneManager::~SceneManager() {
    Clear();
}

bool SceneManager::LoadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cout << "❌ Cannot open SceneManager file: " << filepath << std::endl;
        return false;
    }
    
    std::cout << "🎬 Loading scene from: " << filepath << std::endl;
    
    // Clear existing scene
    RemoveAllObjects();
    
    std::string line;
    int objectCount = 0;
    
    // Read object count
    while (std::getline(file, line)) {
        if (line.find("#ObjectCount") != std::string::npos) {
            if (std::getline(file, line)) {
                objectCount = std::stoi(line);
                std::cout << "🔹 Loading " << objectCount << " objects..." << std::endl;
            }
            break;
        }
    }
    
    // Load objects
    for (int i = 0; i < objectCount; ++i) {
        Object* obj = nullptr;
        int id = -1;
        int modelId = -1, shaderId = -1;
        std::vector<int> textureIds;
        Vector3 position(0, 0, 0), rotation(0, 0, 0), scale(1, 1, 1);
        
        // Read object data
        while (std::getline(file, line)) {
            if (line.find("ID") == 0) {
                sscanf(line.c_str(), "ID %d", &id);
                obj = CreateObject(id);
            }
            else if (line.find("MODEL_ID") == 0) {
                sscanf(line.c_str(), "MODEL_ID %d", &modelId);
            }
            else if (line.find("TEXTURE_ID") == 0) {
                int textureId;
                sscanf(line.c_str(), "TEXTURE_ID %d", &textureId);
                textureIds.push_back(textureId);
            }
            else if (line.find("SHADER_ID") == 0) {
                sscanf(line.c_str(), "SHADER_ID %d", &shaderId);
            }
            else if (line.find("POS") == 0) {
                sscanf(line.c_str(), "POS %f %f %f", &position.x, &position.y, &position.z);
            }
            else if (line.find("ROTATION") == 0) {
                sscanf(line.c_str(), "ROTATION %f %f %f", &rotation.x, &rotation.y, &rotation.z);
            }
            else if (line.find("SCALE") == 0) {
                sscanf(line.c_str(), "SCALE %f %f %f", &scale.x, &scale.y, &scale.z);
            }
            else if (line.empty() || line.find("ID") == 0) {
                // End of current object or start of next object
                break;
            }
        }
        
        // Setup object if created successfully
        if (obj) {
            obj->SetPosition(position);
            obj->SetRotation(rotation);
            obj->SetScale(scale);
            
            if (modelId >= 0) {
                obj->SetModel(modelId);
            }
            
            for (int j = 0; j < (int)textureIds.size(); ++j) {
                obj->SetTexture(textureIds[j], j);
            }
            
            if (shaderId >= 0) {
                obj->SetShader(shaderId);
            }
            
            std::cout << "✅ Created object ID " << id << std::endl;
        }
        
        // If we hit start of next object, put the line back for next iteration
        if (line.find("ID") == 0 && i < objectCount - 1) {
            file.seekg(-((long)line.length() + 1), std::ios::cur);
        }
    }
    
    file.close();
    PrintSceneInfo();
    return true;
}

Object* SceneManager::CreateObject(int id) {
    auto obj = std::make_unique<Object>(id);
    Object* objPtr = obj.get();
    m_objects.push_back(std::move(obj));
    return objPtr;
}

Object* SceneManager::GetObject(int id) {
    for (auto& obj : m_objects) {
        if (obj->GetId() == id) {
            return obj.get();
        }
    }
    return nullptr;
}

void SceneManager::RemoveObject(int id) {
    auto it = std::remove_if(m_objects.begin(), m_objects.end(),
        [id](const std::unique_ptr<Object>& obj) {
            return obj->GetId() == id;
        });
    
    if (it != m_objects.end()) {
        m_objects.erase(it, m_objects.end());
        std::cout << "🗑️ Removed object ID " << id << std::endl;
    }
}

void SceneManager::RemoveAllObjects() {
    m_objects.clear();
    std::cout << "🗑️ Removed all objects" << std::endl;
}

Camera* SceneManager::CreateCamera() {
    auto camera = std::make_unique<Camera>();
    // Set default camera for our screen resolution
    float aspect = (float)Globals::screenWidth / (float)Globals::screenHeight;
    camera->SetPerspective(45.0f * (float)M_PI / 180.0f, aspect, 0.1f, 100.0f);
    camera->SetLookAt(Vector3(0.0f, 0.5f, 3.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
    
    Camera* cameraPtr = camera.get();
    m_cameras.push_back(std::move(camera));
    return cameraPtr;
}

Camera* SceneManager::GetActiveCamera() {
    if (m_activeCameraIndex >= 0 && m_activeCameraIndex < (int)m_cameras.size()) {
        return m_cameras[m_activeCameraIndex].get();
    }
    return nullptr;
}

Camera* SceneManager::GetCamera(int index) {
    if (index >= 0 && index < (int)m_cameras.size()) {
        return m_cameras[index].get();
    }
    return nullptr;
}

void SceneManager::SetActiveCamera(int index) {
    if (index >= 0 && index < (int)m_cameras.size()) {
        m_activeCameraIndex = index;
        std::cout << "📹 Switched to camera " << index << std::endl;
    }
}

void SceneManager::Update(float deltaTime) {
    // Update all objects
    for (auto& obj : m_objects) {
        obj->Update(deltaTime);
    }
}

void SceneManager::Draw() {
    Camera* activeCamera = GetActiveCamera();
    if (!activeCamera) {
        return;
    }
    
    const Matrix& viewMatrixRef = activeCamera->GetViewMatrix();
    const Matrix& projMatrixRef = activeCamera->GetProjectionMatrix();
    Matrix viewMatrix;
    Matrix projectionMatrix;
    viewMatrix = const_cast<Matrix&>(viewMatrixRef);
    projectionMatrix = const_cast<Matrix&>(projMatrixRef);
    
    // Draw all objects
    for (auto& obj : m_objects) {
        obj->Draw(viewMatrix, projectionMatrix);
    }
}

void SceneManager::HandleInput(unsigned char key, bool isPressed) {
    if (!isPressed) return;
    
    Camera* activeCamera = GetActiveCamera();
    if (!activeCamera) return;
    
    float moveSpeed = 0.1f;
    
    switch(key) {
        // Camera movement
        case 'W':
        case 'w':
            activeCamera->MoveForward(-moveSpeed);
            std::cout << "Camera moved forward" << std::endl;
            break;
        case 'S':
        case 's':
            activeCamera->MoveForward(moveSpeed);
            std::cout << "Camera moved backward" << std::endl;
            break;
        case 'A':
        case 'a':
            activeCamera->MoveRight(-moveSpeed);
            std::cout << "Camera moved left" << std::endl;
            break;
        case 'D':
        case 'd':
            activeCamera->MoveRight(moveSpeed);
            std::cout << "Camera moved right" << std::endl;
            break;
        case 'Q':
        case 'q':
            activeCamera->MoveUp(moveSpeed);
            std::cout << "Camera moved up" << std::endl;
            break;
        case 'E':
        case 'e':
            activeCamera->MoveUp(-moveSpeed);
            std::cout << "Camera moved down" << std::endl;
            break;
        case 'R':
        case 'r':
            std::cout << "=== Scene Info ===" << std::endl;
            std::cout << "Camera Position: (" << activeCamera->GetPosition().x << ", " << activeCamera->GetPosition().y << ", " << activeCamera->GetPosition().z << ")" << std::endl;
            std::cout << "Objects: " << m_objects.size() << std::endl;
            std::cout << "Controls: WASD=move, QE=up/down, R=info" << std::endl;
            break;
    }
}

void SceneManager::Clear() {
    RemoveAllObjects();
    m_cameras.clear();
    m_activeCameraIndex = -1;
}

void SceneManager::PrintSceneInfo() {
    std::cout << "\n=== Scene Manager Status ===" << std::endl;
    std::cout << "📦 Objects: " << m_objects.size() << std::endl;
    for (const auto& obj : m_objects) {
        std::cout << "  - Object ID " << obj->GetId() << ": Model=" << obj->GetModelId() 
                  << ", Textures=" << obj->GetTextureIds().size() 
                  << ", Shader=" << obj->GetShaderId() << std::endl;
    }
    
    std::cout << "📹 Cameras: " << m_cameras.size() << " (Active: " << m_activeCameraIndex << ")" << std::endl;
    
    if (Camera* cam = GetActiveCamera()) {
        std::cout << "  - Position: (" << cam->GetPosition().x << ", " << cam->GetPosition().y << ", " << cam->GetPosition().z << ")" << std::endl;
        std::cout << "  - Target: (" << cam->GetTarget().x << ", " << cam->GetTarget().y << ", " << cam->GetTarget().z << ")" << std::endl;
    }
    
    std::cout << "==============================\n" << std::endl;
} 