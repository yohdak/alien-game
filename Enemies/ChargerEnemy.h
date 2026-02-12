#pragma once
#include "BaseEnemy.h"
#include "raylib.h"

enum class ChargerState {
    IDLE,
    CHARGING_UP,  // Persiapan dash (telegraph)
    DASHING,      // Lagi dash cepet
    COOLDOWN      // Capek abis dash
};

class ChargerEnemy : public BaseEnemy {
public:
    ChargerEnemy(int tier, Vector3 startPos);
    
    void Update(float dt, Vector3 playerPos) override;
    void Draw(Model& slimeModel, Model& cubeModel, Model& magnetModel, 
              Model& shadowPlane, Camera3D cam, Vector3 playerPos) override;

private:
    ChargerState mState;
    float mStateTimer;
    Vector3 mDashDirection;
    float mDashSpeed;
    Color bodyColor;
    float scaleSize;
};