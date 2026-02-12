#include "BossEnemy.h"
#include "../Utils/MathUtils.h"
#include "raymath.h"
#include <cmath>
#include <algorithm>

BossEnemy::BossEnemy(BossType type, Vector3 startPos, int waveNumber) 
    : BaseEnemy(4, startPos) {
    
    mBossType = type;
    mPhase = BossPhase::PHASE_1;
    position = startPos;
    velocity = {0, 0, 0};
    active = true;
    tier = 4;
    
    mAttackTimer = 0.0f;
    mAttackCycle = 0;
    mIsCharging = false;
    mShouldSpawnMinion = false;
    mSummonTimer = 0.0f;
    mTeleportTimer = 0.0f;
    mIsTeleporting = false;
    glowIntensity = 0.0f;

    // 🔥 BOSS STATS BASED ON TYPE + WAVE SCALING
    float waveScaling = 1.0f + (waveNumber / 20.0f) * 0.5f;

    switch (type) {
        case BossType::TANK_BOSS:
            hp = 3000 * waveScaling;
            speed = 3.0f;
            radius = 4.0f;
            xpReward = 2000;
            bodyColor = DARKGRAY;
            scaleSize = 5.0f;
            mAttackCooldown = 3.0f;
            mChargeSpeed = 15.0f;
            break;
        
        case BossType::SUMMONER_BOSS:
            hp = 4000 * waveScaling;
            speed = 4.0f;
            radius = 3.5f;
            xpReward = 4000;
            bodyColor = PURPLE;
            scaleSize = 4.5f;
            mAttackCooldown = 5.0f;
            break;
        
        case BossType::ARTILLERY_BOSS:
            hp = 5000 * waveScaling;
            speed = 2.5f;
            radius = 3.0f;
            xpReward = 6000;
            bodyColor = ORANGE;
            scaleSize = 4.0f;
            mAttackCooldown = 1.5f;
            break;
        
        case BossType::TELEPORTER_BOSS:
            hp = 6000 * waveScaling;
            speed = 5.0f;
            radius = 3.5f;
            xpReward = 8000;
            bodyColor = SKYBLUE;
            scaleSize = 4.5f;
            mAttackCooldown = 2.0f;
            mChargeSpeed = 30.0f;
            break;
        
        case BossType::ULTIMATE_BOSS:
            hp = 10000 * waveScaling;
            speed = 6.0f;
            radius = 5.0f;
            xpReward = 20000;
            bodyColor = GOLD;
            scaleSize = 6.0f;
            mAttackCooldown = 1.0f;
            mChargeSpeed = 40.0f;
            break;
    }

    maxHp = hp;
}

void BossEnemy::Update(float dt, Vector3 playerPos) {
    if (!active) return;

    UpdateFlash(dt);

    UpdatePhase();

    // --- 🔥 MOVEMENT LOGIC (Moved here so it runs every frame) ---
    float currentSpeed = speed;

    // Handle Speed Modifiers
    if (mIsCharging) {
        if (mBossType == BossType::TANK_BOSS || 
            mBossType == BossType::TELEPORTER_BOSS || 
            mBossType == BossType::ULTIMATE_BOSS) {
            currentSpeed = mChargeSpeed;
        }
    }

    // Move towards player
    Vector3 dir = Vector3Normalize(Vector3Subtract(playerPos, position));
    position = Vector3Add(position, Vector3Scale(dir, currentSpeed * dt));
    // -------------------------------------------------------------

    ExecuteAttackPattern(dt, playerPos);

    // Update projectiles
    for (auto& p : mProjectiles) {
        if (!p.active) continue;
        
        p.position = Vector3Add(p.position, Vector3Scale(p.direction, p.speed * dt));
        p.lifeTime -= dt;
        
        if (p.lifeTime <= 0 || p.position.y <= 0) {
            p.active = false;
        }
    }

    mProjectiles.erase(
        std::remove_if(mProjectiles.begin(), mProjectiles.end(),
            [](const BossProjectile& p) { return !p.active; }),
        mProjectiles.end()
    );

    // Gravity
    if (position.y > 0) position.y -= 10.0f * dt;
    if (position.y < 0) position.y = 0;

    // Glow pulse
    glowIntensity = 0.5f + sinf(GetTime() * 3.0f) * 0.5f;
}

void BossEnemy::UpdatePhase() {
    float hpPercent = hp / maxHp;
    
    if (hpPercent > 0.66f) {
        mPhase = BossPhase::PHASE_1;
    } else if (hpPercent > 0.33f) {
        if (mPhase == BossPhase::PHASE_1) {
            speed *= 1.3f;
            mAttackCooldown *= 0.8f;
        }
        mPhase = BossPhase::PHASE_2;
    } else {
        if (mPhase == BossPhase::PHASE_2) {
            speed *= 1.5f;
            mAttackCooldown *= 0.6f;
        }
        mPhase = BossPhase::PHASE_3;
    }
}

void BossEnemy::ExecuteAttackPattern(float dt, Vector3 playerPos) {
    mAttackTimer -= dt;
    
    if (mAttackTimer > 0) return;

    // Reset timer
    mAttackTimer = mAttackCooldown;
    mAttackCycle++;

    switch (mBossType) {
        case BossType::TANK_BOSS:
            AttackPattern_Melee(dt, playerPos);
            break;
        case BossType::SUMMONER_BOSS:
            AttackPattern_Summon(dt);
            break;
        case BossType::ARTILLERY_BOSS:
            AttackPattern_Barrage(dt, playerPos);
            break;
        case BossType::TELEPORTER_BOSS:
            AttackPattern_Teleport(dt, playerPos);
            break;
        case BossType::ULTIMATE_BOSS:
            AttackPattern_Ultimate(dt, playerPos);
            break;
    }
}

