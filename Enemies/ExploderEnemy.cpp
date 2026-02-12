#include "ExploderEnemy.h"
#include "../Utils/MathUtils.h"
#include "raymath.h"
#include <cmath>

ExploderEnemy::ExploderEnemy(int tier, Vector3 startPos) : BaseEnemy(tier, startPos) {
    this->tier = tier;
    position = startPos;
    velocity = {0, 0, 0};
    active = true;
    mFuseTimer = -1.0f;
    mIsArmed = false;

    if (tier == 1) {
        hp = 15;            // Sengaja lemah (gampang mati)
        speed = 1.5*8.0f;       // Cepet
        radius = 0.8f;
        xpReward = 25;
        bodyColor = GREEN;
        scaleSize = 0.8f;
        mExplosionRadius = 5.0f;
        mExplosionDamage = 50.0f;
    } 
    else if (tier == 2) {
        hp = 40;
        speed = 9.0f;
        radius = 1.0f;
        xpReward = 80;
        bodyColor = DARKGREEN;
        scaleSize = 1.0f;
        mExplosionRadius = 7.0f;
        mExplosionDamage = 80.0f;
    }
    else {
        hp = 100;
        speed = 10.0f;
        radius = 1.5f;
        xpReward = 200;
        bodyColor = LIME;
        scaleSize = 1.5f;
        mExplosionRadius = 10.0f;
        mExplosionDamage = 120.0f;
    }
}

void ExploderEnemy::Update(float dt, Vector3 playerPos) {
    if (!active) return;

    UpdateFlash(dt);

    Vector3 toPlayer = Vector3Subtract(playerPos, position);
    toPlayer.y = 0;
    float distToPlayer = Vector3Length(toPlayer);

    // --- MOVEMENT (RUSH PLAYER) ---
    Vector3 dir = Vector3Normalize(toPlayer);
    position = Vector3Add(position, Vector3Scale(dir, speed * dt));

    // --- ARM FUSE (Kalau deket player) ---
    if (distToPlayer < mExplosionRadius * 1.5f && !mIsArmed) {
        mIsArmed = true;
        mFuseTimer = 1.0f; // Fuse 1 detik
    }

    // --- COUNTDOWN ---
    if (mIsArmed) {
        mFuseTimer -= dt;
        if (mFuseTimer <= 0) {
            // EXPLODE!
            active = false; // Mati setelah meledak
        }
    }

    // Gravity
    if (position.y > 0) position.y -= 10.0f * dt;
    if (position.y < 0) position.y = 0;
}

bool ExploderEnemy::ShouldExplode(Vector3 playerPos) {
    if (!mIsArmed) return false;
    if (mFuseTimer > 0) return false;
    
    return true;
}

void ExploderEnemy::Draw(Model& slimeModel, Model& cubeModel, Model& magnetModel, 
                         Model& shadowPlane, Camera3D cam, Vector3 playerPos) {
    if (!active) return;

    // Shadow
    float shadowScale = radius * 2.2f;
    DrawModelEx(shadowPlane, {position.x, 0.02f, position.z}, 
                {0, 1, 0}, 0.0f, {shadowScale, 1.0f, shadowScale}, 
                ColorAlpha(BLACK, 0.4f));

    // Body (Sphere = Bom)
    Vector3 drawPos = position;
    drawPos.y += radius;

    // 1. Logika Blink Merah (Peringatan mau meledak)
    Color currentColor = bodyColor;
    if (mIsArmed) {
        float blinkSpeed = 10.0f - (mFuseTimer * 8.0f); // Makin cepat pas mau meledak
        if ((int)(GetTime() * blinkSpeed) % 2 == 0) {
            currentColor = RED;
        }
    }

    // 2. 🔥 FIX: HIT EFFECT (Override warna ledakan)
    // Jika kena hit, warna jadi PUTIH (menimpa warna merah/body)
    Color finalColor = GetRenderColor(currentColor);

    // 3. Draw Sphere
    DrawSphere(drawPos, scaleSize, finalColor);
    DrawSphereWires(drawPos, scaleSize, 8, 8, WHITE);

    // Fuse visual (Sumbu bom)
    if (mIsArmed) {
        Vector3 fuseStart = drawPos;
        fuseStart.y += scaleSize;
        Vector3 fuseEnd = fuseStart;
        fuseEnd.y += 0.5f;
        
        // Sumbu ikut jadi putih kalau kena hit biar konsisten
        Color fuseColor = (flashTimer > 0) ? WHITE : ORANGE;
        
        DrawLine3D(fuseStart, fuseEnd, fuseColor);
        DrawSphere(fuseEnd, 0.1f, YELLOW); // Spark tetap kuning
    }
}