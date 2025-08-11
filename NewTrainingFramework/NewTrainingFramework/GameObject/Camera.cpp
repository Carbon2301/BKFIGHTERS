#include "stdafx.h"
#include "Camera.h"
#include <cmath>
#include <SDL.h>

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
    , m_minZoom(1.0f), m_maxZoom(3.5f)
    , m_currentZoom(1.8f), m_zoomSpeed(2.0f), m_targetZoom(2.0f)
    , m_autoZoomEnabled(false)
    , m_verticalOffset(0.0f)
    , m_characterWidth(0.5f), m_characterHeight(1.0f)
    , m_paddingX(0.3f), m_paddingY(0.2f)
    , m_initialPosition(0.0f, 0.0f, 1.0f)
    , m_initialTarget(0.0f, 0.0f, 0.0f)
    , m_initialZoom(1.8f)
    , m_viewNeedsUpdate(true)
    , m_projectionNeedsUpdate(true)
    , m_vpMatrixNeedsUpdate(true) {
}

Camera::Camera(const Vector3& position, const Vector3& target, const Vector3& up)
    : m_position(position.x, position.y, position.z)
    , m_target(target.x, target.y, target.z)
    , m_up(0.0f, 1.0f, 0.0f)
    , m_left(-1.0f), m_right(1.0f), m_bottom(-1.0f), m_top(1.0f)
    , m_nearPlane(0.1f), m_farPlane(100.0f)
    , m_baseLeft(-1.0f), m_baseRight(1.0f), m_baseBottom(-1.0f), m_baseTop(1.0f)
    , m_minZoom(1.0f), m_maxZoom(3.5f)
    , m_currentZoom(1.8f), m_zoomSpeed(2.0f), m_targetZoom(1.8f)
    , m_autoZoomEnabled(false)
    , m_verticalOffset(0.0f)
    , m_characterWidth(0.5f), m_characterHeight(1.0f)
    , m_paddingX(0.3f), m_paddingY(0.2f)
    , m_initialPosition(position.x, position.y, position.z)
    , m_initialTarget(target.x, target.y, target.z)
    , m_initialZoom(1.8f) 
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
    m_target.x += deltaX;
    m_target.y += deltaY;
    m_viewNeedsUpdate = true;
    m_vpMatrixNeedsUpdate = true;
}

void Camera::UpdateShake(float dt) {
    if (m_shakeTimeRemaining <= 0.0f) {
        m_shakeOffset.x = 0.0f; m_shakeOffset.y = 0.0f; m_shakeOffset.z = 0.0f;
        return;
    }
    m_shakeTimeRemaining -= dt;
    float t = m_shakeTimeRemaining / (m_shakeDuration > 0.0f ? m_shakeDuration : 1.0f);
    if (t < 0.0f) t = 0.0f;
    float amp = m_shakeAmplitude * (0.5f + 0.5f * t);
    float time = SDL_GetTicks() / 1000.0f;
    float ox = sinf(time * m_shakeFrequency * 6.2831853f) * amp;
    float oy = cosf(time * (m_shakeFrequency * 0.8f) * 6.2831853f) * amp;
    m_shakeOffset.x = ox;
    m_shakeOffset.y = oy;
}

void Camera::SetOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    m_left = left;
    m_right = right;
    m_bottom = bottom;
    m_top = top;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
    
    m_baseLeft = left;
    m_baseRight = right;
    m_baseBottom = bottom;
    m_baseTop = top;
    
    if (m_initialZoom == 1.0f) {
        m_initialPosition = m_position;
        m_initialTarget = m_target;
        m_initialZoom = 1.0f;
    }
    
    m_projectionNeedsUpdate = true;
    m_vpMatrixNeedsUpdate = true;
}

void Camera::SetZoomRange(float minZoom, float maxZoom) {
    m_minZoom = minZoom;
    m_maxZoom = maxZoom;
    
    if (m_currentZoom < m_minZoom) m_currentZoom = m_minZoom;
    if (m_currentZoom > m_maxZoom) m_currentZoom = m_maxZoom;
    if (m_targetZoom < m_minZoom) m_targetZoom = m_minZoom;
    if (m_targetZoom > m_maxZoom) m_targetZoom = m_maxZoom;
}