// === ATTACK PATTERNS (State Setters) ===

void BossEnemy::AttackPattern_Melee(float dt, Vector3 playerPos) {
    // Cycle 3 = Charge Mode
    if (mAttackCycle % 3 == 0) {
        mIsCharging = true;
    } else {
        mIsCharging = false;
    }
}

void BossEnemy::AttackPattern_Summon(float dt) {
    mShouldSpawnMinion = true;
    
    // Teleport kiting logic
    if (mAttackCycle % 2 == 0) {
        float angle = GetRandomFloat(0, 360) * DEG2RAD;
        float dist = GetRandomFloat(10, 15);
        position.x += cosf(angle) * dist;
        position.z += sinf(angle) * dist;
    }
}

void BossEnemy::AttackPattern_Barrage(float dt, Vector3 playerPos) {
    int bulletCount = (mPhase == BossPhase::PHASE_3) ? 8 : 5;
    float spreadAngle = 360.0f / bulletCount;
    
    for (int i = 0; i < bulletCount; i++) {
        BossProjectile p;
        p.position = position;
        p.position.y += radius;
        
        float angle = (spreadAngle * i) * DEG2RAD;
        p.direction = {cosf(angle), 0, sinf(angle)};
        
        p.speed = 12.0f;
        p.damage = 20.0f;
        p.radius = 0.4f;
        p.active = true;
        p.lifeTime = 8.0f;
        
        mProjectiles.push_back(p);
    }
}

void BossEnemy::AttackPattern_Teleport(float dt, Vector3 playerPos) {
    if (mAttackCycle % 2 == 0) {
        // Teleport BEHIND player
        Vector3 toPlayer = Vector3Subtract(position, playerPos);
        toPlayer = Vector3Normalize(toPlayer);
        position = Vector3Add(playerPos, Vector3Scale(toPlayer, 10.0f));
        
        mIsTeleporting = true; // Visual effect
        mIsCharging = false;   // Reset speed
    } else {
        // Activate CHARGE MODE (Movement handled in Update)
        mIsCharging = true;
        mIsTeleporting = false;
    }
}

void BossEnemy::AttackPattern_Ultimate(float dt, Vector3 playerPos) {
    int pattern = mAttackCycle % 4;
    
    switch (pattern) {
        case 0: AttackPattern_Barrage(dt, playerPos); break;
        case 1: AttackPattern_Melee(dt, playerPos); break;
        case 2: AttackPattern_Summon(dt); break;
        case 3: AttackPattern_Teleport(dt, playerPos); break;
    }
}

void BossEnemy::Draw(Model& slimeModel, Model& cubeModel, Model& magnetModel, 
                     Model& shadowPlane, Camera3D cam, Vector3 playerPos) {
    if (!active) return;

    // Shadow
    float shadowScale = radius * 3.0f;
    DrawModelEx(shadowPlane, {position.x, 0.05f, position.z}, 
                {0, 1, 0}, 0.0f, {shadowScale, 1.0f, shadowScale}, 
                ColorAlpha(BLACK, 0.6f));

    // Rotation
    Vector3 dir = Vector3Subtract(playerPos, position);
    float rotationY = atan2f(dir.x, dir.z) * RAD2DEG;

    // Body
    Vector3 drawPos = position;
    drawPos.y += scaleSize * 0.5f;
    Vector3 scale = {scaleSize, scaleSize, scaleSize};

    // Phase Colors
    Color currentColor = bodyColor;
    if (mPhase == BossPhase::PHASE_3) currentColor = RED;
    else if (mPhase == BossPhase::PHASE_2) currentColor = ColorBrightness(bodyColor, 0.3f);

    // 🔥 FIX: HIT EFFECT LOGIC
    Color finalColor = GetRenderColor(currentColor);

    // Apply warna (Normal atau Putih)
    cubeModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = finalColor;
    DrawModelEx(cubeModel, drawPos, {0, 1, 0}, rotationY, scale, WHITE);
    
    // ⚠️ WAJIB RESET: Kembalikan warna ke default agar musuh lain tidak ikut berubah
    cubeModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;

    // Glow Aura (Gunakan finalColor agar aura juga ikut kedip/berubah)
    DrawSphere(drawPos, scaleSize * 1.2f, ColorAlpha(currentColor, glowIntensity * 0.3f));

    // HP Bar & Projectiles (Tetap sama)
    Vector3 hpBarPos = drawPos;
    hpBarPos.y += scaleSize + 2.0f;
    float hpPercent = hp / maxHp;
    float barWidth = 4.0f;
    float barHeight = 0.3f;
    
    DrawCube(hpBarPos, barWidth, barHeight, 0.1f, DARKGRAY);
    Vector3 fillPos = hpBarPos;
    fillPos.x -= barWidth * 0.5f * (1.0f - hpPercent);
    DrawCube(fillPos, barWidth * hpPercent, barHeight, 0.1f, RED);

    for (const auto& p : mProjectiles) {
        if (!p.active) continue;
        DrawSphere(p.position, p.radius, ORANGE);
        DrawSphereWires(p.position, p.radius, 4, 4, RED);
    }

    if (mIsTeleporting) {
        DrawSphere(drawPos, scaleSize * 1.5f, ColorAlpha(SKYBLUE, 0.5f));
    }
}