#pragma once
#include "BaseEnemy.h"
#include "raylib.h"
#include <vector>

enum class BossType {
    TANK_BOSS,          // Boss 1 (Wave 20): Tanky, slow, melee
    SUMMONER_BOSS,      // Boss 2 (Wave 40): Spawn minions
    ARTILLERY_BOSS,     // Boss 3 (Wave 60): Barrage projectiles
    TELEPORTER_BOSS,    // Boss 4 (Wave 80): Teleport + charge
    ULTIMATE_BOSS       // Boss 5 (Wave 100): Semua skill + rage mode
};

enum class BossPhase {
    PHASE_1,    // HP > 66%
    PHASE_2,    // HP 33%-66%
    PHASE_3     // HP < 33% (Rage mode)
};

struct BossProjectile {
    Vector3 position;
    Vector3 direction;
    float speed;
    float damage;
    float radius;
    bool active;
    float lifeTime;
};

class BossEnemy : public BaseEnemy {
public:
    // hp dan maxHp diwarisi dari BaseEnemy melalui constructor
    BossEnemy(BossType type, Vector3 startPos, int waveNumber);
    
    void Update(float dt, Vector3 playerPos) override;
    void Draw(Model& slimeModel, Model& cubeModel, Model& magnetModel, 
              Model& shadowPlane, Camera3D cam, Vector3 playerPos) override;

    BossType GetBossType() const { return mBossType; }
    BossPhase GetPhase() const { return mPhase; }
    std::vector<BossProjectile>& GetProjectiles() { return mProjectiles; }
    bool ShouldSpawnMinion() const { return mShouldSpawnMinion; }
    void ConsumeSpawnSignal() { mShouldSpawnMinion = false; }

private:
    void UpdatePhase();
    void ExecuteAttackPattern(float dt, Vector3 playerPos);
    
    // Attack patterns
    void AttackPattern_Melee(float dt, Vector3 playerPos);
    void AttackPattern_Summon(float dt);
    void AttackPattern_Barrage(float dt, Vector3 playerPos);
    void AttackPattern_Teleport(float dt, Vector3 playerPos);
    void AttackPattern_Ultimate(float dt, Vector3 playerPos);

private:
    BossType mBossType;
    BossPhase mPhase;
    
    // Attack state
    float mAttackTimer;
    float mAttackCooldown;
    int mAttackCycle;
    
    // Movement
    Vector3 mTargetPos;
    bool mIsCharging;
    float mChargeSpeed;
    
    // Projectiles
    std::vector<BossProjectile> mProjectiles;
    
    // Summoning
    bool mShouldSpawnMinion;
    float mSummonTimer;
    
    // Teleport
    float mTeleportTimer;
    bool mIsTeleporting;
    
    // Visuals
    Color bodyColor;
    float scaleSize;
    float glowIntensity;
};