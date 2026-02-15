#pragma once
#include "raylib.h"
#include "raymath.h"
#include <vector>

#include "../Systems/ProjectileManager.h"

class ProjectileManager;

enum class WeaponType {
    PISTOL,     
    SHOTGUN,    
    MINIGUN,    
    BAZOOKA     
};

struct PlayerStats {
    float fireRate;        
    int projectileCount;   
    float spreadAngle;     
    float damage;          
    float bulletSpeed;     
    float bulletSize;      
    float moveSpeed;       
    ProjectileType bulletType; 
};

class Player {
public:
    Player();
    void Reset();
    void Update(float dt);
    // 🔥 Metode baru untuk sistem collision physics
    Vector3 GetFuturePosition(float dt);
    void UpdateRotationOnly(float dt);
    
    void Draw(Model& ayamModel, Camera3D cam, Texture2D shadow);

    // Shooting & Dash System
    void TryShoot(Vector3 targetPos, ProjectileManager& projManager, float dt);
    void TryDash(Vector3 mousePos);
    bool IsDashing() const { return dashTime > 0.0f; }
    
    // Weapon Switching
    void SwitchWeapon(WeaponType type);

    // XP & Level
    void AddXP(float amount);
    void LevelUp();

    // Combat
    void TakeDamage(float amount);
    void Heal(float amount);
    bool IsDead() const { return hp <= 0; }

    // Magnet Buff
    void ActivateMagnetBuff(float duration);
    bool HasMagnetBuff() const { return magnetBuffTimer > 0; }

    // Getters
    Vector3 GetPosition() const { return position; }
    void SetPosition(Vector3 pos) { position = pos; } 

    void PushBack(Vector3 direction, float distance) {
        position = Vector3Add(position, Vector3Scale(direction, distance));
    }

    float GetHp() const { return hp; }
    float GetMaxHp() const { return maxHp; }
    int GetLevel() const { return level; }
    float GetCurrentXP() const { return currentXP; }
    float GetNextLevelXP() const { return nextLevelXP; }
    WeaponType GetCurrentWeapon() const { return currentWeapon; }

private:
    Vector3 position;
    float rotationY;
    float walkTimer;
    float shootTimer; 

    // Dash Variables
    float dashCooldown;
    float dashTime;
    Vector3 dashDir;

    // Stats RPG
    float hp;
    float maxHp;
    int level;
    float currentXP;
    float nextLevelXP;

    // Weapon Logic
    WeaponType currentWeapon; 
    PlayerStats stats;        

    // Buffs
    float magnetBuffTimer;
};