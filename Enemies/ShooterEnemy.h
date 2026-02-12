#pragma once
#include "BaseEnemy.h"
#include "raylib.h"
#include <vector>

// Peluru musuh (beda dari player)
struct EnemyBullet {
    Vector3 position;
    Vector3 direction;
    float speed;
    float damage;
    float radius;
    bool active;
    float lifeTime;
};

class ShooterEnemy : public BaseEnemy {
public:
    ShooterEnemy(int tier, Vector3 startPos);
    
    void Update(float dt, Vector3 playerPos) override;
    void Draw(Model& slimeModel, Model& cubeModel, Model& magnetModel, 
              Model& shadowPlane, Camera3D cam, Vector3 playerPos) override;

    void Shoot(Vector3 targetPos);
    std::vector<EnemyBullet>& GetBullets() { return mBullets; }

private:
    // Variable yang dibutuhkan di .cpp
    float mShootTimer;
    float mShootCooldown;
    float mShootRange;
    std::vector<EnemyBullet> mBullets;
    
    Color bodyColor;
    float scaleSize;
};