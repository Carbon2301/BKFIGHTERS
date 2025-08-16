#include "stdafx.h"
#include "WallCollision.h"
#include "Object.h"
#include "../GameManager/SceneManager.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>

WallCollision::WallCollision() {
}

WallCollision::~WallCollision() {
}

void WallCollision::AddWall(float x, float y, float width, float height, int objectId) {
    m_walls.emplace_back(x, y, width, height, objectId);
}

void WallCollision::AddWallFromObject(int objectId) {
    SceneManager* sceneManager = SceneManager::GetInstance();
    if (!sceneManager) {
        return;
    }
    
    Object* wallObject = sceneManager->GetObject(objectId);
    if (!wallObject) {
        return;
    }
    
    const Vector3& position = wallObject->GetPosition();
    const Vector3& scale = wallObject->GetScale();
    
    AddWall(position.x, position.y, scale.x, scale.y, objectId);
}

void WallCollision::ClearWalls() {
    m_walls.clear();
}

void WallCollision::LoadWallsFromScene() {
    ClearWalls();

    std::ifstream file("../Resources/GSPlay.txt");
    if (!file.is_open()) {
        std::cerr << "Cannot open GSPlay.txt" << std::endl;
        return;
    }

    std::string line;
    int objectId = -1;
    float posX = 0, posY = 0;
    float scaleX = 1, scaleY = 1;
    bool inWallBlock = false;

    while (std::getline(file, line)) {
        if (line.find("# Wall") != std::string::npos) {
            // Nếu đang trong block tường trước đó, thêm tường vào danh sách
            if (inWallBlock && objectId != -1) {
                AddWall(posX, posY, scaleX, scaleY, objectId);
            }
            // Bắt đầu block tường mới
            inWallBlock = true;
            objectId = -1;
            posX = posY = 0;
            scaleX = scaleY = 1;
        } else if (inWallBlock) {
            if (line.find("ID") == 0) {
                std::istringstream iss(line);
                std::string dummy;
                iss >> dummy >> objectId;
            } else if (line.find("POS") == 0) {
                std::istringstream iss(line);
                std::string dummy;
                float z;
                iss >> dummy >> posX >> posY >> z;
            } else if (line.find("SCALE") == 0) {
                std::istringstream iss(line);
                std::string dummy;
                float z;
                iss >> dummy >> scaleX >> scaleY >> z;
            } else if (line.empty() || line[0] == '#') {
                // End of wall block, add wall if valid
                if (objectId != -1) {
                    AddWall(posX, posY, scaleX, scaleY, objectId);
                }
                inWallBlock = false;
            }
        }
    }
    // Thêm tường cuối cùng nếu còn
    if (inWallBlock && objectId != -1) {
        AddWall(posX, posY, scaleX, scaleY, objectId);
    }
    
    // Log danh sách tường sau khi load
    std::cout << "[WallCollision] Loaded walls: " << m_walls.size() << std::endl;
    for (const auto& wall : m_walls) {
        std::cout << "  Wall ID: " << wall.objectId << ", Pos: (" << wall.x << ", " << wall.y << ") Size: (" << wall.width << ", " << wall.height << ")" << std::endl;
    }
}

bool WallCollision::CheckAABBCollision(float aLeft, float aRight, float aBottom, float aTop,
                                      float bLeft, float bRight, float bBottom, float bTop) const {
    return (aLeft < bRight && aRight > bLeft && aBottom < bTop && aTop > bBottom);
}

