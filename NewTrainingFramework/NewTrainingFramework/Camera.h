#pragma once
#include "../Utilities/Math.h"

class Camera {
private:
    // Camera parameters
    Vector3 m_position;
    Vector3 m_target;
    Vector3 m_up;
    
    // Projection parameters
    float m_fov;         // Field of view in radians
    float m_aspect;      // Aspect ratio
    float m_nearPlane;   // Near clipping plane
    float m_farPlane;    // Far clipping plane
    
    // Cached matrices
    Matrix m_viewMatrix;
    Matrix m_projectionMatrix;
    Matrix m_viewProjectionMatrix;
    
    // Update flags
    bool m_viewNeedsUpdate;
    bool m_projectionNeedsUpdate;
    bool m_vpMatrixNeedsUpdate;
    
    void UpdateViewMatrix();
    void UpdateProjectionMatrix();
    void UpdateViewProjectionMatrix();
    
public:
    Camera();
    Camera(const Vector3& position, const Vector3& target, const Vector3& up);
    ~Camera();
    
    // Camera positioning
    void SetPosition(const Vector3& position);
    void SetTarget(const Vector3& target);
    void SetUp(const Vector3& up);
    void SetLookAt(const Vector3& position, const Vector3& target, const Vector3& up);
    
    // Movement
    void MoveForward(float distance);
    void MoveRight(float distance);
    void MoveUp(float distance);
    void Translate(const Vector3& offset);
    
    // Projection setup
    void SetPerspective(float fov, float aspect, float nearPlane, float farPlane);
    void SetOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);
    
    // Matrix getters
    const Matrix& GetViewMatrix();
    const Matrix& GetProjectionMatrix();
    const Matrix& GetViewProjectionMatrix();
    
    // Parameter getters
    const Vector3& GetPosition() const { return m_position; }
    const Vector3& GetTarget() const { return m_target; }
    const Vector3& GetUp() const { return m_up; }
    float GetFOV() const { return m_fov; }
    float GetAspect() const { return m_aspect; }
    float GetNearPlane() const { return m_nearPlane; }
    float GetFarPlane() const { return m_farPlane; }
    
    // Utility methods
    Vector3 GetForwardVector() const;
    Vector3 GetRightVector() const;
    Vector3 GetUpVector() const;
}; 