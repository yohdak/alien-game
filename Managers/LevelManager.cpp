#include "LevelManager.h"
#include <iostream>

LevelManager::LevelManager() : mMapWidth(0), mMapHeight(0), mTileSize(2.0f) {
    // Generate simple cubes for visualization placeholders
    Mesh cube = GenMeshCube(mTileSize, mTileSize * 2.0f, mTileSize);
    mWallModel = LoadModelFromMesh(cube);
    
    Mesh crate = GenMeshCube(mTileSize, mTileSize, mTileSize);
    mBreakableModel = LoadModelFromMesh(crate);
    mBreakableModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = RED;
}

void LevelManager::LoadLevelFromImage(const char* imagePath) {
    Image mapImg = LoadImage(imagePath);
    mMapWidth = mapImg.width;
    mMapHeight = mapImg.height;
    
    mCollisionGrid.clear();
    mCollisionGrid.resize(mMapWidth * mMapHeight, 0); // 0 = Jalan
    mBreakables.clear();
    mPortals.clear();

    Color* pixels = LoadImageColors(mapImg);

    for (int y = 0; y < mMapHeight; y++) {
        for (int x = 0; x < mMapWidth; x++) {
            Color c = pixels[y * mMapWidth + x];
            Vector3 pos = { x * mTileSize, 0.0f, y * mTileSize };

            // 1. PUTIH - Wall Physics
            if (c.r == 255 && c.g == 255 && c.b == 255) {
                mCollisionGrid[y * mMapWidth + x] = 1; // ID 1 = Tembok
            }
            // 2. BIRU - Water Physics
            else if (c.b == 255 && c.r == 0 && c.g == 0) {
                mCollisionGrid[y * mMapWidth + x] = 2; // ID 2 = Air
            }
            // 3. MERAH - Breakable Wall
            else if (c.r == 255 && c.g == 0 && c.b == 0) {
                BoundingBox box = {
                    {pos.x - 1, 0, pos.z - 1},
                    {pos.x + 1, 2, pos.z + 1}
                };
                mBreakables.push_back({pos, true, box});
                mCollisionGrid[y * mMapWidth + x] = 1; // Tetap dianggap tembok sampai hancur
            }
            // 4. HIJAU - Portal
            else if (c.g == 255 && c.r == 0 && c.b == 0) {
                BoundingBox box = {
                    {pos.x - 1, 0, pos.z - 1},
                    {pos.x + 1, 2, pos.z + 1}
                };
                mPortals.push_back({pos, "next_level.png", box}); // Logic nama map nanti dikembangkan
            }
        }
    }

    UnloadImageColors(pixels);
    UnloadImage(mapImg);
    std::cout << "🗺️ LEVEL LOADED: " << mMapWidth << "x" << mMapHeight << std::endl;
}

void LevelManager::Update(float dt, Vector3& playerPos, Vector3& playerVel) {
    // Konversi posisi player ke Grid Coordinate
    int gx = (int)((playerPos.x + mTileSize/2) / mTileSize);
    int gy = (int)((playerPos.z + mTileSize/2) / mTileSize);

    // Bounds check
    if (gx >= 0 && gx < mMapWidth && gy >= 0 && gy < mMapHeight) {
        int tileID = mCollisionGrid[gy * mMapWidth + gx];

        // --- LOGIC BIRU (AIR) ---
        if (tileID == 2) {
            // Physics air: Lambatkan gerakan & sedikit buoyancy
            playerVel.x *= 0.90f; // Drag tinggi
            playerVel.z *= 0.90f; 
        }
    }
    
    // --- LOGIC HIJAU (PORTAL) ---
    for (auto& p : mPortals) {
        if (CheckCollisionBoxSphere(p.box, playerPos, 0.5f)) {
            std::cout << "🌀 PORTAL TRIGGERED! Going to: " << p.targetMap << std::endl;
            // TODO: Panggil fungsi ganti level di Game.cpp
        }
    }
}

bool LevelManager::CheckWallCollision(Vector3 pos, float radius) {
    int gx = (int)((pos.x + mTileSize/2) / mTileSize);
    int gy = (int)((pos.z + mTileSize/2) / mTileSize);

    // Cek 3x3 area sekitar player untuk tabrakan dinding statis
    for (int y = gy - 1; y <= gy + 1; y++) {
        for (int x = gx - 1; x <= gx + 1; x++) {
            if (x >= 0 && x < mMapWidth && y >= 0 && y < mMapHeight) {
                // Jika Grid bernilai 1 (Tembok) atau tile Merah yg belum hancur
                if (mCollisionGrid[y * mMapWidth + x] == 1) {
                    Vector3 tilePos = { x * mTileSize, 0, y * mTileSize };
                    BoundingBox tileBox = {
                        {tilePos.x - mTileSize/2, 0, tilePos.z - mTileSize/2},
                        {tilePos.x + mTileSize/2, 4, tilePos.z + mTileSize/2}
                    };
                    if (CheckCollisionBoxSphere(tileBox, pos, radius)) return true;
                }
            }
        }
    }
    return false;
}

bool LevelManager::CheckBreakableCollision(Vector3 pos, float radius, float damage) {
    for (auto& b : mBreakables) {
        if (!b.active) continue;
        if (CheckCollisionBoxSphere(b.box, pos, radius)) {
            b.active = false; // Hancur instan (bisa diubah jadi HP)
            
            // Update Grid jadi 0 (Jalan)
            int gx = (int)(b.position.x / mTileSize);
            int gy = (int)(b.position.z / mTileSize);
            if (gx >= 0 && gx < mMapWidth) mCollisionGrid[gy * mMapWidth + gx] = 0;
            
            return true;
        }
    }
    return false;
}

void LevelManager::Draw() {
    // Optimasi: Hanya gambar tile di sekitar kamera (Culling sederhana)
    // Untuk sekarang gambar semua loop sederhana
    
    for (int y = 0; y < mMapHeight; y++) {
        for (int x = 0; x < mMapWidth; x++) {
            int index = y * mMapWidth + x;
            int tileID = mCollisionGrid[index];
            Vector3 pos = { x * mTileSize, 1.0f, y * mTileSize };

            // Draw Wall (White)
            if (tileID == 1) {
                // Cek apakah ini breakable wall? Kalau bukan, gambar tembok biasa
                bool isBreakable = false;
                // (Cara malas: cek list breakable, idealnya pake ID beda di grid)
                if (!isBreakable) DrawModel(mWallModel, pos, 1.0f, WHITE);
            }
            // Draw Water (Blue)
            else if (tileID == 2) {
                DrawCube(pos, mTileSize, 0.1f, mTileSize, (Color){0, 121, 241, 150});
            }
        }
    }

    // Draw Breakables
    for (auto& b : mBreakables) {
        if (b.active) DrawModel(mBreakableModel, b.position, 1.0f, WHITE);
    }
    
    // Draw Portals
    for (auto& p : mPortals) {
        DrawCubeWires(p.position, 2.0f, 4.0f, 2.0f, GREEN);
        DrawCube(p.position, 1.0f, 3.0f, 1.0f, (Color){0, 255, 0, 100});
    }
}