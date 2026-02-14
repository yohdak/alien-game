#pragma once
#include "raylib.h"
#include <vector>
#include <string>

// Definisi Warna Map (Sesuaikan dengan Paint Anda)
#define COLOR_WALL      WHITE        // 255, 255, 255 (Tembok Mati)
#define COLOR_BREAKABLE RED          // 255, 0, 0     (Tembok Hancur)
#define COLOR_WATER     BLUE         // 0, 0, 255     (Air/Slow)
#define COLOR_PORTAL    GREEN        // 0, 255, 0     (Pindah Map)
#define COLOR_GROUND    GRAY         // 128, 128, 128 (Jalanan - Opsional)

struct DestructibleWall {
    Vector3 position;
    bool active;
    BoundingBox box;
};

struct Portal {
    Vector3 position;
    std::string targetMap; // Nama file map selanjutnya
    BoundingBox box;
};

class LevelManager {
public:
    LevelManager();
    void LoadLevelFromImage(const char* imagePath);
    void Update(float dt, Vector3& playerPos, Vector3& playerVel); // Handle physics air & portal
    void Draw();

    // Collision System
    bool CheckWallCollision(Vector3 pos, float radius);
    bool CheckBreakableCollision(Vector3 pos, float radius, float damage); // Return true jika hancur

private:
    int mMapWidth;
    int mMapHeight;
    float mTileSize; // Ukuran 1 pixel = berapa unit dunia (misal 2.0f)
    
    // Grid Data (0 = Kosong, 1 = Wall, 2 = Water)
    std::vector<int> mCollisionGrid; 
    
    // Objek Dinamis
    std::vector<DestructibleWall> mBreakables;
    std::vector<Portal> mPortals;
    
    // Model untuk visual
    Model mWallModel;     
    Model mBreakableModel;
};