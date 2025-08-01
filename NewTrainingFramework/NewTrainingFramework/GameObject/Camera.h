#pragma once
#include "../../Utilities/Math.h"

class Camera {
private:
    Vector3 m_position;
    Vector3 m_target;
    Vector3 m_up;
    
    float m_left, m_right, m_bottom, m_top;
    float m_nearPlane, m_farPlane;
    
    // Camera zoom and tracking variables
    float m_baseLeft, m_baseRight, m_baseBottom, m_baseTop;
    float m_minZoom, m_maxZoom;
    float m_currentZoom;
    float m_zoomSpeed;
    float m_targetZoom;
    bool m_autoZoomEnabled;
    
    Matrix m_viewMatrix;
    Matrix m_projectionMatrix;
    Matrix m_viewProjectionMatrix;
    
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
    
    void SetPosition(const Vector3& position);
    void SetTarget(const Vector3& target);
    void SetLookAt(const Vector3& position, const Vector3& target, const Vector3& up);
    
    void Move2D(float deltaX, float deltaY);
    void SetPosition2D(float x, float y);
    
    void SetOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);
    
    // Camera zoom and tracking methods
    void EnableAutoZoom(bool enable) { m_autoZoomEnabled = enable; }
    void SetZoomRange(float minZoom, float maxZoom);
    void SetZoomSpeed(float speed) { m_zoomSpeed = speed; }
    void UpdateZoom(float deltaTime);
    void SetTargetZoom(float targetZoom);
    float GetCurrentZoom() const { return m_currentZoom; }
    bool IsAutoZoomEnabled() const { return m_autoZoomEnabled; }
    
    // Dynamic camera positioning based on character positions
    void UpdateCameraForCharacters(const Vector3& player1Pos, const Vector3& player2Pos, float deltaTime);
    
    const Matrix& GetViewMatrix();
    const Matrix& GetProjectionMatrix();
    const Matrix& GetViewProjectionMatrix();
    
    const Vector3& GetPosition() const { return m_position; }
    const Vector3& GetTarget() const { return m_target; }
    float GetLeft() const { return m_left; }
    float GetRight() const { return m_right; }
    float GetBottom() const { return m_bottom; }
    float GetTop() const { return m_top; }
    float GetNearPlane() const { return m_nearPlane; }
    float GetFarPlane() const { return m_farPlane; }
}; 