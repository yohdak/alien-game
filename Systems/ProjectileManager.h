#pragma once
#include "raylib.h"
#include "raymath.h"
#include <vector>

// Forward declarations
// Kita cuma butuh nama kelasnya biar gak error, gak perlu include file-nya
class AssetManager;
class ParticleSystem;
struct PlayerStats; 

// 🔥 DEFINISI TIPE PELURU
// (Penting: Pastikan di Player.h enum ini dihapus atau disesuaikan biar gak bentrok)
enum class ProjectileType {
    NORMAL,     // Telur biasa (Pistol, Shotgun, Minigun)
    EXPLOSIVE   // Bazooka (Meledak Area)
};

struct Projectile {
    Vector3 position;
    Vector3 direction;
    float speed;
    float damage;
    float radius;
    bool active;

    // 🟢 FISIKA PARABOLIC (Lengkungan)
    float verticalVelocity; // Kecepatan lempar ke atas
    float gravity;          // Tarikan ke bawah

    // 🟢 LIMITS
    float maxDistance;
    float traveledDistance;
    float lifeTime;

    // 🟢 TIPE (Penting buat Bazooka)
    ProjectileType type;
};

class ProjectileManager {
public:
    ProjectileManager();

    // Spawn butuh data stats dari player
    void SpawnProjectile(Vector3 pos, Vector3 dir, const PlayerStats& stats);

    // Update cuma butuh Assets (Suara) & Particles (Debu Tanah)
    // Musuh dihapus dari sini karena logic tabrakan pindah ke Game.cpp
    void Update(float dt, AssetManager& assets, ParticleSystem& particles);

    void Draw();
    void Reset();

    // Getter buat dipake di Game.cpp (Collision detection)
    std::vector<Projectile>& GetProjectiles() { return mProjectiles; }

private:
    std::vector<Projectile> mProjectiles;
};