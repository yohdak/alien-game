#pragma once
#include "raylib.h"
#include "raymath.h"

class CameraManager {
public:
    CameraManager();

    // Panggil ini tiap frame
    void Update(float dt, Vector3 targetPos);

    // Ambil object camera buat dipake di BeginMode3D()
    Camera3D GetCamera() const { return mCamera; }

    // Panggil ini pas ada ledakan: AddShake(0.5f)
    void AddShake(float amount);

    // Helper buat dapetin posisi mouse di lantai (Y=0)
    Vector3 GetMouseWorldPosition();

private:
    Camera3D mCamera;
    
    // Config
    Vector3 mOffset;        // Jarak kamera dari target (Zoom/Angle)
    float mSmoothSpeed;     // Seberapa "malas" kameranya (Lerp)
    float mMapLimit;        // Batas map (misal 50.0f)
    float mViewMargin;      // Jarak aman biar gak keliatan void
    
    // State
    float mShakeIntensity;
    Vector3 mCurrentShakeOffset;
};