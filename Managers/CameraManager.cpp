#include "CameraManager.h"
#include <cmath>
#include <algorithm> // Untuk fminf, Clamp
#include "../Utils/MathUtils.h"

CameraManager::CameraManager() {
    // 1. Setup Default Camera Raylib
    mCamera = { 0 };
    mCamera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    mCamera.fovy = 45.0f;
    mCamera.projection = CAMERA_PERSPECTIVE;

    // 2. Config "Rasa" Kamera
    // Posisi Isometric (Naik 35, Mundur 25)
    mOffset = (Vector3){ 0.0f, 35.0f, 25.0f }; 
    
    // Speed (3.0f = Berat/Cinematic, 10.0f = Responsif)
    mSmoothSpeed = 3.0f; 
    
    // Anti-Void Config (Sesuaikan sama Grid Map lu)
    mMapLimit = 50.0f;   // Asumsi map dari -50 sampe 50
    mViewMargin = 18.0f; // Padding biar gak mentok

    mShakeIntensity = 0.0f;
    mCurrentShakeOffset = {0,0,0};
}

void CameraManager::Update(float dt, Vector3 followTarget) {
    
    // --- 1. SHAKE DECAY ---
    if (mShakeIntensity > 0) {
        mShakeIntensity -= 5.0f * dt; // Berkurang seiring waktu
        if (mShakeIntensity < 0) mShakeIntensity = 0;
    }

    // Hitung offset getar acak
    mCurrentShakeOffset = { 
        GetRandomFloat(-1, 1) * mShakeIntensity, 
        GetRandomFloat(-1, 1) * mShakeIntensity, 
        GetRandomFloat(-1, 1) * mShakeIntensity 
    };

    // --- 2. MOUSE PEEK (Ngintip) ---
    // Kita hitung posisi mouse di dunia nyata dulu
    Vector3 mousePos = GetMouseWorldPosition();
    Vector3 lookDir = Vector3Subtract(mousePos, followTarget);
    
    // Batasi ngintip maksimal 12 meter
    float maxPeek = 12.0f;
    if (Vector3Length(lookDir) > maxPeek) {
        lookDir = Vector3Scale(Vector3Normalize(lookDir), maxPeek);
    }

    // Target akhir = Posisi Player + 15% arah mouse
    Vector3 desiredPos = Vector3Add(followTarget, Vector3Scale(lookDir, 0.15f));

    // --- 3. ANTI-VOID (Clamping) ---
    float minVal = -mMapLimit + mViewMargin;
    float maxVal =  mMapLimit - mViewMargin;
    
    desiredPos.x = Clamp(desiredPos.x, minVal, maxVal);
    desiredPos.z = Clamp(desiredPos.z, minVal, maxVal);

    // --- 4. SMOOTH MOVEMENT (Lerp) ---
    mCamera.target.x = Lerp(mCamera.target.x, desiredPos.x, mSmoothSpeed * dt);
    mCamera.target.z = Lerp(mCamera.target.z, desiredPos.z, mSmoothSpeed * dt);
    mCamera.target.y = 0.0f; // Selalu lihat ke lantai rata

    // --- 5. FINALIZE ---
    // Masukkan efek getar ke target kamera
    Vector3 finalTarget = Vector3Add(mCamera.target, mCurrentShakeOffset);

    // Set posisi kamera selalu relatif terhadap target (+Offset)
    mCamera.position = Vector3Add(finalTarget, mOffset);
    mCamera.target = finalTarget; // Update target biar ikut getar juga
}

void CameraManager::AddShake(float amount) {
    mShakeIntensity = amount;
    // Cap biar gak pusing (Maksimal 1.0f)
    if (mShakeIntensity > 1.0f) mShakeIntensity = 1.0f;
}

Vector3 CameraManager::GetMouseWorldPosition() {
    // Raycast dari layar ke bidang datar Y=0
    Ray ray = GetScreenToWorldRay(GetMousePosition(), mCamera);
    
    // Cek tabrakan dengan lantai
    if (ray.direction.y != 0) {
        float t = -ray.position.y / ray.direction.y;
        if (t >= 0) {
            return Vector3Add(ray.position, Vector3Scale(ray.direction, t));
        }
    }
    return mCamera.target; // Fallback
}