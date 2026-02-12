#include "ChargerEnemy.h"
#include "../Utils/MathUtils.h"
#include "raymath.h"
#include <cmath>

ChargerEnemy::ChargerEnemy(int tier, Vector3 startPos) : BaseEnemy(tier, startPos) {
    this->tier = tier;
    position = startPos;
    velocity = {0, 0, 0};
    active = true;
    mState = ChargerState::IDLE;
    mStateTimer = 0.0f;
    mDashDirection = {0, 0, 0};

    if (tier == 1) {
        hp = 40;
        speed = 6.0f;
        radius = 1.2f;
        xpReward = 40;
        bodyColor = PURPLE;
        scaleSize = 1.2f;
        mDashSpeed = 4*25.0f;
    } 
    else if (tier == 2) {
        hp = 120;
        speed = 7.0f;
        radius = 1.6f;
        xpReward = 150;
        bodyColor = VIOLET;
        scaleSize = 1.6f;
        mDashSpeed = 4*35.0f;
    }
    else {
        hp = 300;
        speed = 8.0f;
        radius = 2.2f;
        xpReward = 400;
        bodyColor = MAGENTA;
        scaleSize = 2.2f;
        mDashSpeed = 4*45.0f;
    }
}

void ChargerEnemy::Update(float dt, Vector3 playerPos) {
    if (!active) return;

    UpdateFlash(dt);

    Vector3 toPlayer = Vector3Subtract(playerPos, position);
    toPlayer.y = 0;
    float distToPlayer = Vector3Length(toPlayer);

    switch (mState) {
        case ChargerState::IDLE: {
            // Gerak normal ke player
            Vector3 dir = Vector3Normalize(toPlayer);
            position = Vector3Add(position, Vector3Scale(dir, speed * dt));

            // Kalau deket (10-20 meter), mulai charge up
            if (distToPlayer > 7.5f && distToPlayer < 15.0f) {
                mState = ChargerState::CHARGING_UP;
                mStateTimer = 0.2f; // Telegraph 1 detik
                mDashDirection = dir;
            }
            break;
        }

        case ChargerState::CHARGING_UP: {
            // STOP + WARNING VISUAL
            mStateTimer -= dt;
            if (mStateTimer <= 0) {
                mState = ChargerState::DASHING;
                mStateTimer = 0.16f; 
            }
            break;
        }

        case ChargerState::DASHING: {
            // DASH SUPER CEPAT
            position = Vector3Add(position, Vector3Scale(mDashDirection, mDashSpeed * dt));
            
            mStateTimer -= dt;
            if (mStateTimer <= 0) {
                mState = ChargerState::COOLDOWN;
                mStateTimer = 1.2f; // Cooldown 2 detik
            }
            break;
        }

        case ChargerState::COOLDOWN: {
            // 🔥 LOGIC PERLAMBATAN (DRIFTING)
            // 0.5 detik pertama cooldown dipakai untuk pengereman
            float totalCooldown = 2.0f;
            float slideDuration = 0.5f; 
            float timeInCooldown = totalCooldown - mStateTimer;

            if (timeInCooldown < slideDuration) {
                // Hitung faktor perlambatan (Dari 1.0 ke 0.0)
                float slowdownFactor = 1.0f - (timeInCooldown / slideDuration);
                
                // Gunakan EaseOut (Kuadratik) biar terasa berat
                slowdownFactor = slowdownFactor * slowdownFactor; 

                // Speed awal pengereman = 40% dari dash speed
                float currentSlideSpeed = mDashSpeed * 0.4f * slowdownFactor;

                // Terapkan gerakan sisa momentum
                position = Vector3Add(position, Vector3Scale(mDashDirection, currentSlideSpeed * dt));
            }

            mStateTimer -= dt;
            if (mStateTimer <= 0) {
                mState = ChargerState::IDLE;
            }
            break;
        }
    }

    // Gravity
    if (position.y > 0) position.y -= 10.0f * dt;
    if (position.y < 0) position.y = 0;
}

void ChargerEnemy::Draw(Model& slimeModel, Model& cubeModel, Model& magnetModel, 
                        Model& shadowPlane, Camera3D cam, Vector3 playerPos) {
    if (!active) return;

    // Shadow
    float shadowScale = radius * 2.2f;
    DrawModelEx(shadowPlane, {position.x, 0.02f, position.z}, 
                {0, 1, 0}, 0.0f, {shadowScale, 1.0f, shadowScale}, 
                ColorAlpha(BLACK, 0.4f));

    // Rotation
    Vector3 dir = (mState == ChargerState::DASHING || mState == ChargerState::COOLDOWN) 
                  ? mDashDirection : Vector3Subtract(playerPos, position);
    float rotationY = atan2f(dir.x, dir.z) * RAD2DEG;

    // Body
    Vector3 scale = {scaleSize * 0.7f, scaleSize * 0.8f, scaleSize * 1.5f};
    Vector3 drawPos = position;
    drawPos.y += scale.y * 0.5f;

    Color currentColor = bodyColor;
    if (mState == ChargerState::CHARGING_UP) {
        if ((int)(mStateTimer * 6) % 2 == 0) currentColor = RED;
    }
    // 🔥 VISUAL COOLDOWN: Warna jadi agak gelap (kecapekan)
    else if (mState == ChargerState::COOLDOWN) {
        currentColor = ColorBrightness(bodyColor, -0.3f); 
    }

    Color finalColor = GetRenderColor(currentColor);

    cubeModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = finalColor;
    DrawModelEx(cubeModel, drawPos, {0, 1, 0}, rotationY, scale, WHITE);
    cubeModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;

    // 🔥 TRAIL LOGIC
    if (mState == ChargerState::DASHING) {
        // Trail Ungu (Energi)
        DrawSphere(position, radius * 0.5f, ColorAlpha(PURPLE, 0.3f));
    } 
    else if (mState == ChargerState::COOLDOWN && mStateTimer > 1.5f) {
        // 🔥 TRAIL ASAP PENGEREMAN (Gray/Smoke)
        // Muncul hanya di 0.5 detik pertama cooldown
        Vector3 dustPos = position;
        dustPos.y = 0.5f;
        // Asap di belakang bawah (ban ngerem)
        dustPos = Vector3Subtract(dustPos, Vector3Scale(mDashDirection, 0.5f));
        
        DrawSphere(dustPos, radius * 0.4f, ColorAlpha(GRAY, 0.4f));
        DrawCubeWires(dustPos, radius * 0.5f, radius * 0.5f, radius * 0.5f, ColorAlpha(DARKGRAY, 0.5f));
    }
}