bool WallCollision::CheckWallCollision(const Vector3& position, 
                                      float hurtboxWidth, float hurtboxHeight,
                                      float hurtboxOffsetX, float hurtboxOffsetY) const {
    float hurtboxCenterX = position.x + hurtboxOffsetX;
    float hurtboxCenterY = position.y + hurtboxOffsetY;
    float hurtboxLeft = hurtboxCenterX - hurtboxWidth / 2.0f;
    float hurtboxRight = hurtboxCenterX + hurtboxWidth / 2.0f;
    float hurtboxBottom = hurtboxCenterY - hurtboxHeight / 2.0f;
    float hurtboxTop = hurtboxCenterY + hurtboxHeight / 2.0f;
    
    for (const Wall& wall : m_walls) {
        if (CheckAABBCollision(hurtboxLeft, hurtboxRight, hurtboxBottom, hurtboxTop,
                              wall.GetLeft(), wall.GetRight(), wall.GetBottom(), wall.GetTop())) {
            return true;
        }
    }
    
    return false;
}

Vector3 WallCollision::ResolveCollision(const Vector3& currentPos, const Vector3& newPos,
                                       float hurtboxWidth, float hurtboxHeight,
                                       float hurtboxOffsetX, float hurtboxOffsetY,
                                       const Wall& wall) const {
    Vector3 resolvedPos(newPos.x, newPos.y, newPos.z);
    
    float hurtboxCenterX = newPos.x + hurtboxOffsetX;
    float hurtboxCenterY = newPos.y + hurtboxOffsetY;
    float hurtboxLeft = hurtboxCenterX - hurtboxWidth / 2.0f;
    float hurtboxRight = hurtboxCenterX + hurtboxWidth / 2.0f;
    float hurtboxBottom = hurtboxCenterY - hurtboxHeight / 2.0f;
    float hurtboxTop = hurtboxCenterY + hurtboxHeight / 2.0f;
    
    float currentHurtboxCenterX = currentPos.x + hurtboxOffsetX;
    float currentHurtboxCenterY = currentPos.y + hurtboxOffsetY;
    float currentHurtboxLeft = currentHurtboxCenterX - hurtboxWidth / 2.0f;
    float currentHurtboxRight = currentHurtboxCenterX + hurtboxWidth / 2.0f;
    float currentHurtboxBottom = currentHurtboxCenterY - hurtboxHeight / 2.0f;
    float currentHurtboxTop = currentHurtboxCenterY + hurtboxHeight / 2.0f;
    
    float moveX = newPos.x - currentPos.x;
    float moveY = newPos.y - currentPos.y;
    
    float overlapLeft = wall.GetRight() - hurtboxLeft;
    float overlapRight = hurtboxRight - wall.GetLeft();
    float overlapBottom = wall.GetTop() - hurtboxBottom;
    float overlapTop = hurtboxTop - wall.GetBottom();
    
    bool wasCollidingLeft = (currentHurtboxRight > wall.GetLeft() && currentHurtboxLeft < wall.GetRight() &&
                            currentHurtboxTop > wall.GetBottom() && currentHurtboxBottom < wall.GetTop());
    
    bool resolveHorizontal = false;
    bool resolveVertical = false;
    
    if (std::abs(moveX) > std::abs(moveY)) {
        if (moveX > 0 && overlapLeft <= overlapRight) {
            resolvedPos.x = wall.GetLeft() - hurtboxWidth / 2.0f - hurtboxOffsetX;
            resolveHorizontal = true;
        }
        else if (moveX < 0 && overlapRight <= overlapLeft) {
            resolvedPos.x = wall.GetRight() + hurtboxWidth / 2.0f - hurtboxOffsetX;
            resolveHorizontal = true;
        }
    } 
    else if (std::abs(moveY) > 0.001f) {
        if (moveY > 0 && overlapBottom <= overlapTop) {
            resolvedPos.y = wall.GetBottom() - hurtboxHeight / 2.0f - hurtboxOffsetY;
            resolveVertical = true;
        }
        else if (moveY < 0 && overlapTop <= overlapBottom) {
            resolvedPos.y = wall.GetTop() + hurtboxHeight / 2.0f - hurtboxOffsetY;
            resolveVertical = true;
        }
    }
    
    if (!resolveHorizontal && !resolveVertical) {
        if (overlapLeft <= overlapRight && overlapLeft <= overlapBottom && overlapLeft <= overlapTop) {
            resolvedPos.x = wall.GetLeft() - hurtboxWidth / 2.0f - hurtboxOffsetX;
        }
        else if (overlapRight <= overlapBottom && overlapRight <= overlapTop) {
            resolvedPos.x = wall.GetRight() + hurtboxWidth / 2.0f - hurtboxOffsetX;
        }
        else if (overlapBottom <= overlapTop) {
            resolvedPos.y = wall.GetBottom() - hurtboxHeight / 2.0f - hurtboxOffsetY;
        }
        else {
            resolvedPos.y = wall.GetTop() + hurtboxHeight / 2.0f - hurtboxOffsetY;
        }
    }
    
    return resolvedPos;
}

