#pragma once
#include "../../Utilities/Math.h"
#include <vector>
#include <memory>

// Forward declarations
class Model;
class Texture2D;
class Shaders;
class Animation2D;

class Object {
private:
    // Transform data
    Vector3 m_position;
    Vector3 m_rotation;
    Vector3 m_scale;
    Matrix m_worldMatrix;
    bool m_matrixNeedsUpdate;
    
    // Resource references (IDs from ResourceManager)
    int m_modelId;
    std::vector<int> m_textureIds;
    int m_shaderId;
    
    // Cached resource pointers (for performance)
    std::shared_ptr<Model> m_model;
    std::vector<std::shared_ptr<Texture2D>> m_textures;
    std::shared_ptr<Shaders> m_shader;
    
    // Object ID for identification
    int m_id;
    
    // Auto-rotation
    bool m_autoRotate;
    float m_rotationSpeed;  // degrees per second
    
    void UpdateWorldMatrix();
    void CacheResources();
    
public:
    Object();
    Object(int id);
    ~Object();
    
    // Transform methods
    void SetPosition(const Vector3& position);
    void SetRotation(const Vector3& rotation);
    void SetScale(const Vector3& scale);
    void SetPosition(float x, float y, float z);
    void SetRotation(float x, float y, float z);
    void SetScale(float x, float y, float z);
    void SetScale(float uniform);
    
    // 2D helper methods
    void SetSize(float width, float height) { SetScale(width, height, 1.0f); }
    void Set2DPosition(float x, float y) { SetPosition(x, y, m_position.z); }
    
    const Vector3& GetPosition() const { return m_position; }
    const Vector3& GetRotation() const { return m_rotation; }
    const Vector3& GetScale() const { return m_scale; }
    const Matrix& GetWorldMatrix();
    
    // Resource assignment
    void SetModel(int modelId);
    void SetTexture(int textureId, int index = 0);
    void AddTexture(int textureId);
    void SetShader(int shaderId);
    
    int GetModelId() const { return m_modelId; }
    const std::vector<int>& GetTextureIds() const { return m_textureIds; }
    int GetShaderId() const { return m_shaderId; }
    
    // Object ID
    void SetId(int id) { m_id = id; }
    int GetId() const { return m_id; }
    
    // Rendering
    void Draw(const Matrix& viewMatrix, const Matrix& projectionMatrix);
    void Update(float deltaTime);
    
    // Auto-rotation
    void ToggleAutoRotation();
    void SetAutoRotation(bool enabled, float speed = 30.0f); // 30 degrees/sec default
    bool IsAutoRotating() const { return m_autoRotate; }
    
    // Resource management
    void RefreshResources();

    // Getter cho model (tránh truy cập trực tiếp biến private)
    std::shared_ptr<Model> GetModelPtr() const { return m_model; }

    // Thêm hàm public để gán texture động (dùng cho text)
    void SetDynamicTexture(std::shared_ptr<Texture2D> tex) {
        m_textureIds.clear();
        m_textures.clear();
        m_textures.push_back(tex);
    }

    // Thêm hàm cập nhật UV động cho Sprite2D
    void SetCustomUV(float u0, float v0, float u1, float v1);
}; 