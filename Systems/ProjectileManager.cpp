#include "ProjectileManager.h"
#include "../Player/Player.h" // Butuh ini buat tau PlayerStats & ProjectileType
#include "../Managers/AssetManager.h"
#include "../Managers/ParticleSystem.h"
#include "../Utils/MathUtils.h" 
#include <algorithm>


ProjectileManager::ProjectileManager() {
}

void ProjectileManager::SpawnProjectile(Vector3 pos, Vector3 dir, const PlayerStats& stats) {
    Projectile p;
    
    // 1. Posisi & Arah
    p.position = pos;
    p.position.y = 0.5f; // Muncul agak tinggi dikit dari lantai (pinggang player)
    p.direction = dir;
    
    // 2. Stats dari Player (Senjata)
    p.speed = stats.bulletSpeed;      // Ini yang nentuin seberapa jauh dia terbang
    p.damage = stats.damage;
    p.radius = stats.bulletSize;
    
    // 🔥 3. PARABOLIC PHYSICS (GRAVITY)
    // Ini settingan yang lu suka. 
    // verticalVelocity = 'Pop' awal ke atas
    // gravity = Tarikan ke bawah
    p.verticalVelocity = 4.0f; 
    p.gravity = 25.0f; 
    
    // 4. Logic Jarak & Tipe
    // Max distance safety net (biar gak terbang selamanya)
    p.maxDistance = stats.bulletSpeed * 2.0f; 
    p.traveledDistance = 0.0f;
    
    // PENTING: Simpan tipe peluru (Normal / Explosive)
    p.type = stats.bulletType; 
    
    p.lifeTime = 5.0f; // Backup timer
    p.active = true;

    mProjectiles.push_back(p);
}

void ProjectileManager::Update(float dt, AssetManager& assets, ParticleSystem& particles) {
    for (auto& p : mProjectiles) {
        if (!p.active) continue;

        // --- 1. GERAK HORIZONTAL ---
        Vector3 horizontalMove = Vector3Scale(p.direction, p.speed * dt);
        p.position = Vector3Add(p.position, horizontalMove);
        p.traveledDistance += Vector3Length(horizontalMove);

        // --- 2. GERAK VERTIKAL (GRAVITASI/PARABOLIC) ---
        p.position.y += p.verticalVelocity * dt;
        p.verticalVelocity -= p.gravity * dt; 

        // --- 3. LOGIC KENA TANAH ---
        if (p.position.y <= 0.0f) {
            p.position.y = 0.0f;
            p.active = false; // Peluru mati kena tanah

            // A. Kalau Bazooka (Explosive), ledakannya GEDE
            if (p.type == ProjectileType::EXPLOSIVE) {
                particles.SpawnExplosion(p.position, ORANGE, 50); // Partikel banyak
                
                // Sound Ledakan (Kalau ada asetnya)
                // Sound& boom = assets.GetSound("explosion");
                // PlaySound(boom);
                
                // Note: Logic damage area musuh nanti di Game.cpp
            } 
            // B. Kalau Peluru Biasa (Telor Pecah)
            else {
                particles.SpawnExplosion(p.position, YELLOW, 5);
                
                Sound& sfx = assets.GetSound("crack");
                SetSoundPitch(sfx, GetRandomFloat(1.8f, 2.2f)); 
                PlaySound(sfx);
            }
        }

        // --- 4. LIMIT JARAK/WAKTU ---
        if (p.traveledDistance >= p.maxDistance) p.active = false;
        
        p.lifeTime -= dt;
        if (p.lifeTime <= 0) p.active = false;
    }

    // Cleanup peluru mati
    auto iterator = std::remove_if(mProjectiles.begin(), mProjectiles.end(), 
        [](const Projectile& p) { return !p.active; });
    mProjectiles.erase(iterator, mProjectiles.end());
}

void ProjectileManager::Draw() {
    for (const auto& p : mProjectiles) {
        if (!p.active) continue;
        
        Color pColor;
        
        // Bedain warna peluru biar enak liatnya
        if (p.type == ProjectileType::EXPLOSIVE) {
            pColor = DARKGRAY; // Bazooka item/gelap
        } else {
            pColor = { 255, 230, 180, 255 }; // Telur normal
        }

        DrawSphere(p.position, p.radius, pColor);
    }
}

void ProjectileManager::Reset() {
    mProjectiles.clear();
}