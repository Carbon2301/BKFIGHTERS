#include "stdafx.h"
#include "Camera.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Camera::Camera() 
    : m_position(0.0f, 0.0f, 3.0f)
    , m_target(0.0f, 0.0f, 0.0f)
    , m_up(0.0f, 1.0f, 0.0f)
    , m_fov(45.0f * (float)M_PI / 180.0f)
    , m_aspect(16.0f / 9.0f)
    , m_nearPlane(0.1f)
    , m_farPlane(100.0f)
    , m_viewNeedsUpdate(true)
    , m_projectionNeedsUpdate(true)
    , m_vpMatrixNeedsUpdate(true) {
}

Camera::Camera(const Vector3& position, const Vector3& target, const Vector3& up)
    : m_position(position.x, position.y, position.z)
    , m_target(target.x, target.y, target.z)
    , m_up(up.x, up.y, up.z)
    , m_fov(45.0f * (float)M_PI / 180.0f)
    , m_aspect(16.0f / 9.0f)
    , m_nearPlane(0.1f)
    , m_farPlane(100.0f)
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

void Camera::SetUp(const Vector3& up) {
    m_up.x = up.x;
    m_up.y = up.y;
    m_up.z = up.z;
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
    m_up.x = up.x;
    m_up.y = up.y;
    m_up.z = up.z;
    m_viewNeedsUpdate = true;
    m_vpMatrixNeedsUpdate = true;
}

void Camera::MoveForward(float distance) {
    Vector3 forward = GetForwardVector();
    Vector3 offset = forward * distance;
    m_position.x += offset.x;
    m_position.y += offset.y;
    m_position.z += offset.z;
    m_target.x += offset.x;
    m_target.y += offset.y;
    m_target.z += offset.z;
    m_viewNeedsUpdate = true;
    m_vpMatrixNeedsUpdate = true;
}

void Camera::MoveRight(float distance) {
    Vector3 right = GetRightVector();
    Vector3 offset = right * distance;
    m_position.x += offset.x;
    m_position.y += offset.y;
    m_position.z += offset.z;
    m_target.x += offset.x;
    m_target.y += offset.y;
    m_target.z += offset.z;
    m_viewNeedsUpdate = true;
    m_vpMatrixNeedsUpdate = true;
}

void Camera::MoveUp(float distance) {
    Vector3 up = GetUpVector();
    Vector3 offset = up * distance;
    m_position.x += offset.x;
    m_position.y += offset.y;
    m_position.z += offset.z;
    m_target.x += offset.x;
    m_target.y += offset.y;
    m_target.z += offset.z;
    m_viewNeedsUpdate = true;
    m_vpMatrixNeedsUpdate = true;
}

void Camera::Translate(const Vector3& offset) {
    m_position.x += offset.x;
    m_position.y += offset.y;
    m_position.z += offset.z;
    m_target.x += offset.x;
    m_target.y += offset.y;
    m_target.z += offset.z;
    m_viewNeedsUpdate = true;
    m_vpMatrixNeedsUpdate = true;
}

void Camera::SetPerspective(float fov, float aspect, float nearPlane, float farPlane) {
    m_fov = fov;
    m_aspect = aspect;
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
    m_projectionMatrix.SetPerspective(m_fov, m_aspect, m_nearPlane, m_farPlane);
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

Vector3 Camera::GetForwardVector() const {
    Vector3 forward(m_target.x - m_position.x, m_target.y - m_position.y, m_target.z - m_position.z);
    forward.Normalize();
    return forward;
}

Vector3 Camera::GetRightVector() const {
    Vector3 forward = GetForwardVector();
    Vector3 upCopy(m_up.x, m_up.y, m_up.z);
    Vector3 right = forward.Cross(upCopy);
    right.Normalize();
    return right;
}

Vector3 Camera::GetUpVector() const {
    Vector3 forward = GetForwardVector();
    Vector3 right = GetRightVector();
    Vector3 up = right.Cross(forward);
    up.Normalize();
    return up;
}

void Camera::RotateX(float angleRadians) {
    Vector3 forward = GetForwardVector();
    Vector3 right = GetRightVector();
    
    float cosA = cos(angleRadians);
    float sinA = sin(angleRadians);
    
    Vector3 newForward;
    newForward.x = forward.x;
    newForward.y = forward.y * cosA - forward.z * sinA;
    newForward.z = forward.y * sinA + forward.z * cosA;
    
    m_target.x = m_position.x + newForward.x;
    m_target.y = m_position.y + newForward.y; 
    m_target.z = m_position.z + newForward.z;
    
    m_viewNeedsUpdate = true;
    m_vpMatrixNeedsUpdate = true;
}

void Camera::RotateY(float angleRadians) {
    Vector3 forward = GetForwardVector();
    
    float cosA = cos(angleRadians);
    float sinA = sin(angleRadians);
    
    Vector3 newForward;
    newForward.x = forward.x * cosA + forward.z * sinA;
    newForward.y = forward.y;
    newForward.z = -forward.x * sinA + forward.z * cosA;
    
    m_target.x = m_position.x + newForward.x;
    m_target.y = m_position.y + newForward.y;
    m_target.z = m_position.z + newForward.z;
    
    m_viewNeedsUpdate = true;
    m_vpMatrixNeedsUpdate = true;
}

void Camera::RotateZ(float angleRadians) {
    Vector3 right = GetRightVector();
    Vector3 up = GetUpVector();
    
    float cosA = cos(angleRadians);
    float sinA = sin(angleRadians);
    
    Vector3 newUp;
    newUp.x = up.x * cosA - right.x * sinA;
    newUp.y = up.y * cosA - right.y * sinA;
    newUp.z = up.z * cosA - right.z * sinA;
    
    m_up.x = newUp.x;
    m_up.y = newUp.y;
    m_up.z = newUp.z;
    
    m_viewNeedsUpdate = true;
    m_vpMatrixNeedsUpdate = true;
}

void Camera::ZoomIn(float factor) {
    m_fov *= factor;
    if (m_fov < 10.0f * (float)M_PI / 180.0f) {
        m_fov = 10.0f * (float)M_PI / 180.0f;
    }
    m_projectionNeedsUpdate = true;
    m_vpMatrixNeedsUpdate = true;
}

void Camera::ZoomOut(float factor) {
    m_fov *= factor;
    if (m_fov > 120.0f * (float)M_PI / 180.0f) {
        m_fov = 120.0f * (float)M_PI / 180.0f;
    }
    m_projectionNeedsUpdate = true;
    m_vpMatrixNeedsUpdate = true;
}

void Camera::SetZoom(float fovDegrees) {
    m_fov = fovDegrees * (float)M_PI / 180.0f;
    if (m_fov < 10.0f * (float)M_PI / 180.0f) {
        m_fov = 10.0f * (float)M_PI / 180.0f;
    }
    if (m_fov > 120.0f * (float)M_PI / 180.0f) {
        m_fov = 120.0f * (float)M_PI / 180.0f;
    }
    m_projectionNeedsUpdate = true;
    m_vpMatrixNeedsUpdate = true;
} 