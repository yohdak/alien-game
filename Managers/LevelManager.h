#pragma once
#include "raylib.h"
#include <vector>
#include <string>

// Definisi Warna Map
#define COLOR_WALL      WHITE        // 255, 255, 255 (Tembok)
#define COLOR_BREAKABLE RED          // 255, 0, 0     (Tembok Hancur)
#define COLOR_WATER     BLUE         // 0, 0, 255     (Air/Slow)
#define COLOR_PORTAL    GREEN        // 0, 255, 0     (Pindah Map)
// Abu-abu (128,128,128) dipakai untuk lantai (Ground Shader)

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
    
    // 🔥 FUNGSI BARU INI WAJIB ADA
    void SetGroundShader(Shader shader); 
    
    void LoadLevelFromImage(const char* imagePath);
    void Update(float dt, Vector3& playerPos, Vector3& playerVel);
    void Draw();

    bool CheckWallCollision(Vector3 pos, float radius);
    bool CheckBreakableCollision(Vector3 pos, float radius, float damage);

private:
    int mMapWidth;
    int mMapHeight;
    float mTileSize;
    
    // 0=Kosong, 1=Wall, 2=Water, 3=Ground
    std::vector<int> mCollisionGrid; 
    
    std::vector<DestructibleWall> mBreakables;
    std::vector<Portal> mPortals;
    
    Model mWallModel;     
    Model mBreakableModel;
    Model mGroundModel; // 🔥 Model untuk lantai
    
    Shader mRefShader;  // 🔥 Simpan referensi shader
    bool mShaderSet;
};