#pragma once
#include "raylib.h"
#include "raymath.h"
#include <vector>

// Struct Particle kita pindah kesini
struct Particle {
    Vector3 position;
    Vector3 velocity;
    Color color;
    float size;
    float life;
    float maxLife;
    bool active;
};

class ParticleSystem {
public:
    ParticleSystem();
    
    // Fungsi Utama
    void Update(float dt);
    void Draw();
    void SpawnExplosion(Vector3 center, Color color, int count);
    void Reset(); // Buat bersihin partikel pas Game Over/Reset

private:
    std::vector<Particle> mParticles;
    
    // Helper khusus buat partikel
    float GetRandomFloat(float min, float max);
};