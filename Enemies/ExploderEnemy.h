#pragma once
#include "BaseEnemy.h"
#include "raylib.h"

class ExploderEnemy : public BaseEnemy {
public:
    ExploderEnemy(int tier, Vector3 startPos);
    
    void Update(float dt, Vector3 playerPos) override;
    void Draw(Model& slimeModel, Model& cubeModel, Model& magnetModel, 
              Model& shadowPlane, Camera3D cam, Vector3 playerPos) override;

    // ✅ EXPLOSION CHECK
    bool ShouldExplode(Vector3 playerPos);
    float GetExplosionRadius() const { return mExplosionRadius; }
    float GetExplosionDamage() const { return mExplosionDamage; }

private:
    float mFuseTimer;
    float mExplosionRadius;
    float mExplosionDamage;
    bool mIsArmed;
    Color bodyColor;
    float scaleSize;
};