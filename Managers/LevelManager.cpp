#include "LevelManager.h"
#include <iostream>

LevelManager::LevelManager() : mMapWidth(0), mMapHeight(0), mTileSize(2.0f), mCameraRef(nullptr) {
    // Generate simple cubes for visualization placeholders
    Mesh cube = GenMeshCube(mTileSize, mTileSize * 2.0f, mTileSize);
    mWallModel = LoadModelFromMesh(cube);
    
    Mesh crate = GenMeshCube(mTileSize, mTileSize, mTileSize);
    mBreakableModel = LoadModelFromMesh(crate);
    mBreakableModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = RED;
    
    // Ground tile: flat plane
    Mesh groundMesh = GenMeshPlane(mTileSize, mTileSize, 1, 1);
    mGroundTileModel = LoadModelFromMesh(groundMesh);
}

void LevelManager::SetCamera(Camera3D* cam) {
    mCameraRef = cam;
}

void LevelManager::LoadLevelFromImage(const char* imagePath) {
    Image mapImg = LoadImage(imagePath);
    if (mapImg.data == nullptr) {
        std::cout << "❌ FAILED TO LOAD MAP: " << imagePath << std::endl;
        return;
    }
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

            // 1. PUTIH - Wall (player only)
            if (c.r == 255 && c.g == 255 && c.b == 255) {
                mCollisionGrid[y * mMapWidth + x] = 1; // ID 1 = Tembok player-only
            }
            // 2. KUNING - Wall (player + enemy)
            else if (c.r == 255 && c.g == 255 && c.b == 0) {
                mCollisionGrid[y * mMapWidth + x] = 3; // ID 3 = Tembok player+enemy
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
                mPortals.push_back({pos, "next_level.png", box});
            }
            // else: Hitam (0,0,0) = Ground, tileID tetap 0
        }
    }

    UnloadImageColors(pixels);
    UnloadImage(mapImg);
    std::cout << "🗺️ LEVEL LOADED: " << mMapWidth << "x" << mMapHeight << std::endl;
}

void LevelManager::Update(float dt, Vector3& playerPos, Vector3& playerVel) {
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

    for (int y = gy - 1; y <= gy + 1; y++) {
        for (int x = gx - 1; x <= gx + 1; x++) {
            if (x >= 0 && x < mMapWidth && y >= 0 && y < mMapHeight) {
                int tid = mCollisionGrid[y * mMapWidth + x];
                // Player collides with both wall types (1 and 3)
                if (tid == 1 || tid == 3) {
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

bool LevelManager::CheckEnemyWallCollision(Vector3 pos, float radius) {
    int gx = (int)((pos.x + mTileSize/2) / mTileSize);
    int gy = (int)((pos.z + mTileSize/2) / mTileSize);

    for (int y = gy - 1; y <= gy + 1; y++) {
        for (int x = gx - 1; x <= gx + 1; x++) {
            if (x >= 0 && x < mMapWidth && y >= 0 && y < mMapHeight) {
                // Enemy only collides with yellow walls (ID 3)
                if (mCollisionGrid[y * mMapWidth + x] == 3) {
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
            b.active = false;
            
            int gx = (int)(b.position.x / mTileSize);
            int gy = (int)(b.position.z / mTileSize);
            if (gx >= 0 && gx < mMapWidth) mCollisionGrid[gy * mMapWidth + gx] = 0;
            
            return true;
        }
    }
    return false;
}

int LevelManager::GetTileAt(Vector3 worldPos) const {
    int gx = (int)((worldPos.x + mTileSize/2) / mTileSize);
    int gy = (int)((worldPos.z + mTileSize/2) / mTileSize);
    
    if (gx >= 0 && gx < mMapWidth && gy >= 0 && gy < mMapHeight) {
        return mCollisionGrid[gy * mMapWidth + gx];
    }
    return -1; // Out of bounds
}

void LevelManager::Draw() {
    if (mMapWidth == 0 || mMapHeight == 0) return;
    
    // Range-based culling: hanya render tile dekat kamera
    int camTileX = 0, camTileZ = 0;
    int renderRange = 35; // tiles radius
    
    if (mCameraRef) {
        camTileX = (int)(mCameraRef->target.x / mTileSize);
        camTileZ = (int)(mCameraRef->target.z / mTileSize);
    }
    
    int startX = (mCameraRef) ? (camTileX - renderRange) : 0;
    int endX   = (mCameraRef) ? (camTileX + renderRange) : mMapWidth;
    int startY = (mCameraRef) ? (camTileZ - renderRange) : 0;
    int endY   = (mCameraRef) ? (camTileZ + renderRange) : mMapHeight;
    
    // Clamp to map bounds
    if (startX < 0) startX = 0;
    if (startY < 0) startY = 0;
    if (endX > mMapWidth) endX = mMapWidth;
    if (endY > mMapHeight) endY = mMapHeight;
    
    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            int index = y * mMapWidth + x;
            int tileID = mCollisionGrid[index];
            
            // Ground tile (semua non-wall tile dapat lantai)
            if (tileID != 1 && tileID != 3) {
                Vector3 groundPos = { x * mTileSize, -0.01f, y * mTileSize };
                DrawModel(mGroundTileModel, groundPos, 1.0f, (Color){60, 60, 65, 255});
            }

            Vector3 pos = { x * mTileSize, 1.0f, y * mTileSize };

            // Draw Wall - White (player only)
            if (tileID == 1) {
                bool isBreakable = false;
                for (auto& b : mBreakables) {
                    if (b.active && (int)(b.position.x / mTileSize) == x && (int)(b.position.z / mTileSize) == y) {
                        isBreakable = true;
                        break;
                    }
                }
                if (!isBreakable) DrawModel(mWallModel, pos, 1.0f, WHITE);
            }
            // Draw Wall - Yellow (player + enemy)
            else if (tileID == 3) {
                Vector3 yellowPos = { x * mTileSize, mTileSize, y * mTileSize };
                DrawCube(yellowPos, mTileSize, mTileSize * 2.0f, mTileSize, YELLOW);
            }
            // Draw Water (Blue)
            else if (tileID == 2) {
                DrawCube({x * mTileSize, 0.05f, y * mTileSize}, mTileSize, 0.1f, mTileSize, (Color){0, 100, 220, 150});
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