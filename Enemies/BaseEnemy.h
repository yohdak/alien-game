#pragma once
#include "raylib.h"

class BaseEnemy {
public:
    virtual ~BaseEnemy(); // Destructor dipindah ke cpp

    // --- LOGIC UTAMA ---
    // UpdateFlash: Helper untuk mengurangi timer kedip (Wajib dipanggil di Update anak)
    void UpdateFlash(float dt); 
    
    // GetRenderColor: Helper untuk menentukan warna (Putih pas kena hit, normal pas enggak)
    Color GetRenderColor(Color originalColor);

    virtual void Update(float dt, Vector3 playerPos) = 0;
    
    virtual void Draw(Model& slimeModel, Model& cubeModel, Model& magnetModel, 
                      Model& shadowPlane, Camera3D cam, Vector3 playerPos) = 0;

    // --- GETTERS & SETTERS ---
    Vector3 GetPosition() const { return position; }
    float GetRadius() const { return radius; }
    bool IsActive() const { return active; }
    float GetHealth() const { return hp; }
    float GetMaxHealth() const { return maxHp; }
    int GetXPReward() const { return xpReward; }
    int GetTier() const { return tier; }
    
    virtual bool CanSplit() const { return false; }
    
    // Dipindah ke CPP
    virtual void TakeDamage(float amount); 

protected:
    // Constructor
    BaseEnemy(int tierInput, Vector3 startPos);

    Vector3 position;
    Vector3 velocity;
    float hp;
    float maxHp;
    float speed;
    float radius;
    int tier;
    bool active;
    int xpReward;

    // 🔥 Variabel Timer Kedip
    float flashTimer; 
};