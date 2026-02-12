#include "WaveManager.h"
#include "../Utils/MathUtils.h"
#include <algorithm>
#include <iostream>

WaveManager::WaveManager() {
    Reset();
}

void WaveManager::Reset() {
    state = WaveState::WAITING;
    currentWave = 0;
    spawnedThisWave = 0;
    currentSpawnIndex = 0;
    spawnTimer = 0.0f;
    waveTimer = 1.0f; // 3 detik sebelum wave 1
    readyToSpawn = false;
    waveConfig = {};
}

void WaveManager::Update(float dt, int playerLevel, int aliveEnemyCount) {
    
    switch (state) {
        
        case WaveState::WAITING: {
            waveTimer -= dt;
            if (waveTimer <= 0) {
                StartNextWave(playerLevel);
            }
            break;
        }

        case WaveState::SPAWNING: {
            // Cek apakah semua musuh udah di-spawn
            if (spawnedThisWave >= waveConfig.totalEnemies) {
                state = WaveState::FIGHTING;
                break;
            }

            // Spawn timer
            spawnTimer -= dt;
            if (spawnTimer <= 0) {
                readyToSpawn = true;
                spawnTimer = waveConfig.spawnInterval;
            }
            break;
        }

        case WaveState::FIGHTING: {
            // Tunggu semua musuh mati
            if (aliveEnemyCount == 0) {
                state = WaveState::COMPLETED;
                waveTimer = 1.0f; // 5 detik sebelum wave berikutnya
            }
            break;
        }

        case WaveState::COMPLETED: {
            waveTimer -= dt;
            if (waveTimer <= 0) {
                state = WaveState::WAITING;
                
                // Wave delay scaling (makin lama makin pendek)
                if (currentWave < 10) waveTimer = 1.0f;
                else if (currentWave < 30) waveTimer = 1.0f;
                else if (currentWave < 60) waveTimer = 1.0f;
                else waveTimer = 1.0f;
            }
            break;
        }
    }
}

void WaveManager::StartNextWave(int playerLevel) {
    currentWave++;
    spawnedThisWave = 0;
    currentSpawnIndex = 0;
    
    GenerateWaveConfig(currentWave, playerLevel);
    
    state = WaveState::SPAWNING;
    spawnTimer = 0.0f; // Langsung spawn
    
    std::cout << "=== WAVE " << currentWave << " ===" << std::endl;
    std::cout << "Type: " << (int)waveConfig.waveType << std::endl;
    std::cout << "Total enemies: " << waveConfig.totalEnemies << std::endl;
}

WaveType WaveManager::DetermineWaveType(int waveNum) {
    // Boss tiap 5 wave
    if (waveNum % 5 == 0) {
        return WaveType::BOSS;
    }
    
    return WaveType::NORMAL;
}

