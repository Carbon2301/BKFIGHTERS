#include "stdafx.h"
#include "TeleportCollision.h"
#include "../GameManager/SceneManager.h"
#include <fstream>
#include <sstream>
#include <iostream>

TeleportCollision::TeleportCollision() {}
TeleportCollision::~TeleportCollision() {}

void TeleportCollision::ClearTeleports() {
    m_gates.clear();
}

void TeleportCollision::AddTeleport(float x, float y, float width, float height, int objectId) {
    m_gates.emplace_back(x, y, width, height, objectId);
}

void TeleportCollision::LoadTeleportsFromScene() {
    ClearTeleports();

    std::ifstream file("../Resources/GSPlay.txt");
    if (!file.is_open()) {
        std::cerr << "Cannot open GSPlay.txt" << std::endl;
        return;
    }

    std::string line;
    int objectId = -1;
    float posX = 0, posY = 0;
    float scaleX = 1, scaleY = 1;
    bool inTeleportBlock = false;

    while (std::getline(file, line)) {
        if (line.find("# Teleport") != std::string::npos) {
            if (inTeleportBlock && objectId != -1) {
                AddTeleport(posX, posY, scaleX, scaleY, objectId);
            }
            inTeleportBlock = true;
            objectId = -1;
            posX = posY = 0;
            scaleX = scaleY = 1;
        } else if (inTeleportBlock) {
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
                if (objectId != -1) {
                    AddTeleport(posX, posY, scaleX, scaleY, objectId);
                }
                inTeleportBlock = false;
            }
        }
    }
    if (inTeleportBlock && objectId != -1) {
        AddTeleport(posX, posY, scaleX, scaleY, objectId);
    }
}

bool TeleportCollision::GetPairedTeleportId(int fromId, int& outToId) const {
    // Pair by position in m_gates: 0<->1, 2<->3, ...
    for (size_t i = 0; i < m_gates.size(); ++i) {
        if (m_gates[i].objectId == fromId) {
            size_t pairIndex = (i % 2 == 0) ? (i + 1) : (i - 1);
            if (pairIndex < m_gates.size()) {
                outToId = m_gates[pairIndex].objectId;
                return true;
            }
            return false;
        }
    }
    return false;
}

bool TeleportCollision::DetectEnterFromLeft(const Vector3& prevPos,
                                            const Vector3& newPos,
                                            float hurtboxWidth, float hurtboxHeight,
                                            float hurtboxOffsetX, float hurtboxOffsetY,
                                            bool movingRight,
                                            int& outFromId,
                                            int& outToId) const {
    if (!movingRight) return false;

    const float prevCenterX = prevPos.x + hurtboxOffsetX;
    const float prevCenterY = prevPos.y + hurtboxOffsetY;
    const float prevRight = prevCenterX + hurtboxWidth * 0.5f;
    const float prevTop = prevCenterY + hurtboxHeight * 0.5f;
    const float prevBottom = prevCenterY - hurtboxHeight * 0.5f;

    const float newCenterX = newPos.x + hurtboxOffsetX;
    const float newCenterY = newPos.y + hurtboxOffsetY;
    const float newRight = newCenterX + hurtboxWidth * 0.5f;
    const float newTop = newCenterY + hurtboxHeight * 0.5f;
    const float newBottom = newCenterY - hurtboxHeight * 0.5f;

    for (size_t i = 0; i < m_gates.size(); ++i) {
        const TeleportGate& g = m_gates[i];
        const float gLeft = g.GetLeft();
        const float gRight = g.GetRight();
        const float gTop = g.GetTop();
        const float gBottom = g.GetBottom();

        bool crossedLeftEdge = (prevRight < gLeft && newRight >= gLeft);
        if (!crossedLeftEdge) continue;

        bool overlapYPrev = !(prevTop < gBottom || prevBottom > gTop);
        bool overlapYNew = !(newTop < gBottom || newBottom > gTop);
        if (!(overlapYPrev || overlapYNew)) continue;

        int toId;
        if (GetPairedTeleportId(g.objectId, toId)) {
            outFromId = g.objectId;
            outToId = toId;
            return true;
        }
    }
    return false;
}

bool TeleportCollision::ComputeExitPosition(int toId,
                                            float currentY,
                                            float hurtboxWidth, float hurtboxHeight,
                                            float hurtboxOffsetX, float hurtboxOffsetY,
                                            Vector3& outExitPos) const {
    for (const auto& g : m_gates) {
        if (g.objectId == toId) {
            float exitX = g.GetLeft() - (hurtboxOffsetX - hurtboxWidth * 0.5f);
            float minY = g.GetBottom() + hurtboxHeight * 0.5f - hurtboxOffsetY;
            float maxY = g.GetTop()    - hurtboxHeight * 0.5f - hurtboxOffsetY;
            float exitY = currentY;
            if (exitY < minY) exitY = minY;
            if (exitY > maxY) exitY = maxY;

            outExitPos = Vector3(exitX, exitY, 0.0f);
            return true;
        }
    }
    return false;
}

bool TeleportCollision::GetPortalBounds(int id, float& outLeft, float& outRight, float& outBottom, float& outTop) const {
    for (const auto& g : m_gates) {
        if (g.objectId == id) {
            outLeft = g.GetLeft();
            outRight = g.GetRight();
            outBottom = g.GetBottom();
            outTop = g.GetTop();
            return true;
        }
    }
    return false;
}


