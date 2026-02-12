#include "ParticleSystem.h"
#include <cmath>
#include <cstdlib>
#include <algorithm> // Buat std::remove_if

ParticleSystem::ParticleSystem() {
    mParticles.reserve(1000); // Optimasi memori
}

void ParticleSystem::Update(float dt) {
    float gravity = 35.0f; 

    for (auto& p : mParticles) {
        if (!p.active) continue;
        
        // Fisika
        p.velocity.y -= gravity * dt;
        p.position = Vector3Add(p.position, Vector3Scale(p.velocity, dt));
        
        // Bounce di lantai
        if (p.position.y < 0) { 
            p.position.y = 0; 
            p.velocity.y *= -0.6f; 
        }
        
        // Umur
        p.life -= dt;
        if (p.life <= 0) {
            p.active = false;
        } else {
             // Fade out alpha
             float alpha = p.life / p.maxLife;
             p.color.a = (unsigned char)(alpha * 255);
        }
    }

    // Cleanup partikel mati (Lambda)
    mParticles.erase(std::remove_if(mParticles.begin(), mParticles.end(), 
        [](const Particle& p){ return !p.active; }), 
        mParticles.end());
}

void ParticleSystem::Draw() {
    // Loop gambar simpel
    for(const auto& p : mParticles) {
        if(!p.active) continue;
        DrawCube(p.position, p.size, p.size, p.size, p.color);
    }
}

void ParticleSystem::SpawnExplosion(Vector3 center, Color color, int count) {
    for(int i=0; i<count; i++) {
        Particle p;
        p.position = center;
        
        // Logic acak-acak arah (sama kayak yang lama)
        float theta = GetRandomFloat(0, 360) * DEG2RAD;
        float phi = GetRandomFloat(0, 180) * DEG2RAD;
        float speed = GetRandomFloat(5.0f, 15.0f);
        
        p.velocity.x = sinf(phi) * cosf(theta) * speed;
        p.velocity.y = cosf(phi) * speed;
        p.velocity.z = sinf(phi) * sinf(theta) * speed;
        
        p.size = GetRandomFloat(0.2f, 0.6f);
        p.color = color;
        p.life = GetRandomFloat(0.4f, 0.8f);
        p.maxLife = p.life;
        p.active = true;
        
        mParticles.push_back(p);
    }
}

void ParticleSystem::Reset() {
    mParticles.clear();
}

float ParticleSystem::GetRandomFloat(float min, float max) {
    return min + ((float)GetRandomValue(0, 10000) / 10000.0f) * (max - min);
}