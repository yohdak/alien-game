#pragma once
#include "BaseEnemy.h"
#include "raymath.h"
#include "../Systems/ItemManager.h" // ✅ Include ItemManager

enum class SlimeVariant {
    BASIC,   // Gak drop apa-apa
    MAGNET,  // Drop magnet
    HEALTH,  // Drop HP
    WEAPON   // Drop weapon crate
};

class SlimeJumper : public BaseEnemy {
public:
    SlimeJumper(int tier, Vector3 startPos);
    
    void Update(float dt, Vector3 playerPos) override;
    void Draw(Model& slimeModel, Model& cubeModel, Model& magnetModel, Model& shadowPlane, 
              Camera3D cam, Vector3 playerPos) override;

    // ✅ GETTERS
    bool HasLoot() const;
    SlimeVariant GetVariant() const;
    ItemType GetLootType() const;        // ✅ BARU
    int GetWeaponDropTier() const;       // ✅ BARU

private:
    float jumpTimer;
    bool isJumping;
    float verticalSpeed;
    Vector3 jumpDir;
    
    SlimeVariant variant;
    bool hasLoot;
    ItemType lootType;        // ✅ BARU
    int weaponDropTier;       // ✅ BARU (0-3)
};