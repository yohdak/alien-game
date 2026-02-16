#pragma once
#include <string>
#include <vector>

// Forward declaration
class Player;

// --- SAVE DATA STRUCTURES ---

struct PlayerSaveData {
    int wave;
    int level;
    float currentXP;
    float nextLevelXP;
    float hp;
    float maxHp;
    int weaponType; // 0=Pistol, 1=Shotgun, 2=Minigun, 3=Bazooka
};

struct HighScoreEntry {
    int wave;
    int level;
    int score; // Calculated: wave * 1000 + level * 100
    std::string timestamp;
};

// --- SAVE MANAGER CLASS ---

class SaveManager {
public:
    SaveManager();
    
    // === SAVE OPERATIONS ===
    void SavePlayerProgress(const Player& player, int currentWave);
    void SaveHighScore(int wave, int level);
    
    // === LOAD OPERATIONS ===
    bool LoadPlayerProgress(PlayerSaveData& outData);
    std::vector<HighScoreEntry> LoadHighScores();
    
    // === UTILITIES ===
    bool HasSaveFile() const;
    void DeleteSaveFile();
    
private:
    std::string mSavePath;
    std::string mHighScorePath;
    
    void EnsureSaveDirectory();
    std::string GetTimestamp();
    
    // Helper parsing functions
    void WriteKeyValue(std::ofstream& file, const std::string& key, const std::string& value);
    bool ReadKeyValue(const std::string& line, std::string& outKey, std::string& outValue);
};
