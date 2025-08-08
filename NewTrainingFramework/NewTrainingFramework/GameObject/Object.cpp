#include "stdafx.h"
#include "Object.h"
#include "../GameManager/ResourceManager.h"
#include "Model.h"
#include "Texture2D.h"
#include "Shaders.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Object::Object() 
    : m_position(0.0f, 0.0f, 0.0f)
    , m_rotation(0.0f, 0.0f, 0.0f)
    , m_scale(1.0f, 1.0f, 1.0f)
    , m_matrixNeedsUpdate(true)
    , m_modelId(-1)
    , m_shaderId(-1)
    , m_id(-1)
    , m_autoRotate(false)
    , m_rotationSpeed(30.0f)
    , m_isLiftPlatform(false)
    , m_liftStartPos(0.0f, 0.0f, 0.0f)
    , m_liftEndPos(0.0f, 0.0f, 0.0f)
    , m_liftSpeed(1.0f)
    , m_liftProgress(0.0f)
    , m_liftGoingUp(true)
    , m_liftPauseTime(1.0f)
    , m_liftPauseTimer(0.0f) {
    m_worldMatrix.SetIdentity();
}

Object::Object(int id) 
    : m_position(0.0f, 0.0f, 0.0f)
    , m_rotation(0.0f, 0.0f, 0.0f)
    , m_scale(1.0f, 1.0f, 1.0f)
    , m_matrixNeedsUpdate(true)
    , m_modelId(-1)
    , m_shaderId(-1)
    , m_id(id)
    , m_autoRotate(false)
    , m_rotationSpeed(30.0f)
    , m_isLiftPlatform(false)
    , m_liftStartPos(0.0f, 0.0f, 0.0f)
    , m_liftEndPos(0.0f, 0.0f, 0.0f)
    , m_liftSpeed(1.0f)
    , m_liftProgress(0.0f)
    , m_liftGoingUp(true)
    , m_liftPauseTime(1.0f)
    , m_liftPauseTimer(0.0f) {
    m_worldMatrix.SetIdentity();
}

Object::~Object() {
}

void Object::SetPosition(const Vector3& position) {
    m_position.x = position.x;
    m_position.y = position.y;
    m_position.z = position.z;
    m_matrixNeedsUpdate = true;
}

void Object::SetRotation(const Vector3& rotation) {
    m_rotation.x = rotation.x;
    m_rotation.y = rotation.y;
    m_rotation.z = rotation.z;
    m_matrixNeedsUpdate = true;
}

void Object::SetScale(const Vector3& scale) {
    m_scale.x = scale.x;
    m_scale.y = scale.y;
    m_scale.z = scale.z;
    m_matrixNeedsUpdate = true;
}

void Object::SetPosition(float x, float y, float z) {
    SetPosition(Vector3(x, y, z));
}

void Object::SetRotation(float x, float y, float z) {
    SetRotation(Vector3(x, y, z));
}

void Object::SetScale(float x, float y, float z) {
    SetScale(Vector3(x, y, z));
}

void Object::SetScale(float uniform) {
    SetScale(Vector3(uniform, uniform, uniform));
}

const Matrix& Object::GetWorldMatrix() {
    if (m_matrixNeedsUpdate) {
        UpdateWorldMatrix();
    }
    return m_worldMatrix;
}

void Object::UpdateWorldMatrix() {
    // Build world matrix: World = Translation × RotationZ × RotationY × RotationX × Scale
    Matrix translation, rotationX, rotationY, rotationZ, scale;
    
    translation.SetTranslation(m_position.x, m_position.y, m_position.z);
    rotationX.SetRotationX(m_rotation.x);
    rotationY.SetRotationY(m_rotation.y);
    rotationZ.SetRotationZ(m_rotation.z);
    scale.SetScale(m_scale.x, m_scale.y, m_scale.z);
    
    // Combine transformations step by step to avoid const reference issues
    Matrix temp1 = rotationX * scale;
    Matrix temp2 = rotationY * temp1;
    Matrix temp3 = rotationZ * temp2;
    m_worldMatrix = temp3 * translation;
    m_matrixNeedsUpdate = false;
}

void Object::SetModel(int modelId) {
    m_modelId = modelId;
    m_model = ResourceManager::GetInstance()->GetModel(modelId);
}

