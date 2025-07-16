#include "stdafx.h"
#include "Camera.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Camera::Camera() 
    : m_position(0.0f, 0.0f, 1.0f)  // Default 2D position with z=1 for depth
    , m_target(0.0f, 0.0f, 0.0f)    // Looking at origin
    , m_up(0.0f, 1.0f, 0.0f)        // Standard 2D up vector
    , m_left(-1.0f), m_right(1.0f), m_bottom(-1.0f), m_top(1.0f)
    , m_nearPlane(0.1f), m_farPlane(100.0f)
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
    m_projectionNeedsUpdate = true;
    m_vpMatrixNeedsUpdate = true;
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
    // 2D-only: Always use orthographic projection
    m_projectionMatrix.SetOrthographic(m_left, m_right, m_bottom, m_top, m_nearPlane, m_farPlane);
    m_projectionNeedsUpdate = false;
}

void Camera::UpdateViewProjectionMatrix() {
    // Make sure individual matrices are up to date
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