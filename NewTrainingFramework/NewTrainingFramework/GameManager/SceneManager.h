#pragma once
#include "../GameObject/Object.h"
#include "../GameObject/Camera.h"
#include "StateType.h"
#include <vector>
#include <memory>
#include <string>

struct CameraConfig {
    // 2D Camera settings (orthographic only)
    Vector3 position = Vector3(0.0f, 0.0f, 1.0f);
    Vector3 target = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
    
    // 2D Orthographic projection settings
    float left = -1.78f;
    float right = 1.78f;
    float bottom = -1.0f;
    float top = 1.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
};

class SceneManager {
private:
    static SceneManager* s_instance;

    std::vector<std::unique_ptr<Object>> m_objects;
    std::vector<std::unique_ptr<Camera>> m_cameras;

    int m_activeCameraIndex;
    CameraConfig m_cameraConfig;

    SceneManager();
   
public:
    static SceneManager* GetInstance();
    static void DestroyInstance();

    ~SceneManager();
    
    // Scene loading
    bool LoadFromFile(const std::string& filepath);
    bool LoadSceneForState(StateType stateType);
    
    // Object management
    Object* CreateObject(int id = -1);
    Object* GetObject(int id);
    void RemoveObject(int id);
    void RemoveAllObjects();
    
    // Camera management (2D only)
    Camera* CreateCamera();
    Camera* GetActiveCamera();
    Camera* GetCamera(int index);
    void SetActiveCamera(int index);
    int GetActiveCameraIndex() const { return m_activeCameraIndex; }
    int GetCameraCount() const { return (int)m_cameras.size(); }

    // Camera configuration (2D orthographic only)
    void SetupCameraFromConfig();
    const CameraConfig& GetCameraConfig() const { return m_cameraConfig; }

    void Update(float deltaTime);
    void Draw();

    // 2D input handling only
    void HandleInput(unsigned char key, bool isPressed);

    void Clear();
    const std::vector<std::unique_ptr<Object>>& GetObjects() const { return m_objects; }

private:
    std::string GetSceneFileForState(StateType stateType);
    bool ParseCameraConfig(std::ifstream& file, const std::string& line);
}; 