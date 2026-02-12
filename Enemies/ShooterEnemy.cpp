#include "ShooterEnemy.h"
#include "../Utils/MathUtils.h"
#include "raymath.h"
#include <cmath>
#include <algorithm>

ShooterEnemy::ShooterEnemy(int tier, Vector3 startPos) : BaseEnemy(tier, startPos) {
    this->tier = tier;
    position = startPos;
    velocity = {0, 0, 0};
    active = true;
    mShootTimer = 0.0f;

    // 🔥 STATS BASED ON TIER
    if (tier == 1) {
        hp = 30;
        speed = 3.0f;           // Lambat (kiting enemy)
        radius = 1.0f;
        xpReward = 30;
        bodyColor = ORANGE;
        scaleSize = 1.0f;
        mShootCooldown = 2.0f;  // Nembak tiap 2 detik
        mShootRange = 20.0f;    // Range 20 meter
    } 
    else if (tier == 2) {
        hp = 80;
        speed = 2.5f;
        radius = 1.3f;
        xpReward = 100;
        bodyColor = YELLOW;
        scaleSize = 1.3f;
        mShootCooldown = 1.5f;  // Lebih cepet
        mShootRange = 25.0f;
    }
    else {
        hp = 200;
        speed = 2.0f;
        radius = 1.8f;
        xpReward = 300;
        bodyColor = GOLD;
        scaleSize = 1.8f;
        mShootCooldown = 1.0f;  // Rapid fire
        mShootRange = 30.0f;
    }
}

void ShooterEnemy::Update(float dt, Vector3 playerPos) {
    if (!active) return;

    UpdateFlash(dt);

    // --- 1. MOVEMENT (Keep distance from player) ---
    Vector3 toPlayer = Vector3Subtract(playerPos, position);
    toPlayer.y = 0;
    float distToPlayer = Vector3Length(toPlayer);
    
    // Kiting behavior: Jaga jarak optimal (15 meter)
    float optimalRange = 15.0f;
    
    if (distToPlayer < optimalRange) {
        // Terlalu deket, mundur
        Vector3 dir = Vector3Normalize(toPlayer);
        position = Vector3Subtract(position, Vector3Scale(dir, speed * dt));
    } 
    else if (distToPlayer > mShootRange) {
        // Terlalu jauh, maju
        Vector3 dir = Vector3Normalize(toPlayer);
        position = Vector3Add(position, Vector3Scale(dir, speed * dt));
    }
    // Else: Perfect range, stay still

    // --- 2. SHOOTING ---
    mShootTimer -= dt;
    
    if (distToPlayer <= mShootRange && mShootTimer <= 0) {
        Shoot(playerPos);
        mShootTimer = mShootCooldown;
    }

    // --- 3. UPDATE BULLETS ---
    for (auto& b : mBullets) {
        if (!b.active) continue;
        
        b.position = Vector3Add(b.position, Vector3Scale(b.direction, b.speed * dt));
        b.lifeTime -= dt;
        
        if (b.lifeTime <= 0) b.active = false;
        
        // Cek kena tanah
        if (b.position.y <= 0) b.active = false;
    }

    // Cleanup
    mBullets.erase(
        std::remove_if(mBullets.begin(), mBullets.end(),
            [](const EnemyBullet& b) { return !b.active; }),
        mBullets.end()
    );

    // Gravity
    if (position.y > 0) position.y -= 10.0f * dt;
    if (position.y < 0) position.y = 0;
}

void ShooterEnemy::Shoot(Vector3 targetPos) {
    EnemyBullet b;
    b.position = position;
    b.position.y += radius * 0.7f; // Spawn dari tengah body
    
    Vector3 dir = Vector3Subtract(targetPos, b.position);
    dir.y = 0; // Shoot horizontal
    b.direction = Vector3Normalize(dir);
    
    b.speed = 3*15.0f;
    b.damage = 10.0f;
    b.radius = 0.1f;
    b.active = true;
    b.lifeTime = 5.0f/3.0f; // 1.667 seconds
    
    mBullets.push_back(b);
}

void ShooterEnemy::Draw(Model& slimeModel, Model& cubeModel, Model& magnetModel, 
                        Model& shadowPlane, Camera3D cam, Vector3 playerPos) {
    if (!active) return;

    // --- 1. SHADOW (Tetap) ---
    float shadowScale = radius * 2.2f;
    DrawModelEx(shadowPlane, {position.x, 0.02f, position.z}, 
                {0, 1, 0}, 0.0f, {shadowScale, 1.0f, shadowScale}, 
                ColorAlpha(BLACK, 0.4f));

    // --- 2. BODY ROTATION (Tetap) ---
    Vector3 dir = Vector3Subtract(playerPos, position);
    float rotationY = atan2f(dir.x, dir.z) * RAD2DEG;

    // --- 3. BODY DRAW (Tetap + Hit Effect) ---
    Vector3 scale = {scaleSize * 1.5f, scaleSize * 0.6f, scaleSize * 0.8f};
    Vector3 drawPos = position;
    drawPos.y += scale.y * 0.5f;

    Color finalColor = GetRenderColor(bodyColor);

    cubeModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = finalColor;
    DrawModelEx(cubeModel, drawPos, {0, 1, 0}, rotationY, scale, WHITE);
    cubeModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;

    // --- 4. BARREL (Tetap) ---
    Color barrelColor = (flashTimer > 0) ? WHITE : DARKGRAY;
    Vector3 barrelOffset = Vector3RotateByAxisAngle({0, 0, scaleSize * 1.0f}, {0, 1, 0}, rotationY * DEG2RAD);
    Vector3 barrelPos = Vector3Add(drawPos, barrelOffset);
    DrawCylinder(barrelPos, 0.15f, 0.15f, 0.5f, 8, barrelColor);

    // --- 5. 🔥 UPGRADED BULLET VISUALS ---
    
    // Aktifkan Additive Blending (Warna bertumpuk jadi makin terang/glowing)
    BeginBlendMode(BLEND_ADDITIVE); 

    for (const auto& b : mBullets) {
        if (!b.active) continue;

        // Tentukan warna bullet (lebih terang dari body musuh)
        Color glowColor = bodyColor;
        Color coreColor = WHITE;

        // Hitung posisi Ekor (Trail)
        // Ekor memanjang ke BELAKANG arah gerak peluru
        float trailLength = 1.5f; // Panjang ekor visual
        Vector3 tailPos = Vector3Subtract(b.position, Vector3Scale(b.direction, trailLength));

        // A. Gambar Glow Sphere (Aura luar)
        DrawSphere(b.position, b.radius * 1.2f, ColorAlpha(glowColor, 0.4f));

        // B. Gambar Core Cylinder (Badan peluru memanjang)
        // Dari Ekor (kecil) ke Kepala (besar)
        DrawCylinderEx(tailPos, b.position, b.radius * 0.1f, b.radius * 0.6f, 6, glowColor);

        // C. Gambar Hot Core Line (Inti laser putih di tengah)
        DrawLine3D(tailPos, b.position, coreColor);
        
        // D. Spark di kepala peluru (Titik impact/depan)
        DrawSphere(b.position, b.radius * 0.4f, coreColor);
    }

    EndBlendMode(); // Matikan blending additive
}