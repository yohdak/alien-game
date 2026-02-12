#include "ItemManager.h"
#include "../Utils/MathUtils.h"
#include "raymath.h"
#include <algorithm>
#include "rlgl.h"

ItemManager::ItemManager() {}

void ItemManager::SpawnItem(Vector3 pos, ItemType type, int weaponTier) {
    DroppedItem item;
    item.position = pos;
    item.position.y = 0.5f; // Spawn agak tinggi
    item.type = type;
    item.lifeTime = 15.0f; // 15 detik sebelum hilang
    item.active = true;
    item.bobTimer = GetRandomFloat(0, 6.28f);
    item.weaponTier = weaponTier;
    
    mItems.push_back(item);
}

void ItemManager::Update(float dt) {
    for (auto& item : mItems) {
        if (!item.active) continue;
        
        // Countdown lifetime
        item.lifeTime -= dt;
        if (item.lifeTime <= 0) {
            item.active = false;
            continue;
        }
        
        // Animasi bob (naik-turun)
        item.bobTimer += dt * 3.0f;
        item.position.y = 0.5f + sinf(item.bobTimer) * 0.2f;
    }
    
    // Cleanup
    mItems.erase(
        std::remove_if(mItems.begin(), mItems.end(), 
            [](const DroppedItem& i) { return !i.active; }
        ),
        mItems.end()
    );
}

void ItemManager::Draw(Model& magnetModel) {
    for (const auto& item : mItems) {
        if (!item.active) continue;
        
        float time = GetTime();
        Color itemColor;
        
        // --- RENDER BERDASARKAN TIPE ---
        rlPushMatrix();
            rlTranslatef(item.position.x, item.position.y, item.position.z);
            rlRotatef(time * 100.0f, 0, 1, 0); // Putar
            
            switch (item.type) {
                case ItemType::MAGNET: {
                    // Model Magnet (Pake model asli)
                    itemColor = BLUE;
                    DrawModelEx(magnetModel, Vector3Zero(), {0,1,0}, 0, {0.01f, 0.01f, 0.01f}, itemColor);
                    break;
                }
                
                case ItemType::HEALTH_PACK: {
                    // Kubus Merah (Health Pack)
                    itemColor = RED;
                    DrawCube(Vector3Zero(), 0.4f, 0.4f, 0.4f, itemColor);
                    DrawCubeWires(Vector3Zero(), 0.4f, 0.4f, 0.4f, WHITE);
                    
                    // Tanda + (Cross)
                    DrawCube({0, 0, 0}, 0.1f, 0.5f, 0.1f, WHITE);
                    DrawCube({0, 0, 0}, 0.5f, 0.1f, 0.1f, WHITE);
                    break;
                }
                
                case ItemType::WEAPON_DROP: {
                    // Limas/Pyramid (Weapon Crate)
                    // Warna berdasarkan tier weapon
                    if (item.weaponTier == 0) itemColor = GRAY;       // Pistol
                    else if (item.weaponTier == 1) itemColor = ORANGE; // Shotgun
                    else if (item.weaponTier == 2) itemColor = YELLOW; // Minigun
                    else itemColor = PURPLE;                           // Bazooka
                    
                    // Gambar Pyramid (Manual vertices karena raylib gak punya DrawPyramid)
                    float size = 0.5f;
                    Vector3 apex = {0, size, 0};      // Puncak
                    Vector3 base1 = {-size/2, 0, -size/2};
                    Vector3 base2 = {size/2, 0, -size/2};
                    Vector3 base3 = {size/2, 0, size/2};
                    Vector3 base4 = {-size/2, 0, size/2};
                    
                    // Draw 4 triangles + base
                    DrawTriangle3D(apex, base2, base1, itemColor);
                    DrawTriangle3D(apex, base3, base2, itemColor);
                    DrawTriangle3D(apex, base4, base3, itemColor);
                    DrawTriangle3D(apex, base1, base4, itemColor);
                    DrawTriangle3D(base1, base2, base3, ColorAlpha(itemColor, 0.5f));
                    DrawTriangle3D(base1, base3, base4, ColorAlpha(itemColor, 0.5f));
                    
                    // Wireframe
                    DrawLine3D(apex, base1, WHITE);
                    DrawLine3D(apex, base2, WHITE);
                    DrawLine3D(apex, base3, WHITE);
                    DrawLine3D(apex, base4, WHITE);
                    break;
                }
                
                default:
                    break;
            }
        rlPopMatrix();
        
        // Particle glow effect
        if (item.lifeTime < 3.0f) {
            // Blink warning (Item mau hilang)
            if ((int)(item.lifeTime * 4) % 2 == 0) {
                DrawSphere(item.position, 0.3f, ColorAlpha(itemColor, 0.3f));
            }
        }
    }
}

ItemType ItemManager::CheckPickup(Vector3 playerPos, float pickupRadius, int& outWeaponTier) {
    for (auto& item : mItems) {
        if (!item.active) continue;
        
        float dist = Vector3Distance(playerPos, item.position);
        if (dist < pickupRadius) {
            item.active = false;
            outWeaponTier = item.weaponTier; // Output weapon tier
            return item.type;
        }
    }
    
    outWeaponTier = -1;
    return ItemType::NONE;
}

void ItemManager::Reset() {
    mItems.clear();
}