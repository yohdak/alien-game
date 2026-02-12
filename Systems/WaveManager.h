#pragma once
#include "raylib.h"
#include <vector>

enum class WaveState {
    WAITING,    // Jeda antar wave
    SPAWNING,   // Lagi spawn musuh
    FIGHTING,   // Musuh masih hidup
    COMPLETED   // Wave selesai
};

enum class WaveType {
    NORMAL,      // Wave biasa
    BOSS         // Tiap 20 wave
};

// Enemy types yang bisa di-spawn
enum class EnemySpawnType {
    CUBE_WALKER,
    SHOOTER,
    CHARGER,
    EXPLODER,
    SLIME_JUMPER,
    MINI_BOSS,
    BOSS
};

struct EnemySpawnEntry {
    EnemySpawnType type;
    int tier;
    int count;
};

struct WaveConfig {
    int waveNumber;
    WaveType waveType;
    
    std::vector<EnemySpawnEntry> enemies;
    int totalEnemies;
    
    float spawnInterval;
    float waveDelay; // Delay before wave starts
};

class WaveManager {
public:
    WaveManager();

    void Update(float dt, int playerLevel, int aliveEnemyCount);

    // Spawn control
    bool ShouldSpawn() const { return readyToSpawn; }
    EnemySpawnEntry GetNextSpawn();
    void ConsumeSpawnSignal() { 
        readyToSpawn = false; 
        spawnedThisWave++; 
    }

    // Wave control
    void StartNextWave(int playerLevel);

    // Getters
    int GetCurrentWave() const { return currentWave; }
    WaveState GetState() const { return state; }
    WaveType GetWaveType() const { return waveConfig.waveType; }
    float GetWaveTimer() const { return waveTimer; }
    int GetRemainingEnemies() const { return waveConfig.totalEnemies - spawnedThisWave; }
    int GetWaveBonusXP() const;

    void Reset();

    void ForceSkipWave();

private:
    void GenerateWaveConfig(int waveNum, int playerLevel);
    WaveType DetermineWaveType(int waveNum);
    
    // Enemy composition generators
    void AddNormalWaveEnemies(int waveNum);
    void AddMiniBossWave(int waveNum);
    void AddBossWave(int waveNum);
    

private:
    WaveState state;
    WaveConfig waveConfig;

    int currentWave;
    int spawnedThisWave;
    int currentSpawnIndex; // Track urutan spawn

    float spawnTimer;
    float waveTimer;
    bool readyToSpawn;
};