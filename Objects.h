#pragma once
#include "raylib.h"
#include "raymath.h"

// Struct Peluru
struct Bullet {
    Vector3 position;
    Vector3 velocity;
    float radius;
    bool active;
};

// Struct XP (Permata Hijau)
struct XPGem {
    Vector3 position;
    int value;
    bool active;
};

// NOTE: 
// - struct Particle sudah pindah ke "Managers/ParticleSystem.h"
// - struct Enemy sudah pindah ke "Enemies/BaseEnemy.h"
// HAPUS BIAR GAK ERROR REDEFINITION