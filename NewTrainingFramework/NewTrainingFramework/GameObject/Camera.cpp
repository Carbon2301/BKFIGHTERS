#include "stdafx.h"
#include "Camera.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Camera::Camera() 
    : m_position(0.0f, 0.0f, 1.0f)
    , m_target(0.0f, 0.0f, 0.0f)
    , m_up(0.0f, 1.0f, 0.0f)
    , m_left(-1.0f), m_right(1.0f), m_bottom(-1.0f), m_top(1.0f)
    , m_nearPlane(0.1f), m_farPlane(100.0f)
    , m_baseLeft(-1.0f), m_baseRight(1.0f), m_baseBottom(-1.0f), m_baseTop(1.0f)
    , m_minZoom(0.3f), m_maxZoom(2.0f)
    , m_currentZoom(1.0f), m_zoomSpeed(2.0f), m_targetZoom(1.0f)
    , m_autoZoomEnabled(false)
    , m_viewNeedsUpdate(true)
    , m_projectionNeedsUpdate(true)
    , m_vpMatrixNeedsUpdate(true) {
}

Camera::Camera(const Vector3& position, const Vector3& target, const Vector3& up)
    : m_position(position.x, position.y, position.z)
    , m_target(target.x, target.y, target.z)
    , m_up(0.0f, 1.0f, 0.0f)  // Force 2D up vector
    , m_left(-1.0f), m_right(1.0f), m_bottom(-1.0f), m_top(1.0f)
    , m_nearPlane(0.1f), m_farPlane(100.0f)
    , m_baseLeft(-1.0f), m_baseRight(1.0f), m_baseBottom(-1.0f), m_baseTop(1.0f)
    , m_minZoom(0.3f), m_maxZoom(2.0f)
    , m_currentZoom(1.0f), m_zoomSpeed(2.0f), m_targetZoom(1.0f)
    , m_autoZoomEnabled(false)
    , m_viewNeedsUpdate(true)
    , m_projectionNeedsUpdate(true)
    , m_vpMatrixNeedsUpdate(true) {
}

Camera::~Camera() {
}

void Camera::SetPosition(const Vector3& position) {
    m_position.x = position.x;
    m_position.y = position.y;
    m_position.z = position.z;
    m_viewNeedsUpdate = true;
    m_vpMatrixNeedsUpdate = true;
}

void Camera::SetTarget(const Vector3& target) {
    m_target.x = target.x;
    m_target.y = target.y;
    m_target.z = target.z;
    m_viewNeedsUpdate = true;
    m_vpMatrixNeedsUpdate = true;
}

void Camera::SetLookAt(const Vector3& position, const Vector3& target, const Vector3& up) {
    m_position.x = position.x;
    m_position.y = position.y;
    m_position.z = position.z;
    m_target.x = target.x;
    m_target.y = target.y;
    m_target.z = target.z;
    // Force 2D up vector (ignore input up vector)
    m_up.x = 0.0f;
    m_up.y = 1.0f;
    m_up.z = 0.0f;
    m_viewNeedsUpdate = true;
    m_vpMatrixNeedsUpdate = true;
}

void Camera::Move2D(float deltaX, float deltaY) {
    m_position.x += deltaX;
    m_position.y += deltaY;
    m_target.x += deltaX;
    m_target.y += deltaY;
    m_viewNeedsUpdate = true;
    m_vpMatrixNeedsUpdate = true;
}

void Camera::SetPosition2D(float x, float y) {
    float deltaX = x - m_position.x;
    float deltaY = y - m_position.y;
    m_position.x = x;
    m_position.y = y;
    // Move target to maintain relative position
    m_target.x += deltaX;
    m_target.y += deltaY;
    m_viewNeedsUpdate = true;
    m_vpMatrixNeedsUpdate = true;
}

void Camera::SetOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    m_left = left;
    m_right = right;
    m_bottom = bottom;
    m_top = top;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
    
    // Store base values for zoom calculations
    m_baseLeft = left;
    m_baseRight = right;
    m_baseBottom = bottom;
    m_baseTop = top;
    
    m_projectionNeedsUpdate = true;
    m_vpMatrixNeedsUpdate = true;
}

void Camera::SetZoomRange(float minZoom, float maxZoom) {
    m_minZoom = minZoom;
    m_maxZoom = maxZoom;
    
    // Clamp current zoom to new range
    if (m_currentZoom < m_minZoom) m_currentZoom = m_minZoom;
    if (m_currentZoom > m_maxZoom) m_currentZoom = m_maxZoom;
    if (m_targetZoom < m_minZoom) m_targetZoom = m_minZoom;
    if (m_targetZoom > m_maxZoom) m_targetZoom = m_maxZoom;
}

