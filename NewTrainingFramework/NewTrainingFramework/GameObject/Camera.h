#pragma once
#include "../../Utilities/Math.h"

class Camera {
private:
    // 2D Camera parameters (simplified)
    Vector3 m_position;  // Only x,y used for 2D, z for depth ordering
    Vector3 m_target;    // Only x,y used for 2D
    Vector3 m_up;        // Fixed to (0,1,0) for 2D
    
    // 2D Orthographic projection parameters only
    float m_left, m_right, m_bottom, m_top;
    float m_nearPlane, m_farPlane;
    
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
    
    // 2D positioning only
    void SetPosition(const Vector3& position);
    void SetTarget(const Vector3& target);
    void SetLookAt(const Vector3& position, const Vector3& target, const Vector3& up);
    
    // Simple 2D movement (x,y only)
    void Move2D(float deltaX, float deltaY);
    void SetPosition2D(float x, float y);
    
    // 2D Orthographic projection only
    void SetOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);
    
    // Matrix getters
    const Matrix& GetViewMatrix();
    const Matrix& GetProjectionMatrix();
    const Matrix& GetViewProjectionMatrix();
    
    // Parameter getters (simplified)
    const Vector3& GetPosition() const { return m_position; }
    const Vector3& GetTarget() const { return m_target; }
    float GetLeft() const { return m_left; }
    float GetRight() const { return m_right; }
    float GetBottom() const { return m_bottom; }
    float GetTop() const { return m_top; }
    float GetNearPlane() const { return m_nearPlane; }
    float GetFarPlane() const { return m_farPlane; }
}; 