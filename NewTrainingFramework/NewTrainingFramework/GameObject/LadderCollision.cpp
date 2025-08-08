#include "stdafx.h"
#include "LadderCollision.h"
#include "../GameManager/SceneManager.h"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

LadderCollision::LadderCollision() {}
LadderCollision::~LadderCollision() {}

void LadderCollision::AddLadder(float x, float y, float width, float height, int objectId) {
    m_ladders.emplace_back(x, y, width, height, objectId);
}

void LadderCollision::ClearLadders() {
    m_ladders.clear();
}

void LadderCollision::LoadLaddersFromScene() {
    ClearLadders();

    std::ifstream file("Resources/Scenes/GSPlay.txt");
    if (!file.is_open()) {
        std::cerr << "Cannot open GSPlay.txt" << std::endl;
        return;
    }

    std::string line;
    int objectId = -1;
    float posX = 0, posY = 0;
    float scaleX = 1, scaleY = 1;
    bool inLadderBlock = false;

    while (std::getline(file, line)) {
        if (line.find("# Ladder") != std::string::npos) {
            if (inLadderBlock && objectId != -1) {
                AddLadder(posX, posY, scaleX, scaleY, objectId);
            }
            inLadderBlock = true;
            objectId = -1;
            posX = posY = 0;
            scaleX = scaleY = 1;
        } else if (inLadderBlock) {
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
                    AddLadder(posX, posY, scaleX, scaleY, objectId);
                }
                inLadderBlock = false;
            }
        }
    }
    if (inLadderBlock && objectId != -1) {
        AddLadder(posX, posY, scaleX, scaleY, objectId);
    }
}

bool LadderCollision::CheckLadderOverlapWithHurtbox(
    float posX, float posY,
    float hurtboxWidth, float hurtboxHeight,
    float hurtboxOffsetX, float hurtboxOffsetY,
    float& outLadderCenterX,
    float& outLadderTop,
    float& outLadderBottom) const {

    const float hurtboxCenterX = posX + hurtboxOffsetX;
    const float hurtboxCenterY = posY + hurtboxOffsetY;
    const float hLeft = hurtboxCenterX - hurtboxWidth * 0.5f;
    const float hRight = hurtboxCenterX + hurtboxWidth * 0.5f;
    const float hBottom = hurtboxCenterY - hurtboxHeight * 0.5f;
    const float hTop = hurtboxCenterY + hurtboxHeight * 0.5f;

    for (const auto& ladder : m_ladders) {
        const float lLeft = ladder.GetLeft();
        const float lRight = ladder.GetRight();
        const float lBottom = ladder.GetBottom();
        const float lTop = ladder.GetTop();

        const bool overlapX = (hRight >= lLeft && hLeft <= lRight);
        const bool overlapY = (hTop >= lBottom && hBottom <= lTop);
        if (overlapX && overlapY) {
            outLadderCenterX = ladder.x;
            outLadderTop = lTop;
            outLadderBottom = lBottom;
            return true;
        }
    }
    return false;
}