void Camera::UpdateZoom(float deltaTime) {
    if (!m_autoZoomEnabled) return;
    
    // Smooth zoom interpolation
    float zoomDiff = m_targetZoom - m_currentZoom;
    if (std::abs(zoomDiff) > 0.001f) {
        m_currentZoom += zoomDiff * m_zoomSpeed * deltaTime;
        
        // Clamp to zoom range
        if (m_currentZoom < m_minZoom) m_currentZoom = m_minZoom;
        if (m_currentZoom > m_maxZoom) m_currentZoom = m_maxZoom;
        
        // Update orthographic projection with new zoom
        m_left = m_baseLeft / m_currentZoom;
        m_right = m_baseRight / m_currentZoom;
        m_bottom = m_baseBottom / m_currentZoom;
        m_top = m_baseTop / m_currentZoom;
        
        m_projectionNeedsUpdate = true;
        m_vpMatrixNeedsUpdate = true;
    }
}

void Camera::SetTargetZoom(float targetZoom) {
    m_targetZoom = targetZoom;
    if (m_targetZoom < m_minZoom) m_targetZoom = m_minZoom;
    if (m_targetZoom > m_maxZoom) m_targetZoom = m_maxZoom;
}

void Camera::UpdateCameraForCharacters(const Vector3& player1Pos, const Vector3& player2Pos, float deltaTime) {
    if (!m_autoZoomEnabled) return;
    
    // Calculate distance between players
    float distance = std::abs(player1Pos.x - player2Pos.x);
    
    // Calculate target zoom based on distance with smoother curve
    // Closer players = higher zoom (smaller view), farther players = lower zoom (larger view)
    float targetZoom;
    
    if (distance < 0.5f) {
        // Very close - maximum zoom
        targetZoom = m_maxZoom;
    } else if (distance > 10.0f) {
        // Very far - minimum zoom
        targetZoom = m_minZoom;
    } else {
        // Smooth interpolation with easing curve for more natural feel
        float normalizedDistance = (distance - 0.5f) / 9.5f; // 0.5 to 10.0 range
        // Use ease-out curve for smoother zoom transition
        float easedDistance = 1.0f - (1.0f - normalizedDistance) * (1.0f - normalizedDistance);
        targetZoom = m_maxZoom - (easedDistance * (m_maxZoom - m_minZoom));
    }
    
    // Set target zoom and update
    SetTargetZoom(targetZoom);
    UpdateZoom(deltaTime);
    
    // Calculate center point between players for camera position
    float centerX = (player1Pos.x + player2Pos.x) * 0.5f;
    float centerY = (player1Pos.y + player2Pos.y) * 0.5f;
    
    // Add slight vertical offset to keep characters in better view
    centerY += 0.2f;
    
    // Smooth camera movement to center point
    Vector3 targetPosition(centerX, centerY, m_position.z);
    Vector3 currentPos = m_position;
    
    // Smooth interpolation for camera position with different speeds for X and Y
    float moveSpeedX = 4.0f; // Faster horizontal movement
    float moveSpeedY = 2.5f; // Slower vertical movement for stability
    Vector3 newPosition;
    newPosition.x = currentPos.x + (targetPosition.x - currentPos.x) * moveSpeedX * deltaTime;
    newPosition.y = currentPos.y + (targetPosition.y - currentPos.y) * moveSpeedY * deltaTime;
    newPosition.z = currentPos.z;
    
    // Update camera position and target
    SetPosition2D(newPosition.x, newPosition.y);
}

const Matrix& Camera::GetViewMatrix() {
    if (m_viewNeedsUpdate) {
        UpdateViewMatrix();
    }
    return m_viewMatrix;
}

const Matrix& Camera::GetProjectionMatrix() {
    if (m_projectionNeedsUpdate) {
        UpdateProjectionMatrix();
    }
    return m_projectionMatrix;
}

const Matrix& Camera::GetViewProjectionMatrix() {
    if (m_vpMatrixNeedsUpdate) {
        UpdateViewProjectionMatrix();
    }
    return m_viewProjectionMatrix;
}

void Camera::UpdateViewMatrix() {
    Vector3 posCopy(m_position.x, m_position.y, m_position.z);
    Vector3 targetCopy(m_target.x, m_target.y, m_target.z);
    Vector3 upCopy(m_up.x, m_up.y, m_up.z);
    m_viewMatrix.SetLookAt(posCopy, targetCopy, upCopy);
    m_viewNeedsUpdate = false;
}

void Camera::UpdateProjectionMatrix() {
    m_projectionMatrix.SetOrthographic(m_left, m_right, m_bottom, m_top, m_nearPlane, m_farPlane);
    m_projectionNeedsUpdate = false;
}

void Camera::UpdateViewProjectionMatrix() {
    if (m_viewNeedsUpdate) {
        UpdateViewMatrix();
    }
    if (m_projectionNeedsUpdate) {
        UpdateProjectionMatrix();
    }
    
    Matrix viewCopy = m_viewMatrix;
    Matrix projCopy = m_projectionMatrix;
    m_viewProjectionMatrix = viewCopy * projCopy;
    m_vpMatrixNeedsUpdate = false;
} 