void WaveManager::GenerateWaveConfig(int waveNum, int playerLevel) {
    waveConfig = {};
    waveConfig.waveNumber = waveNum;
    waveConfig.waveType = DetermineWaveType(waveNum);
    waveConfig.enemies.clear();

    // 1. Generate Base Enemies
    if (waveConfig.waveType == WaveType::BOSS) {
        AddBossWave(waveNum);
    } else {
        AddNormalWaveEnemies(waveNum);
    }

    // 2. 🔥 SLIME JUMPER (Loot Goblin)
    // Chance dinaikkan ke 50% dan menggunakan .insert() ke .begin()
    // agar Slime menjadi musuh PERTAMA yang spawn di wave ini.
    if (GetRandomValue(0, 100) < 50) { 
        int slimeCount = GetRandomValue(2, 4); 
        
        waveConfig.enemies.insert(
            waveConfig.enemies.begin(), 
            {EnemySpawnType::SLIME_JUMPER, 1, slimeCount}
        );
        
        waveConfig.totalEnemies += slimeCount;
    }

    // 3. Spawn Interval Scaling
    if (waveNum <= 5) {
        waveConfig.spawnInterval = 0.8f;
    } else if (waveNum <= 15) {
        waveConfig.spawnInterval = 0.5f;
    } else {
        waveConfig.spawnInterval = 0.3f; // Fast spawn late game
    }
}
// === NORMAL WAVE ===
void WaveManager::AddNormalWaveEnemies(int waveNum) {
    // Aggressive scaling: Wave 1: 15, Wave 24: ~80
    int baseCount = 15 + (waveNum * 3);
    
    // 🔥 ENEMY UNLOCK PROGRESSION (Every 5 waves)
    // Wave 1-4:   CubeWalker only
    // Wave 6-9:   CubeWalker + Shooter
    // Wave 11-14: CubeWalker + Shooter + Charger
    // Wave 16+:   ALL TYPES
    
    if (waveNum <= 4) {
        // PHASE 1: Tutorial (CubeWalker only)
        waveConfig.enemies.push_back({EnemySpawnType::CUBE_WALKER, 1, baseCount});
        waveConfig.totalEnemies = baseCount;
    }
    else if (waveNum <= 9) {
        // PHASE 2: Shooter unlocked (Wave 6+)
        int cubeCount = (int)(baseCount * 0.65f);
        int shooterCount = (int)(baseCount * 0.35f);
        
        waveConfig.enemies.push_back({EnemySpawnType::CUBE_WALKER, 1, cubeCount});
        waveConfig.enemies.push_back({EnemySpawnType::SHOOTER, 1, shooterCount});
        waveConfig.totalEnemies = cubeCount + shooterCount;
    }
    else if (waveNum <= 14) {
        // PHASE 3: Charger unlocked (Wave 11+)
        int cubeCount = (int)(baseCount * 0.45f);
        int shooterCount = (int)(baseCount * 0.35f);
        int chargerCount = (int)(baseCount * 0.2f);
        
        waveConfig.enemies.push_back({EnemySpawnType::CUBE_WALKER, 1, cubeCount});
        waveConfig.enemies.push_back({EnemySpawnType::SHOOTER, 1, shooterCount});
        waveConfig.enemies.push_back({EnemySpawnType::CHARGER, 1, chargerCount});
        waveConfig.totalEnemies = cubeCount + shooterCount + chargerCount;
    }
    else {
        // PHASE 4: Exploder unlocked (Wave 16+) - HELL MODE
        int cubeCount = (int)(baseCount * 0.35f);
        int shooterCount = (int)(baseCount * 0.3f);
        int chargerCount = (int)(baseCount * 0.2f);
        int exploderCount = (int)(baseCount * 0.15f);
        
        waveConfig.enemies.push_back({EnemySpawnType::CUBE_WALKER, 1, cubeCount});
        waveConfig.enemies.push_back({EnemySpawnType::SHOOTER, 1, shooterCount});
        waveConfig.enemies.push_back({EnemySpawnType::CHARGER, 1, chargerCount});
        waveConfig.enemies.push_back({EnemySpawnType::EXPLODER, 1, exploderCount});
        waveConfig.totalEnemies = cubeCount + shooterCount + chargerCount + exploderCount;
    }
    
    // 🔥 TIER SCALING
    // Wave 10+: Add Tier 2 (25%)
    // Wave 20+: Add Tier 3 (15%)
    
    if (waveNum >= 10 && waveNum < 20) {
        int tier2Count = (int)(waveConfig.totalEnemies * 0.25f);
        waveConfig.enemies.push_back({EnemySpawnType::CUBE_WALKER, 2, tier2Count});
        waveConfig.totalEnemies += tier2Count;
    }
    else if (waveNum >= 20) {
        int tier2Count = (int)(waveConfig.totalEnemies * 0.3f);
        int tier3Count = (int)(waveConfig.totalEnemies * 0.15f);
        
        waveConfig.enemies.push_back({EnemySpawnType::CUBE_WALKER, 2, tier2Count});
        waveConfig.enemies.push_back({EnemySpawnType::SHOOTER, 3, tier3Count});
        waveConfig.totalEnemies += tier2Count + tier3Count;
    }
}


// === BOSS WAVE ===
void WaveManager::AddBossWave(int waveNum) {
    // 1 Boss + swarm (30% normal wave size)
    
    waveConfig.enemies.push_back({EnemySpawnType::BOSS, 4, 1});
    
    int swarmCount = (int)((15 + waveNum * 3) * 0.3f);
    waveConfig.enemies.push_back({EnemySpawnType::CUBE_WALKER, 1, swarmCount});
    
    waveConfig.totalEnemies = 1 + swarmCount;
    
    // Boss wave: slower spawn
    waveConfig.spawnInterval = 1.0f;
}
EnemySpawnEntry WaveManager::GetNextSpawn() {
    // Iterate through enemy list sequentially
    
    int accumulated = 0;
    for (auto& entry : waveConfig.enemies) {
        accumulated += entry.count;
        
        if (spawnedThisWave < accumulated) {
            return entry;
        }
    }
    
    // Fallback (shouldn't reach here)
    return {EnemySpawnType::CUBE_WALKER, 1, 1};
}

int WaveManager::GetWaveBonusXP() const {
    int baseBonus = currentWave * 50;
    
    // Bonus multiplier
    if (waveConfig.waveType == WaveType::BOSS) {
        return baseBonus * 5; // Boss wave: 5x XP
    } 
    
    return baseBonus;
}

void WaveManager::ForceSkipWave() {
    state = WaveState::COMPLETED;
    // Pastikan spawner berhenti
    spawnedThisWave = waveConfig.totalEnemies; 
}