Vector3 WallCollision::ResolveWallCollision(const Vector3& currentPos, const Vector3& newPos,
                                          float hurtboxWidth, float hurtboxHeight,
                                          float hurtboxOffsetX, float hurtboxOffsetY) const {
    Vector3 resolvedPos(newPos.x, newPos.y, newPos.z);
    
    for (const Wall& wall : m_walls) {
        float hurtboxCenterX = resolvedPos.x + hurtboxOffsetX;
        float hurtboxCenterY = resolvedPos.y + hurtboxOffsetY;
        float hurtboxLeft = hurtboxCenterX - hurtboxWidth / 2.0f;
        float hurtboxRight = hurtboxCenterX + hurtboxWidth / 2.0f;
        float hurtboxBottom = hurtboxCenterY - hurtboxHeight / 2.0f;
        float hurtboxTop = hurtboxCenterY + hurtboxHeight / 2.0f;
        
        if (!CheckAABBCollision(hurtboxLeft, hurtboxRight, hurtboxBottom, hurtboxTop,
                               wall.GetLeft(), wall.GetRight(), wall.GetBottom(), wall.GetTop())) {
            continue;
        }        
        float penetrationLeft = hurtboxRight - wall.GetLeft();
        float penetrationRight = wall.GetRight() - hurtboxLeft;
        float penetrationTop = hurtboxTop - wall.GetBottom();
        float penetrationBottom = wall.GetTop() - hurtboxBottom;
        
        float minPenetration = penetrationLeft;
        int direction = 0;
        
        if (penetrationRight < minPenetration) {
            minPenetration = penetrationRight;
            direction = 1;
        }
        if (penetrationTop < minPenetration) {
            minPenetration = penetrationTop;
            direction = 2;
        }
        if (penetrationBottom < minPenetration) {
            minPenetration = penetrationBottom;
            direction = 3;
        }
        
        switch (direction) {
            case 0:
                resolvedPos.x = wall.GetLeft() - hurtboxWidth / 2.0f - hurtboxOffsetX;
                break;
            case 1:
                resolvedPos.x = wall.GetRight() + hurtboxWidth / 2.0f - hurtboxOffsetX;
                break;
            case 2:
                resolvedPos.y = wall.GetBottom() - hurtboxHeight / 2.0f - hurtboxOffsetY;
                break;
            case 3:
                resolvedPos.y = wall.GetTop() + hurtboxHeight / 2.0f - hurtboxOffsetY;
                break;
        }
    }
    
    return resolvedPos;
}

bool WallCollision::CheckCharacterWallCollision(const Vector3& position, 
                                               float hurtboxWidth, float hurtboxHeight,
                                               float hurtboxOffsetX, float hurtboxOffsetY) const {
    return CheckWallCollision(position, hurtboxWidth, hurtboxHeight, 
                             hurtboxOffsetX, hurtboxOffsetY);
}

Vector3 WallCollision::ResolveCharacterWallCollision(const Vector3& currentPos, const Vector3& newPos,
                                                   float hurtboxWidth, float hurtboxHeight,
                                                   float hurtboxOffsetX, float hurtboxOffsetY) const {
    return ResolveWallCollision(currentPos, newPos,
                               hurtboxWidth, hurtboxHeight,
                               hurtboxOffsetX, hurtboxOffsetY);
}