void Camera::UpdateZoom(float deltaTime) {
    if (!m_autoZoomEnabled) return;
    
    float zoomDiff = m_targetZoom - m_currentZoom;
    float absZoomDiff = (zoomDiff < 0) ? -zoomDiff : zoomDiff;
    if (absZoomDiff > 0.001f) {
        m_currentZoom += zoomDiff * m_zoomSpeed * deltaTime;
        
        if (m_currentZoom < m_minZoom) m_currentZoom = m_minZoom;
        if (m_currentZoom > m_maxZoom) m_currentZoom = m_maxZoom;
        
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

    float baseViewWidth = m_baseRight - m_baseLeft;
    float baseViewHeight = m_baseTop - m_baseBottom;

    float distanceX = (player1Pos.x > player2Pos.x) ? (player1Pos.x - player2Pos.x) : (player2Pos.x - player1Pos.x);
    float distanceY = (player1Pos.y > player2Pos.y) ? (player1Pos.y - player2Pos.y) : (player2Pos.y - player1Pos.y);
    float normalizedDistanceX = distanceX / (baseViewWidth * 0.6f);
    if (normalizedDistanceX < 0.0f) normalizedDistanceX = 0.0f;
    if (normalizedDistanceX > 1.0f) normalizedDistanceX = 1.0f;

    float paddingScale = 0.3f + 0.7f * normalizedDistanceX; 
    float effectivePaddingX = m_paddingX * paddingScale;
    float effectivePaddingY = m_paddingY * paddingScale;

    float minX = (player1Pos.x < player2Pos.x) ? player1Pos.x : player2Pos.x;
    float maxX = (player1Pos.x > player2Pos.x) ? player1Pos.x : player2Pos.x;
    float minY = (player1Pos.y < player2Pos.y) ? player1Pos.y : player2Pos.y;
    float maxY = (player1Pos.y > player2Pos.y) ? player1Pos.y : player2Pos.y;

    minX -= (m_characterWidth * 0.5f + effectivePaddingX);
    maxX += (m_characterWidth * 0.5f + effectivePaddingX);
    minY -= (m_characterHeight * 0.5f + effectivePaddingY);
    maxY += (m_characterHeight * 0.5f + effectivePaddingY);

    float requiredWidth = maxX - minX;
    float requiredHeight = maxY - minY;

    float zoomForWidth = baseViewWidth / requiredWidth;
    float zoomForHeight = baseViewHeight / requiredHeight;

    float targetZoom = (zoomForWidth < zoomForHeight) ? zoomForWidth : zoomForHeight;

    float closeBias = 1.0f + (1.0f - paddingScale) * 0.60f;
    targetZoom *= closeBias;

    if (targetZoom < m_minZoom) targetZoom = m_minZoom;
    if (targetZoom > m_maxZoom) targetZoom = m_maxZoom;

    SetTargetZoom(targetZoom);
    UpdateZoom(deltaTime);

    float centerX = (minX + maxX) * 0.5f;
    float effectiveVerticalOffset = m_verticalOffset * paddingScale;
    float dynamicYOffset = -(1.0f - paddingScale) * 0.45f;
    float centerY = (minY + maxY) * 0.5f + effectiveVerticalOffset + dynamicYOffset;

    Vector3 targetPosition(centerX, centerY, m_position.z);
    Vector3 currentPos = m_position;

    float moveSpeedX = 5.0f;
    float moveSpeedY = 5.0f;
    Vector3 newPosition;
    newPosition.x = currentPos.x + (targetPosition.x - currentPos.x) * moveSpeedX * deltaTime;
    newPosition.y = currentPos.y + (targetPosition.y - currentPos.y) * moveSpeedY * deltaTime;
    newPosition.z = currentPos.z;

    UpdateShake(deltaTime);
    SetPosition2D(newPosition.x + m_shakeOffset.x, newPosition.y + m_shakeOffset.y);
}

void Camera::SetCharacterDimensions(float width, float height) {
    m_characterWidth = width;
    m_characterHeight = height;
}

void Camera::SetCameraPadding(float paddingX, float paddingY) {
    m_paddingX = paddingX;
    m_paddingY = paddingY;
}

void Camera::ResetToInitialState() {
    m_position = m_initialPosition;
    m_target = m_initialTarget;
    
    m_currentZoom = m_initialZoom;
    m_targetZoom = m_initialZoom;
    
    m_left = m_baseLeft;
    m_right = m_baseRight;
    m_bottom = m_baseBottom;
    m_top = m_baseTop;
    
    m_viewNeedsUpdate = true;
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

void Camera::AddShake(float amplitude, float duration, float frequency) {
    if (amplitude <= 0.0f || duration <= 0.0f) return;
    m_shakeAmplitude = amplitude;
    m_shakeDuration = duration;
    m_shakeFrequency = frequency;
    m_shakeTimeRemaining = duration;
}