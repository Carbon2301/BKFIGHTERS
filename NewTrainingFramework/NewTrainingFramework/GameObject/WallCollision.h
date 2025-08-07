#pragma once
#include "../../Utilities/Math.h"
#include <vector>

class Object;

// Struct để đại diện cho một tường
struct Wall {
    float x, y;           // Vị trí tâm của tường
    float width, height;  // Kích thước của tường
    int objectId;         // ID của object tường trong scene
    
    Wall(float px, float py, float w, float h, int id) 
        : x(px), y(py), width(w), height(h), objectId(id) {}
        
    // Lấy các cạnh của tường (dùng cho collision detection)
    float GetLeft() const { return x - width / 2.0f; }
    float GetRight() const { return x + width / 2.0f; }
    float GetBottom() const { return y - height / 2.0f; }
    float GetTop() const { return y + height / 2.0f; }
};

class WallCollision {
private:
    std::vector<Wall> m_walls;
    
    // Helper methods for collision detection
    bool CheckAABBCollision(float aLeft, float aRight, float aBottom, float aTop,
                           float bLeft, float bRight, float bBottom, float bTop) const;
    
    Vector3 ResolveCollision(const Vector3& currentPos, const Vector3& newPos,
                           float hurtboxWidth, float hurtboxHeight,
                           float hurtboxOffsetX, float hurtboxOffsetY,
                           const Wall& wall) const;

public:
    WallCollision();
    ~WallCollision();
    
    // Quản lý tường
    void AddWall(float x, float y, float width, float height, int objectId);
    void AddWallFromObject(int objectId);  // Tự động lấy thông tin từ SceneManager
    void ClearWalls();
    void LoadWallsFromScene();  // Load tất cả tường từ scene hiện tại
    
    // Collision detection và resolution
    bool CheckWallCollision(const Vector3& position, 
                           float hurtboxWidth, float hurtboxHeight,
                           float hurtboxOffsetX, float hurtboxOffsetY) const;
    
    Vector3 ResolveWallCollision(const Vector3& currentPos, const Vector3& newPos,
                               float hurtboxWidth, float hurtboxHeight,
                               float hurtboxOffsetX, float hurtboxOffsetY) const;
    
    // Character-specific methods (using raw data to avoid dependencies)
    bool CheckCharacterWallCollision(const Vector3& position, 
                                   float hurtboxWidth, float hurtboxHeight,
                                   float hurtboxOffsetX, float hurtboxOffsetY) const;
    Vector3 ResolveCharacterWallCollision(const Vector3& currentPos, const Vector3& newPos,
                                        float hurtboxWidth, float hurtboxHeight,
                                        float hurtboxOffsetX, float hurtboxOffsetY) const;
    
    // Getters
    const std::vector<Wall>& GetWalls() const { return m_walls; }
    size_t GetWallCount() const { return m_walls.size(); }
};
