#pragma once
#include "raylib.h"
#include <vector>

// 🔥 TIPE ITEM (Tambah HP & Weapon)
enum class ItemType {
    NONE,
    MAGNET,      // Magnet buff (udah ada)
    HEALTH_PACK, // +50 HP
    WEAPON_DROP  // Random weapon upgrade
};

struct DroppedItem {
    Vector3 position;
    ItemType type;
    float lifeTime;
    bool active;
    float bobTimer; // Animasi naik-turun
    
    // ✅ KHUSUS WEAPON DROP
    int weaponTier; // 0=Pistol, 1=Shotgun, 2=Minigun, 3=Bazooka
};

class ItemManager {
public:
    ItemManager();

    void Update(float dt);
    void Draw(Model& magnetModel);
    
    // Spawn item
    void SpawnItem(Vector3 pos, ItemType type, int weaponTier = 0);
    
    // Check pickup (return tipe + weapon tier via reference)
    ItemType CheckPickup(Vector3 playerPos, float pickupRadius, int& outWeaponTier);
    
    void Reset();

    const std::vector<DroppedItem>& GetItems() const { return mItems; }

private:
    std::vector<DroppedItem> mItems;
};