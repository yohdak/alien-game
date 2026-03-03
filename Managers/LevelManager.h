#pragma once
#include "raylib.h"
#include <vector>
#include <string>

// Definisi Warna Map
#define COLOR_WALL      WHITE        // 255, 255, 255 (Tembok - player only)
#define COLOR_WALL_ALL  YELLOW       // 255, 255, 0   (Tembok - player + enemy)
#define COLOR_BREAKABLE RED          // 255, 0, 0     (Tembok Hancur)
#define COLOR_WATER     BLUE         // 0, 0, 255     (Air/Slow)
#define COLOR_PORTAL    GREEN        // 0, 255, 0     (Pindah Map)
// Hitam (0,0,0) = Ground/Jalan

struct DestructibleWall {
    Vector3 position;
    bool active;
    BoundingBox box;
};

struct Portal {
    Vector3 position;
    std::string targetMap; 
    BoundingBox box;
};

class LevelManager {
public:
    LevelManager();
    
    void SetCamera(Camera3D* cam);
    
    void LoadLevelFromImage(const char* imagePath);
    void Update(float dt, Vector3& playerPos, Vector3& playerVel);
    void Draw();
    int GetMapWidth() const { return mMapWidth; }
    int GetMapHeight() const { return mMapHeight; }
    float GetTileSize() const { return mTileSize; }
    int GetTileAt(Vector3 worldPos) const;

    bool CheckWallCollision(Vector3 pos, float radius);
    bool CheckEnemyWallCollision(Vector3 pos, float radius);
    bool CheckBreakableCollision(Vector3 pos, float radius, float damage);

private:
    int mMapWidth;
    int mMapHeight;
    float mTileSize;
    
    // 0=Kosong, 1=Wall(player), 2=Water, 3=Wall(player+enemy)
    std::vector<int> mCollisionGrid; 
    
    std::vector<DestructibleWall> mBreakables;
    std::vector<Portal> mPortals;
    
    Model mWallModel;     
    Model mBreakableModel;
    Model mGroundTileModel;
    
    Camera3D* mCameraRef;
};