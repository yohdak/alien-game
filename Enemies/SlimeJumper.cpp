#include "SlimeJumper.h"
#include "raymath.h"
#include <cmath>
#include "rlgl.h"

SlimeJumper::SlimeJumper(int tierInput, Vector3 startPos) : BaseEnemy(tierInput, startPos) {
    tier = tierInput;
    position = startPos;
    velocity = {0, 0, 0};
    active = true;

    if (tier == 1) { hp = 30; speed = 6.0f; radius = 1.0f; xpReward = 25; }
    else if (tier == 2) { hp = 100; speed = 4.5f; radius = 1.5f; xpReward = 80; }
    else { hp = 500; speed = 3.0f; radius = 3.0f; xpReward = 500; }

    jumpTimer = GetRandomValue(0, 100) / 100.0f;
    isJumping = false;
    verticalSpeed = 0;

    // ✅ RANDOM VARIANT SYSTEM
    // 70% = Basic (Box), 30% = Special (Magnet/Loot)
    int roll = GetRandomValue(1, 100);
    if (roll <= 70) {
        variant = SlimeVariant::BASIC;
        hasLoot = false;
    } else {
        variant = SlimeVariant::MAGNET;
        hasLoot = true;
    }

     // ✅ DROP SYSTEM (RNG)
    
    if (roll <= 50) {
        // 50% = Gak drop apa-apa
        variant = SlimeVariant::BASIC;
        hasLoot = false;
        lootType = ItemType::NONE;
    } 
    else if (roll <= 70) {
        // 20% = Drop Magnet
        variant = SlimeVariant::MAGNET;
        hasLoot = true;
        lootType = ItemType::MAGNET;
    }
    else if (roll <= 85) {
        // 15% = Drop HP Pack
        variant = SlimeVariant::HEALTH;
        hasLoot = true;
        lootType = ItemType::HEALTH_PACK;
    }
    else {
        // 15% = Drop Weapon (Random tier)
        variant = SlimeVariant::WEAPON;
        hasLoot = true;
        lootType = ItemType::WEAPON_DROP;
        weaponDropTier = GetRandomValue(0, 3); // 0-3 (Pistol-Bazooka)
    }
}

// ✅ Getter baru
ItemType SlimeJumper::GetLootType() const {
    return lootType;
}

int SlimeJumper::GetWeaponDropTier() const {
    return weaponDropTier;

}

void SlimeJumper::Update(float dt, Vector3 playerPos) {

    UpdateFlash(dt);

    if (!isJumping) {
        jumpTimer += dt;
        if (jumpTimer > 1.5f) {
            isJumping = true;
            verticalSpeed = 10.0f;
            
            Vector3 dir = Vector3Subtract(playerPos, position);
            dir.y = 0;
            jumpDir = Vector3Normalize(dir);
            jumpTimer = 0;
        }
    } else {
        // Fisika Lompat
        position.y += verticalSpeed * dt;
        verticalSpeed -= 30.0f * dt;
        
        position.x += jumpDir.x * speed * 2.0f * dt;
        position.z += jumpDir.z * speed * 2.0f * dt;

        if (position.y <= 0) {
            position.y = 0;
            isJumping = false;
        }
    }
}

