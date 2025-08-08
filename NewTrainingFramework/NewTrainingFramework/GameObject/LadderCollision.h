#pragma once
#include "../../Utilities/Math.h"
#include <vector>

struct Ladder {
    float x, y;
    float width, height;
    int objectId;

    Ladder(float px, float py, float w, float h, int id)
        : x(px), y(py), width(w), height(h), objectId(id) {}

    float GetLeft() const { return x - width * 0.5f; }
    float GetRight() const { return x + width * 0.5f; }
    float GetBottom() const { return y - height * 0.5f; }
    float GetTop() const { return y + height * 0.5f; }
};

class LadderCollision {
private:
    std::vector<Ladder> m_ladders;

public:
    LadderCollision();
    ~LadderCollision();

    void AddLadder(float x, float y, float width, float height, int objectId);
    void ClearLadders();

    void LoadLaddersFromScene();

    bool CheckLadderOverlapWithHurtbox(float posX, float posY,
                                       float hurtboxWidth, float hurtboxHeight,
                                       float hurtboxOffsetX, float hurtboxOffsetY,
                                       float& outLadderCenterX,
                                       float& outLadderTop,
                                       float& outLadderBottom) const;

    const std::vector<Ladder>& GetLadders() const { return m_ladders; }
};


