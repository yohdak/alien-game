// CubeWalker.h
#pragma once
#include "BaseEnemy.h"

class CubeWalker : public BaseEnemy {
public:
    CubeWalker(int tier, Vector3 startPos);
    void Update(float dt, Vector3 playerPos) override;
    
    // Parameter draw tetep sama, tapi dalemnya nanti kita pake warna beda
    void Draw(Model& slimeModel, Model& cubeModel, Model& magnetModel, Model& shadowPlane, Camera3D cam, Vector3 playerPos) override;

    bool CanSplit() const override { return canSplitStatus; }

private:
    Color bodyColor;  // Warna badan (Merah, Hijau, Hitam?)
    float scaleSize;  // Ukuran badan (Kecil/Gede)
    bool canSplitStatus;
};