void Object::SetTexture(int textureId, int index) {
    std::cout << "Object ID " << m_id << " setting texture ID " << textureId << " at index " << index << std::endl;
    
    // Resize texture arrays if needed
    if (index >= (int)m_textureIds.size()) {
        m_textureIds.resize(index + 1, -1);
        m_textures.resize(index + 1, nullptr);
    }
    
    m_textureIds[index] = textureId;
    auto texture = ResourceManager::GetInstance()->GetTexture(textureId);
    m_textures[index] = texture;
    
    if (texture) {
        std::cout << "Object ID " << m_id << " successfully assigned texture ID " << textureId << std::endl;
    } else {
        std::cout << "Object ID " << m_id << " FAILED to get texture ID " << textureId << " from ResourceManager!" << std::endl;
    }
}

void Object::AddTexture(int textureId) {
    m_textureIds.push_back(textureId);
    m_textures.push_back(ResourceManager::GetInstance()->GetTexture(textureId));
}

void Object::SetShader(int shaderId) {
    m_shaderId = shaderId;
    m_shader = ResourceManager::GetInstance()->GetShader(shaderId);
}

void Object::CacheResources() {
    ResourceManager* rm = ResourceManager::GetInstance();
    
    // Cache model
    if (m_modelId >= 0) {
        m_model = rm->GetModel(m_modelId);
    }
    
    // Cache textures
    m_textures.clear();
    for (int textureId : m_textureIds) {
        if (textureId >= 0) {
            m_textures.push_back(rm->GetTexture(textureId));
        } else {
            m_textures.push_back(nullptr);
        }
    }
    
    // Cache shader
    if (m_shaderId >= 0) {
        m_shader = rm->GetShader(m_shaderId);
    }
}

void Object::RefreshResources() {
    CacheResources();
}

void Object::Draw(const Matrix& viewMatrix, const Matrix& projectionMatrix) {
    // Check if we have valid resources
    if (!m_model || !m_shader) {
        std::cout << "Object ID " << m_id << " missing resources: model=" << (m_model ? "OK" : "NULL") 
                  << ", shader=" << (m_shader ? "OK" : "NULL") << std::endl;
        return;
    }
    
    // Skip debug spam - render silently without texture
    
    // Use shader
    glUseProgram(m_shader->program);
    
    // Calculate MVP matrix step by step to avoid const reference issues
    const Matrix& worldMatrixRef = GetWorldMatrix();
    Matrix worldMatrix;
    worldMatrix = const_cast<Matrix&>(worldMatrixRef);
    Matrix viewMatrixCopy;
    viewMatrixCopy = const_cast<Matrix&>(viewMatrix);
    Matrix projMatrixCopy;
    projMatrixCopy = const_cast<Matrix&>(projectionMatrix);
    
    Matrix wvMatrix = worldMatrix * viewMatrixCopy;
    Matrix mvpMatrix = wvMatrix * projMatrixCopy;
    
    // Set MVP uniform
    GLint mvpLocation = glGetUniformLocation(m_shader->program, "u_mvpMatrix");
    if (mvpLocation != -1) {
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, &mvpMatrix.m[0][0]);
    }
    
    // Bind textures
    bool hasValidTexture = false;
    for (int i = 0; i < (int)m_textures.size(); ++i) {
        if (m_textures[i]) {
            m_textures[i]->Bind(i);
            
            // Set texture uniform (assume u_texture for first texture)
            if (i == 0) {
                GLint textureLocation = glGetUniformLocation(m_shader->program, "u_texture");
                if (textureLocation != -1) {
                    glUniform1i(textureLocation, i);
                }
                hasValidTexture = true;
            }
        }
    }
    
    // Removed fallback binding to texture ID 0 to avoid masking missing textures
    // if (!hasValidTexture) {
    //     auto fallbackTexture = ResourceManager::GetInstance()->GetTexture(0);
    //     if (fallbackTexture) {
    //         fallbackTexture->Bind(0);
    //         GLint textureLocation = glGetUniformLocation(m_shader->program, "u_texture");
    //         if (textureLocation != -1) {
    //             glUniform1i(textureLocation, 0);
    //         }
    //     }
    // }
    
    // Draw model
    m_model->Draw();
    
    // Unbind textures
    for (int i = 0; i < (int)m_textures.size(); ++i) {
        if (m_textures[i]) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
}

