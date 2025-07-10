#include "stdafx.h"
#include "Object.h"
#include "ResourceManager.h"
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
    , m_autoRotate(true)
    , m_rotationSpeed(30.0f) {
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
    , m_autoRotate(true)
    , m_rotationSpeed(30.0f) {
    m_worldMatrix.SetIdentity();
}

Object::~Object() {
    // Resources are managed by ResourceManager, just clear pointers
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
    m_worldMatrix = translation * temp3;
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
    
    // If no valid texture, use default texture or wireframe mode
    if (!hasValidTexture) {
        // Try to use texture ID 0 as fallback
        auto fallbackTexture = ResourceManager::GetInstance()->GetTexture(0);
        if (fallbackTexture) {
            fallbackTexture->Bind(0);
            GLint textureLocation = glGetUniformLocation(m_shader->program, "u_texture");
            if (textureLocation != -1) {
                glUniform1i(textureLocation, 0);
            }
        }
    }
    
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