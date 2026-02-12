#include "Player.h"
#include "rlgl.h"
#include <cmath>
#include <algorithm>
#include "../Systems/ProjectileManager.h"
#include "../Utils/MathUtils.h" 

// Helper buat rotasi sudut
float LerpAngle(float current, float target, float t) {
    float diff = fmodf(target - current + 180, 360) - 180;
    if (diff < -180) diff += 360;
    return current + diff * fminf(t, 1.0f);
}

Player::Player() {
    Reset();
}

void Player::Reset() {
    position = { 0, 0.5f, 0 };
    rotationY = 0.0f;
    walkTimer = 0.0f;
    shootTimer = 0.0f;
    
    dashCooldown = 0.0f;
    dashTime = 0.0f;
    dashDir = {0, 0, 0};

    hp = 100.0f;
    maxHp = 100.0f;
    level = 1;
    currentXP = 0;
    nextLevelXP = 100.0f;
    magnetBuffTimer = 0.0f; 

    SwitchWeapon(WeaponType::PISTOL);
}

void Player::SwitchWeapon(WeaponType type) {
    currentWeapon = type;

    switch (type) {
        case WeaponType::PISTOL:
            stats.damage = 25.0f;
            stats.fireRate = 0.25f;
            stats.bulletSpeed = 40.0f;
            stats.bulletSize = 0.3f;
            stats.projectileCount = 1;
            stats.spreadAngle = 2.0f; 
            stats.moveSpeed = 10.0f;  
            stats.bulletType = ProjectileType::NORMAL;
            break;

        case WeaponType::SHOTGUN:
            stats.damage = 15.0f;     
            stats.fireRate = 0.9f;    
            stats.bulletSpeed = 30.0f;
            stats.bulletSize = 0.25f;
            stats.projectileCount = 5; 
            stats.spreadAngle = 30.0f; 
            stats.moveSpeed = 9.0f;    
            stats.bulletType = ProjectileType::NORMAL;
            break;

        case WeaponType::MINIGUN:
            stats.damage = 8.0f;
            stats.fireRate = 0.06f;   
            stats.bulletSpeed = 45.0f;
            stats.bulletSize = 0.2f;
            stats.projectileCount = 1;
            stats.spreadAngle = 6.0f; 
            stats.moveSpeed = 7.0f;   
            stats.bulletType = ProjectileType::NORMAL;
            break;

        case WeaponType::BAZOOKA:
            stats.damage = 150.0f;    
            stats.fireRate = 1.5f;    
            stats.bulletSpeed = 20.0f; 
            stats.bulletSize = 0.8f;   
            stats.projectileCount = 1;
            stats.spreadAngle = 0.0f; 
            stats.moveSpeed = 8.0f;
            stats.bulletType = ProjectileType::EXPLOSIVE; 
            break;
    }
}

void Player::Update(float dt) {
    if (IsDead()) return;

    if (shootTimer > 0) shootTimer -= dt;
    if (magnetBuffTimer > 0) magnetBuffTimer -= dt;
    if (dashCooldown > 0) dashCooldown -= dt;

    if (IsKeyPressed(KEY_ONE)) SwitchWeapon(WeaponType::PISTOL);
    if (IsKeyPressed(KEY_TWO)) SwitchWeapon(WeaponType::SHOTGUN);
    if (IsKeyPressed(KEY_THREE)) SwitchWeapon(WeaponType::MINIGUN);
    if (IsKeyPressed(KEY_FOUR)) SwitchWeapon(WeaponType::BAZOOKA);

    float scroll = GetMouseWheelMove();
    if (scroll != 0) {
        int current = (int)currentWeapon;
        current += (scroll > 0) ? 1 : -1;
        
        if (current > 3) current = 0;
        if (current < 0) current = 3;
        
        SwitchWeapon((WeaponType)current);
    }

    // DASH OVERRIDE MOVEMENT DENGAN PHASING (Anticipate, Action, Recovery)
    if (dashTime > 0.0f) {
        dashTime -= dt;
        float p = 1.0f - (dashTime / 0.4f); 
        
        if (p >= 0.2f && p <= 0.8f) {
            position.x += dashDir.x * 35.0f * dt;
            position.z += dashDir.z * 35.0f * dt;
        } 
        else if (p > 0.8f) {
            float slide = 1.0f - ((p - 0.8f) / 0.2f); 
            position.x += dashDir.x * 10.0f * slide * dt;
            position.z += dashDir.z * 10.0f * slide * dt;
        }
        return; 
    }

    // Normal Movement Logic
    Vector3 input = { 0, 0, 0 };
    if (IsKeyDown(KEY_W)) input.z -= 1;
    if (IsKeyDown(KEY_S)) input.z += 1;
    if (IsKeyDown(KEY_A)) input.x -= 1;
    if (IsKeyDown(KEY_D)) input.x += 1;

    if (Vector3Length(input) > 0) {
        input = Vector3Normalize(input);
        position = Vector3Add(position, Vector3Scale(input, stats.moveSpeed * dt));

        float targetRotation = (atan2f(input.x, input.z) * RAD2DEG) + 90.0f;
        rotationY = LerpAngle(rotationY, targetRotation, 10.0f * dt);

        walkTimer += dt * 12.0f; 
        if (walkTimer > PI * 2.0f) walkTimer -= PI * 2.0f;

    } else {
        float target = 0.0f;
        if (walkTimer > PI) target = PI * 2.0f;
        walkTimer = Lerp(walkTimer, target, 10.0f * dt);
        if (fabs(target - walkTimer) < 0.01f) walkTimer = 0.0f;
    }
}

