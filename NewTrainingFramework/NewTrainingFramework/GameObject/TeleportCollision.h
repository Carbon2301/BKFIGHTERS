#pragma once

#include <vector>
#include <utility>
#include <optional>
#include <unordered_map>
#include "../../Utilities/Math.h"

struct TeleportGate {
    float x, y; 
    float width, height;
    int objectId;

    TeleportGate(float px, float py, float w, float h, int id)
        : x(px), y(py), width(w), height(h), objectId(id) {}

    float GetLeft() const { return x - width * 0.5f; }
    float GetRight() const { return x + width * 0.5f; }
    float GetBottom() const { return y - height * 0.5f; }
    float GetTop() const { return y + height * 0.5f; }
};

class TeleportCollision {
private:
    std::vector<TeleportGate> m_gates;

public:
    TeleportCollision();
    ~TeleportCollision();

    void ClearTeleports();
    void AddTeleport(float x, float y, float width, float height, int objectId);
    void LoadTeleportsFromScene();

    const std::vector<TeleportGate>& GetTeleports() const { return m_gates; }

    bool GetPairedTeleportId(int fromId, int& outToId) const;

    bool DetectEnterFromLeft(const Vector3& prevPos,
                             const Vector3& newPos,
                             float hurtboxWidth, float hurtboxHeight,
                             float hurtboxOffsetX, float hurtboxOffsetY,
                             bool movingRight,
                             int& outFromId,
                             int& outToId) const;

    bool ComputeExitPosition(int toId,
                             float currentY,
                             float hurtboxWidth, float hurtboxHeight,
                             float hurtboxOffsetX, float hurtboxOffsetY,
                             Vector3& outExitPos) const;

    bool GetPortalBounds(int id, float& outLeft, float& outRight, float& outBottom, float& outTop) const;
};


