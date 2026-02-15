#include "LevelManager.h"
#include <iostream>
#include "rlgl.h" // ✅ Required for direct drawing

LevelManager::LevelManager() : mMapWidth(0), mMapHeight(0), mTileSize(2.0f) {
    // Generate simple cubes for visualization placeholders
    Mesh cube = GenMeshCube(mTileSize, mTileSize * 2.0f, mTileSize);
    mWallModel = LoadModelFromMesh(cube);
    
    Mesh crate = GenMeshCube(mTileSize, mTileSize, mTileSize);
    mBreakableModel = LoadModelFromMesh(crate);
    mBreakableModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = RED;
    
    mHasCollisionMap = false;
    mCollisionPixels = nullptr;
    mHasMapTexture = false;
}

LevelManager::~LevelManager() {
    if (mHasCollisionMap) {
        UnloadImageColors(mCollisionPixels);
        UnloadImage(mCollisionMap);
    }
    if (mHasMapTexture) {
        UnloadTexture(mMapTexture);
    }
}

void LevelManager::LoadCollisionMap(const char* imagePath) {
    if (mHasCollisionMap) {
        UnloadImageColors(mCollisionPixels);
        UnloadImage(mCollisionMap);
    }
    if (mHasMapTexture) {
        UnloadTexture(mMapTexture);
        mHasMapTexture = false;
    }

    mCollisionMap = LoadImage(imagePath);
    if (mCollisionMap.data != nullptr) {
        mCollisionPixels = LoadImageColors(mCollisionMap);
        mHasCollisionMap = true;
        
        // Update dimensions
        mMapWidth = mCollisionMap.width;
        mMapHeight = mCollisionMap.height;

        // Load Texture for Visualization
        mMapTexture = LoadTextureFromImage(mCollisionMap);
        SetTextureFilter(mMapTexture, TEXTURE_FILTER_POINT); // Pixelated look
        mHasMapTexture = true;
        
        std::cout << "🗺️ COLLISION MAP LOADED: " << mCollisionMap.width << "x" << mCollisionMap.height << std::endl;
    } else {
        std::cout << "❌ FAILED TO LOAD COLLISION MAP: " << imagePath << std::endl;
    }
}

bool LevelManager::IsPixelCollision(Vector3 pos, float radius) {
    if (!mHasCollisionMap) return false;

    // Asumsi Plane 100x100 (dari -50 sampai 50)
    // Map Image Koordinat: (0,0) di Top-Left
    // World (0,0) di tengah map image
    
    // Mapping [-50, 50] -> [0, Width]
    float mapSize = 100.0f; // Sesuai GenMeshPlane(100, 100)
    float halfSize = mapSize / 2.0f;
    
    // Normalisasi (0.0 - 1.0)
    float u = (pos.x + halfSize) / mapSize;
    float v = (pos.z + halfSize) / mapSize;
    
    int tx = (int)(u * mCollisionMap.width);
    int ty = (int)(v * mCollisionMap.height);
    
    // Strict Boundary Check (Diluar Map = Tembok / Void)
    // Jika koordinat pixel diluar range image, langsung return true (collision)
    if (tx < 0 || tx >= mCollisionMap.width || ty < 0 || ty >= mCollisionMap.height) {
        return true; 
    }

    // Ambil warna pixel
    Color c = mCollisionPixels[ty * mCollisionMap.width + tx];

    // Logika: Warna Gelap = Tembok / Void
    // Misal: R,G,B < 80 dianggap tembok
    if (c.r < 80 && c.g < 80 && c.b < 80) {
        return true; 
    }
    
    return false;
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
    // 1. Draw Map Surface
    if (mHasMapTexture) {
        // Draw textured quad directly using RLGL
        rlSetTexture(mMapTexture.id);
        rlBegin(RL_QUADS);
            rlColor4ub(255, 255, 255, 255);
            rlNormal3f(0.0f, 1.0f, 0.0f);

            // Top-Left (-50, -50) -> UV(0,0)
            rlTexCoord2f(0.0f, 0.0f); rlVertex3f(-50.0f, 0.0f, -50.0f);
            
            // Bottom-Left (-50, 50) -> UV(0,1)
            rlTexCoord2f(0.0f, 1.0f); rlVertex3f(-50.0f, 0.0f, 50.0f);
            
            // Bottom-Right (50, 50) -> UV(1,1)
            rlTexCoord2f(1.0f, 1.0f); rlVertex3f(50.0f, 0.0f, 50.0f);
            
            // Top-Right (50, -50) -> UV(1,0)
            rlTexCoord2f(1.0f, 0.0f); rlVertex3f(50.0f, 0.0f, -50.0f);
        rlEnd();
        rlSetTexture(0);
    } 
    else {
        // Fallback: Default Loop (for grid based levels if any)
        for (int y = 0; y < mMapHeight; y++) {
            for (int x = 0; x < mMapWidth; x++) {
                int index = y * mMapWidth + x;
                int tileID = mCollisionGrid[index];
                Vector3 pos = { x * mTileSize, 1.0f, y * mTileSize };

                if (tileID == 1) DrawModel(mWallModel, pos, 1.0f, WHITE);
                else if (tileID == 2) DrawCube(pos, mTileSize, 0.1f, mTileSize, (Color){0, 121, 241, 150});
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

Vector3 LevelManager::GetPlayerSpawnPoint() {
    if (!mHasCollisionMap) return {0, 0, 0};

    // Scan for Blue Pixel (Player Spawn)
    // Blue in Raylib: 0, 121, 241, 255 (Default BLUE)
    // Our Editor: COL_PLAYER = BLUE
    for (int y = 0; y < mCollisionMap.height; y++) {
        for (int x = 0; x < mCollisionMap.width; x++) {
            Color c = mCollisionPixels[y * mCollisionMap.width + x];
            
            // Check for Blue-ish pixel
            if (c.b > 200 && c.r < 100 && c.g < 150) {
                 float u = (float)x / mCollisionMap.width;
                 float v = (float)y / mCollisionMap.height;
                 
                 // Map UV to World Coordinates (Plane 100x100)
                 float worldX = (u - 0.5f) * 100.0f;
                 float worldZ = (v - 0.5f) * 100.0f;
                 
                 return { worldX, 0.0f, worldZ };
            }
        }
    }
    return {0, 0, 0};
}

std::vector<Vector3> LevelManager::GetEnemySpawnPoints() {
    std::vector<Vector3> spawns;
    if (!mHasCollisionMap) return spawns;

    for (int y = 0; y < mCollisionMap.height; y++) {
        for (int x = 0; x < mCollisionMap.width; x++) {
            Color c = mCollisionPixels[y * mCollisionMap.width + x];
            
            // Check for Red-ish pixel (Enemy Spawn)
            if (c.r > 200 && c.g < 100 && c.b < 100) {
                 float u = (float)x / mCollisionMap.width;
                 float v = (float)y / mCollisionMap.height;
                 
                 float worldX = (u - 0.5f) * 100.0f;
                 float worldZ = (v - 0.5f) * 100.0f;
                 
                 spawns.push_back({ worldX, 0.0f, worldZ });
            }
        }
    }
    return spawns;
}