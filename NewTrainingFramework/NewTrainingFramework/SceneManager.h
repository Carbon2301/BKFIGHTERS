#pragma once
#include "Object.h"
#include "Camera.h"
#include <vector>
#include <memory>
#include <string>

class SceneManager {
private:
    // Singleton instance
    static SceneManager* s_instance;
    
    // Scene objects
    std::vector<std::unique_ptr<Object>> m_objects;
    std::vector<std::unique_ptr<Camera>> m_cameras;
    
    // Active camera index
    int m_activeCameraIndex;
    
    // Private constructor for singleton
    SceneManager();
    
public:
    // Singleton access
    static SceneManager* GetInstance();
    static void DestroyInstance();
    
    // Destructor
    ~SceneManager();
    
    // Load scene from SM.txt file
    bool LoadFromFile(const std::string& filepath);
    
    // Object management
    Object* CreateObject(int id = -1);
    Object* GetObject(int id);
    void RemoveObject(int id);
    void RemoveAllObjects();
    
    // Camera management
    Camera* CreateCamera();
    Camera* GetActiveCamera();
    Camera* GetCamera(int index);
    void SetActiveCamera(int index);
    int GetActiveCameraIndex() const { return m_activeCameraIndex; }
    int GetCameraCount() const { return (int)m_cameras.size(); }
    
    // Scene operations
    void Update(float deltaTime);
    void Draw();
    
    // Input handling (for camera controls)
    void HandleInput(unsigned char key, bool isPressed);
    
    // Utility
    void Clear();
    void PrintSceneInfo();
    
    // Object iteration
    const std::vector<std::unique_ptr<Object>>& GetObjects() const { return m_objects; }
}; 