void Object::Update(float deltaTime) {
    // Auto-rotation logic
    if (m_autoRotate) {
        float radiansPerSecond = m_rotationSpeed * (float)M_PI / 180.0f;
        
        // Rotate around Y axis (vertical rotation)
        m_rotation.y += radiansPerSecond * deltaTime;
        
        // Keep rotation in [0, 2π] range
        if (m_rotation.y > 2.0f * (float)M_PI) {
            m_rotation.y -= 2.0f * (float)M_PI;
        }
        
        m_matrixNeedsUpdate = true;
    }
    
    // Lift platform logic
    if (m_isLiftPlatform) {
        // Check if we're in pause state
        if (m_liftPauseTimer > 0.0f) {
            m_liftPauseTimer -= deltaTime;
            return; // Don't move during pause
        }
        
        // Calculate movement
        float distance = m_liftSpeed * deltaTime;
        float totalDistance = Vector3::Distance(m_liftStartPos, m_liftEndPos);
        
        if (m_liftGoingUp) {
            m_liftProgress += distance / totalDistance;
            if (m_liftProgress >= 1.0f) {
                m_liftProgress = 1.0f;
                m_liftGoingUp = false;
                m_liftPauseTimer = m_liftPauseTime; // Start pause at top
            }
        } else {
            m_liftProgress -= distance / totalDistance;
            if (m_liftProgress <= 0.0f) {
                m_liftProgress = 0.0f;
                m_liftGoingUp = true;
                m_liftPauseTimer = m_liftPauseTime; // Start pause at bottom
            }
        }
        
        // Interpolate position
        m_position = Vector3::Lerp(m_liftStartPos, m_liftEndPos, m_liftProgress);
        m_matrixNeedsUpdate = true;
    }
}

// Auto-rotation methods
void Object::ToggleAutoRotation() {
    m_autoRotate = !m_autoRotate;
    std::cout << "Object ID " << m_id << " auto-rotation " << (m_autoRotate ? "ON" : "OFF") << std::endl;
}

void Object::SetAutoRotation(bool enabled, float speed) {
    m_autoRotate = enabled;
    m_rotationSpeed = speed;
    std::cout << "Object ID " << m_id << " auto-rotation set to " << (enabled ? "ON" : "OFF") 
              << " at " << speed << " degrees/sec" << std::endl;
} 

// Thêm hàm cập nhật UV động cho Sprite2D
void Object::SetCustomUV(float u0, float v0, float u1, float v1) {
    if (!m_model || m_model->vboId == 0) return;
    if (m_model->vertices.size() < 4) return;
    // Đúng thứ tự đỉnh Sprite2D.nfg
    m_model->vertices[0].uv = Vector2(u0, v0); // bottom-left
    m_model->vertices[1].uv = Vector2(u0, v1); // top-left
    m_model->vertices[2].uv = Vector2(u1, v1); // top-right
    m_model->vertices[3].uv = Vector2(u1, v0); // bottom-right
    glBindBuffer(GL_ARRAY_BUFFER, m_model->vboId);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * sizeof(Vertex), m_model->vertices.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Lift platform methods
void Object::SetLiftPlatform(bool enabled, const Vector3& startPos, const Vector3& endPos, 
                            float speed, float pauseTime) {
    m_isLiftPlatform = enabled;
    if (enabled) {
        m_liftStartPos = startPos;
        m_liftEndPos = endPos;
        m_liftSpeed = speed;
        m_liftPauseTime = pauseTime;
        m_liftProgress = 0.0f;
        m_liftGoingUp = true;
        m_liftPauseTimer = 0.0f;
        
        // Set initial position to start position
        m_position = startPos;
        m_matrixNeedsUpdate = true;
        
        std::cout << "Object ID " << m_id << " lift platform enabled: " 
                  << "(" << startPos.x << ", " << startPos.y << ", " << startPos.z << ") to "
                  << "(" << endPos.x << ", " << endPos.y << ", " << endPos.z << ")" << std::endl;
    } else {
        std::cout << "Object ID " << m_id << " lift platform disabled" << std::endl;
    }
}

void Object::SetLiftPlatform(bool enabled, float startX, float startY, float startZ,
                            float endX, float endY, float endZ, float speed, float pauseTime) {
    SetLiftPlatform(enabled, Vector3(startX, startY, startZ), Vector3(endX, endY, endZ), speed, pauseTime);
}