void Player::TryShoot(Vector3 targetPos, ProjectileManager& projManager, float dt) {
    if (shootTimer > 0) return;

    Vector3 baseDir = Vector3Subtract(targetPos, position);
    baseDir.y = 0;
    baseDir = Vector3Normalize(baseDir);

    int count = stats.projectileCount;
    float spreadRad = stats.spreadAngle * DEG2RAD;

    for (int i = 0; i < count; i++) {
        Vector3 finalDir = baseDir;

        if (count > 1) {
            float step = spreadRad / (float)(count - 1);
            float angle = -spreadRad / 2.0f + (step * i);
            finalDir = Vector3RotateByAxisAngle(baseDir, (Vector3){0,1,0}, angle);
        }
        else if (stats.spreadAngle > 0) {
            float randomAngle = GetRandomFloat(-spreadRad/2.0f, spreadRad/2.0f);
            finalDir = Vector3RotateByAxisAngle(baseDir, (Vector3){0,1,0}, randomAngle);
        }

        projManager.SpawnProjectile(position, finalDir, stats);
    }

    shootTimer = stats.fireRate;
}

void Player::TryDash(Vector3 mousePos) {
    if (dashCooldown <= 0.0f) {
        Vector3 dir = Vector3Subtract(position, mousePos); 
        dir.y = 0.0f;
        dashDir = Vector3Normalize(dir);
        
        dashTime = 0.4f;    
        dashCooldown = 1.0f; 
    }
}

void Player::LevelUp() {
    level++;
    currentXP -= nextLevelXP;
    nextLevelXP *= 1.3f;
    maxHp += 20.0f; 
    hp = maxHp; 
}

void Player::Draw(Model& sotoModel, Camera3D cam, Texture2D shadow) {
    if (IsDead()) return;

    float hop = fabsf(sinf(walkTimer)) * 0.15f; 
    float tilt = sinf(walkTimer) * 12.0f; 

    Vector3 drawPos = position;
    
    // --- VARIABEL ANIMASI DASH (Squash & Stretch) ---
    float dashScaleY = 1.0f;
    float dashScaleXZ = 1.0f;
    float dashRollAngle = 0.0f;

    if (dashTime <= 0.0f) {
        drawPos.y += hop;
    } else {
        float p = 1.0f - (dashTime / 0.4f);
        
        if (p < 0.2f) { 
            // FASE 1: ANTISIPASI
            float t = p / 0.2f;
            dashScaleY = 1.0f - (0.4f * t);   
            dashScaleXZ = 1.0f + (0.3f * t);  
            dashRollAngle = t * 20.0f;        
        } 
        else if (p < 0.8f) { 
            // FASE 2: ACTION
            float t = (p - 0.2f) / 0.6f;
            drawPos.y += 1.2f * sinf(t * PI); 
            
            dashScaleY = 0.6f + (0.6f * sinf(t * PI)); 
            dashScaleXZ = 1.3f - (0.4f * sinf(t * PI));

            float ease = 1.0f - powf(1.0f - t, 3.0f);
            dashRollAngle = 20.0f - (ease * 380.0f); 
        } 
        else { 
            // FASE 3: RECOVERY
            float t = (p - 0.8f) / 0.2f;
            dashScaleY = 0.6f + (0.4f * t);   
            dashScaleXZ = 1.3f - (0.3f * t);  
            dashRollAngle = 0.0f;             
        }
    }

    // --- DRAW SHADOW ---
    float shadowSize = 1.2f * dashScaleXZ; 
    rlPushMatrix();
        rlTranslatef(position.x, 0.01f, position.z); 
        rlRotatef(90, 1, 0, 0);
        rlDisableDepthMask();
        DrawTexturePro(shadow, 
            (Rectangle){ 0, 0, (float)shadow.width, (float)shadow.height },
            (Rectangle){ -shadowSize/2, -shadowSize/2, shadowSize, shadowSize },
            (Vector2){ 0, 0 }, 0.0f, ColorAlpha(WHITE, 0.5f));
        rlEnableDepthMask();
    rlPopMatrix();

    // --- DRAW MODEL ---
    if (sotoModel.meshCount > 0) {
        float modelScale = 0.012f;
        rlPushMatrix(); 
            rlTranslatef(drawPos.x, drawPos.y, drawPos.z);
            
            if (dashTime > 0.0f) {
                Vector3 rollAxis = {-dashDir.z, 0.0f, dashDir.x}; 
                rlRotatef(dashRollAngle, rollAxis.x, rollAxis.y, rollAxis.z);
            }

            rlRotatef(rotationY, 0, 1, 0); 
            
            if (dashTime <= 0.0f) {
                rlRotatef(tilt, 1, 0, 0); 
            }

            rlScalef(modelScale * dashScaleXZ, modelScale * dashScaleY, modelScale * dashScaleXZ);
            
            DrawModel(sotoModel, {0, 0, 0}, 1.0f, WHITE);
        rlPopMatrix(); 
    }
}

void Player::AddXP(float amount) {
    if (IsDead()) return;
    currentXP += amount;
    if (currentXP >= nextLevelXP) LevelUp();
}

void Player::TakeDamage(float amount) {
    if (dashTime > 0.0f) return; // I-Frames (Kebal saat salto)

    hp -= amount;
    if (hp < 0) hp = 0;
}

void Player::Heal(float amount) {
    hp += amount;
    if (hp > maxHp) hp = maxHp;
}

void Player::ActivateMagnetBuff(float duration) {
    magnetBuffTimer = duration;
}