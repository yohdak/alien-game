#pragma once
#include "BaseEnemy.h"
#include "raylib.h"

// 🐀 ENUM MODEL TYPE
enum class RatModelType {
    NORMAL,      // Rat.glb (Kecil-sedang, bisa warna-warni)
    HAMSTER,     // Hamster.glb (Sedang, oren)
    SPINY_BOSS   // Spiny_mouse.glb (Gede, buat boss/elite)
};

class Rat : public BaseEnemy {
public:
    Rat(int tierInput, Vector3 startPos);

    void Update(float dt, Vector3 playerPos) override;
    void Draw(Model& slimeModel, Model& cubeModel, Model& magnetModel, Model& shadowPlane, 
              Camera3D cam, Vector3 playerPos) override;

    // ✅ DRAW DENGAN MODEL TIKUS ASLI
    void DrawWithRatModels(Model& ratModel, Model& hamsterModel, Model& spinyModel, 
                          Model& shadowPlane, Vector3 playerPos);

private:
    float mAnimTimer;
    float mJumpTimer;
    float mJumpVelocity;
    bool mIsJumping;
    
    Color bodyColor;
    float scaleSize;
    
    // ✅ MODEL SELECTION
    RatModelType modelType;
};