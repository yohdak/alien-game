#include "Rat.h"
#include "../Utils/MathUtils.h"
#include "raymath.h"
#include <cmath>

Rat::Rat(int tierInput, Vector3 startPos) : BaseEnemy(tierInput, startPos) {
    tier = tierInput;
    position = startPos;
    velocity = {0, 0, 0};
    active = true;
    mAnimTimer = 0.0f;
    mJumpTimer = GetRandomFloat(1.0f, 3.0f);
    mJumpVelocity = 0.0f;
    mIsJumping = false;

    // 🔥 STATS + MODEL + WARNA BERDASARKAN TIER & RNG
    if (tier == 1) {
        int roll = GetRandomValue(0, 100);

        if (roll < 40) { 
            // VARIANT 1: TIKUS PUTIH (Speedy)
            hp = 15;
            speed = 9.0f;
            radius = 0.6f;
            scaleSize = 0.5f;
            bodyColor = RAYWHITE;
            xpReward = 8;
            modelType = RatModelType::NORMAL; // ✅ Pake Rat.glb
        } 
        else if (roll < 80) { 
            // VARIANT 2: HAMSTER OREN (Balanced)
            hp = 25;
            speed = 6.5f;
            radius = 0.9f;
            scaleSize = 0.8f;
            bodyColor = ORANGE; // Tint oren (biar makin oren)
            xpReward = 20;
            modelType = RatModelType::HAMSTER; // ✅ Pake Hamster.glb
        } 
        else { 
            // VARIANT 3: TIKUS HITAM (Tank Kecil)
            hp = 80;
            speed = 4.0f;
            radius = 1.3f;
            scaleSize = 1.1f;
            bodyColor = (Color){ 30, 30, 30, 255 }; // Hitam
            xpReward = 50;
            modelType = RatModelType::NORMAL; // ✅ Pake Rat.glb (tint hitam)
        }
    } 
    else if (tier == 2) {
        int roll = GetRandomValue(0, 100);
        
        if (roll < 50) {
            // TIKUS BIRU (Elite Rat)
            hp = 120;
            speed = 5.5f;
            radius = 1.5f;
            scaleSize = 1.2f;
            bodyColor = BLUE;
            xpReward = 120;
            modelType = RatModelType::NORMAL; // ✅ Rat.glb (tint biru)
        } else {
            // HAMSTER GEDE (Elite Hamster)
            hp = 150;
            speed = 4.5f;
            radius = 1.8f;
            scaleSize = 1.5f;
            bodyColor = GOLD;
            xpReward = 150;
            modelType = RatModelType::HAMSTER; // ✅ Hamster.glb (tint emas)
        }
    }
    else {
        // TIER 3+: SPINY MOUSE BOSS (RAT KING)
        hp = 600;
        speed = 3.0f;
        radius = 3.0f;
        scaleSize = 2.5f; // GEDE BANGET
        bodyColor = (Color){ 180, 50, 50, 255 }; // Merah gelap
        xpReward = 1200;
        modelType = RatModelType::SPINY_BOSS; // ✅ Spiny_mouse.glb
    }
}

void Rat::Update(float dt, Vector3 playerPos) {
    if (!active) return;

    // --- MOVEMENT ---
    Vector3 dir = Vector3Subtract(playerPos, position);
    dir.y = 0;
    dir = Vector3Normalize(dir);
    position = Vector3Add(position, Vector3Scale(dir, speed * dt));

    // --- JUMP LOGIC ---
    mJumpTimer -= dt;
    
    if (!mIsJumping && mJumpTimer <= 0) {
        mIsJumping = true;
        mJumpVelocity = 4.0f;
        mJumpTimer = GetRandomFloat(2.0f, 4.0f);
    }

    if (mIsJumping) {
        position.y += mJumpVelocity * dt;
        mJumpVelocity -= 15.0f * dt;

        if (position.y <= 0.0f) {
            position.y = 0.0f;
            mIsJumping = false;
        }
    }

    mAnimTimer += dt * speed * 2.5f;
}

// ✅ DRAW FALLBACK (Pake Cube kalau model belum ada)
void Rat::Draw(Model& slimeModel, Model& cubeModel, Model& magnetModel, Model& shadowPlane, 
               Camera3D cam, Vector3 playerPos) {
    if (!active) return;

    float dx = playerPos.x - position.x;
    float dz = playerPos.z - position.z;
    float rotationY = atan2f(dx, dz) * RAD2DEG;
    float wobble = sinf(mAnimTimer) * 5.0f;

    // Shadow
    float shadowScale = radius * 2.2f;
    DrawModelEx(shadowPlane, {position.x, 0.02f, position.z}, 
                {0, 1, 0}, 0.0f, {shadowScale, 1.0f, shadowScale}, 
                ColorAlpha(BLACK, 0.4f));

    // Body (Cube placeholder)
    Vector3 bodyScale = { scaleSize * 0.5f, scaleSize * 0.4f, scaleSize * 1.0f };
    Vector3 drawPos = position;
    drawPos.y += bodyScale.y * 0.5f;

    cubeModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = bodyColor;
    DrawModelEx(cubeModel, drawPos, {0, 1, 0}, rotationY + wobble, bodyScale, WHITE);
}

// ✅ DRAW DENGAN MODEL TIKUS ASLI (BARU!)
void Rat::DrawWithRatModels(Model& ratModel, Model& hamsterModel, Model& spinyModel, 
                            Model& shadowPlane, Vector3 playerPos) { 
    if (!active) return;

    // --- 1. SHADOW ---
    float shadowScale = radius * 2.2f;
    DrawModelEx(shadowPlane, {position.x, 0.02f, position.z}, 
                {0, 1, 0}, 0.0f, {shadowScale, 1.0f, shadowScale}, 
                ColorAlpha(BLACK, 0.4f));

    // --- 2. PILIH MODEL BERDASARKAN TYPE ---
    Model* currentModel = &ratModel;
    
    switch (modelType) {
        case RatModelType::NORMAL:
            currentModel = &ratModel;
            break;
        case RatModelType::HAMSTER:
            currentModel = &hamsterModel;
            break;
        case RatModelType::SPINY_BOSS:
            currentModel = &spinyModel;
            break;
    }

    Vector3 drawPos = position;
    drawPos.y += radius; // ✅ Naikin sesuai radius (biar gak tenggelam)


    // --- 3. ROTASI (MENGHADAP PLAYER) ---
    Vector3 dir = Vector3Subtract(playerPos, position);
    dir.y = 0;
    float rotationY = atan2f(dir.x, dir.z) * RAD2DEG;

    // Wobble animation (jalan goyang-goyang)
    float wobble = sinf(mAnimTimer) * 3.0f;

    // --- 4. SCALE (Ukuran model) ---
    // Model .glb biasanya punya skala sendiri, jadi kita adjust
    float modelScale = scaleSize * 0.01f*2.5; // Adjust sesuai ukuran model lu
    Vector3 scale = {modelScale, modelScale, modelScale};


    // --- 5. DRAW MODEL + TINT COLOR ---
    // PENTING: Tint color bakal ngubah warna model
    DrawModelEx(*currentModel, drawPos, {0, 1, 0}, rotationY + wobble, scale, bodyColor);
}