void SlimeJumper::Draw(Model& slimeModel, Model& cubeModel, Model& magnetModel, Model& shadowPlane, Camera3D cam, Vector3 playerPos) 
{
    // --- 1. SETUP WARNA DASAR (TIER & VARIANT) ---
    Color baseColor = (Color){ 0, 180, 255, 255 }; // Cyan (Tier 1)
    
    if (tier == 2) baseColor = (Color){ 0, 100, 255, 255 }; // Biru Laut (Tier 2)
    if (tier > 2) baseColor = (Color){ 180, 0, 255, 255 };  // Ungu (Tier 3)

    // Variant Magnet agak beda warnanya
    if (variant == SlimeVariant::MAGNET) {
        baseColor.r = (unsigned char)fminf(baseColor.r + 30, 255);
        baseColor.g = (unsigned char)fminf(baseColor.g + 30, 255);
    }

    // --- 2. LOGIKA FLASH "HAMPIR PUTIH" (OUTER SHELL) ---
    // Kita manipulasi Vector4 untuk dikirim ke Shader
    Vector4 shaderColorVec = ColorNormalize(baseColor); 

    if (flashTimer > 0) {
        float flashStrength = 0.8f; // 80% Putih, 20% Warna Asli
        
        // Lerp (Linear Interpolation) ke arah Putih (1.0)
        shaderColorVec.x = shaderColorVec.x + (1.0f - shaderColorVec.x) * flashStrength; // R
        shaderColorVec.y = shaderColorVec.y + (1.0f - shaderColorVec.y) * flashStrength; // G
        shaderColorVec.z = shaderColorVec.z + (1.0f - shaderColorVec.z) * flashStrength; // B
        // Alpha (.w) biarkan tetap
    }

    // --- 3. ANIMASI FISIK (SQUASH & STRETCH) ---
    float stretch = 1.0f + (position.y * 0.4f); 
    float squash = 1.0f / sqrtf(stretch);
    Vector3 scale = { radius * squash, radius * stretch, radius * squash };

    Vector3 centerPos = position;
    centerPos.y += radius * stretch * 0.5f;

    // --- 4. RENDER SHADOW ---
    rlDisableDepthMask();
    float shadowScale = (radius * 2.5f) * (1.0f / (1.0f + position.y * 0.5f)); 
    DrawModelEx(shadowPlane, 
                (Vector3){position.x, 0.02f, position.z}, 
                (Vector3){0, 1, 0}, 0.0f, 
                (Vector3){shadowScale, 1.0f, shadowScale}, 
                (Color){0, 0, 0, 120});
    rlEnableDepthMask();

    // --- 5. RENDER INNER OBJECT (BOX / MAGNET) ---
    rlPushMatrix();
        rlTranslatef(centerPos.x, centerPos.y, centerPos.z);
        
        float time = GetTime();
        rlRotatef(time * 50.0f, 0, 1, 0);
        rlRotatef(sinf(time * 2.0f) * 15.0f, 1, 0, 0); 
        rlRotatef(cosf(time * 1.5f) * 10.0f, 0, 0, 1);

        float baseScale = 0.004f; 
        float itemScale = baseScale * radius;
        Vector3 modelScale = { itemScale, itemScale, itemScale };

        // Tentukan Warna Inner Object
        Color innerTint;
        
        if (variant == SlimeVariant::BASIC) {
            innerTint = (Color){ 200, 180, 160, 255 }; // Coklat Pudar
        } else {
            innerTint = (Color){ 220, 200, 180, 255 }; // Krem Magnet
            if (tier == 2) innerTint = (Color){ 180, 220, 255, 255 };
        }

        // 🔥 LOGIKA FLASH "HAMPIR PUTIH" (INNER OBJECT)
        if (flashTimer > 0) {
            float mixFactor = 0.8f; // 80% Putih
            innerTint.r = (unsigned char)(255 * mixFactor + innerTint.r * (1.0f - mixFactor));
            innerTint.g = (unsigned char)(255 * mixFactor + innerTint.g * (1.0f - mixFactor));
            innerTint.b = (unsigned char)(255 * mixFactor + innerTint.b * (1.0f - mixFactor));
        }

        // Render Model Dalam
        if (variant == SlimeVariant::BASIC) {
            rlRotatef(-90.0f, 0, 0, 1);
            DrawModelEx(cubeModel, Vector3Zero(), (Vector3){0,1,0}, 0.0f, 
                        (Vector3){itemScale * 200, itemScale * 200, itemScale * 200}, innerTint);
        } 
        else if (variant == SlimeVariant::MAGNET) {
            rlRotatef(-90.0f, 0, 0, 1);
            float pulse = 1.0f + sinf(time * 5.0f) * 0.1f;
            DrawModelEx(magnetModel, Vector3Zero(), (Vector3){0,1,0}, 0.0f, 
                        Vector3Scale(modelScale, pulse), innerTint);
        }
    rlPopMatrix();

    // --- 6. RENDER OUTER SHELL (SLIME SKIN) ---
    rlDisableDepthMask(); 
        
        // Kirim warna yang sudah di-flash (shaderColorVec) ke Shader
        SetShaderValue(slimeModel.materials[0].shader, 
                       GetShaderLocation(slimeModel.materials[0].shader, "colDiffuse"), 
                       &shaderColorVec, SHADER_UNIFORM_VEC4);
        
        DrawModelEx(slimeModel, centerPos, (Vector3){0,1,0}, 0.0f, scale, WHITE);
        
    rlEnableDepthMask(); 
}

// ✅ GETTER UNTUK LOOT CHECK
bool SlimeJumper::HasLoot() const {
    return hasLoot;
}

SlimeVariant SlimeJumper::GetVariant() const {
    return variant;
}