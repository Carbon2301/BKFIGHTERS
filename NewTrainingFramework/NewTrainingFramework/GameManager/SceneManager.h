#pragma once
#include "../GameObject/Object.h"
#include "../GameObject/Camera.h"
#include <vector>
#include <memory>
#include <string>

class SceneManager {
private:

    static SceneManager* s_instance;

    std::vector<std::unique_ptr<Object>> m_objects;
    std::vector<std::unique_ptr<Camera>> m_cameras;

    int m_activeCameraIndex;

    SceneManager();
   
public:
    static SceneManager* GetInstance();
    static void DestroyInstance();

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

    void Update(float deltaTime);
    void Draw();

    void HandleInput(unsigned char key, bool isPressed);

    void Clear();
    void PrintSceneInfo();

    const std::vector<std::unique_ptr<Object>>& GetObjects() const { return m_objects; }
}; 