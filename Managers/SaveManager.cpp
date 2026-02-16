#include "SaveManager.h"
#include "../Player/Player.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>

SaveManager::SaveManager() 
    : mSavePath("saves/player_save.txt")
    , mHighScorePath("saves/highscores.txt")
{
    EnsureSaveDirectory();
}

// ============================================================================
// SAVE OPERATIONS
// ============================================================================

void SaveManager::SavePlayerProgress(const Player& player, int currentWave) {
    std::ofstream file(mSavePath);
    if (!file.is_open()) {
        std::cerr << "❌ ERROR: Cannot create save file: " << mSavePath << std::endl;
        return;
    }

    WriteKeyValue(file, "WAVE", std::to_string(currentWave));
    WriteKeyValue(file, "LEVEL", std::to_string(player.GetLevel()));
    WriteKeyValue(file, "CURRENT_XP", std::to_string(player.GetCurrentXP()));
    WriteKeyValue(file, "NEXT_LEVEL_XP", std::to_string(player.GetNextLevelXP()));
    WriteKeyValue(file, "HP", std::to_string(player.GetHp()));
    WriteKeyValue(file, "MAX_HP", std::to_string(player.GetMaxHp()));
    WriteKeyValue(file, "WEAPON", std::to_string((int)player.GetCurrentWeapon()));

    file.close();
    std::cout << "💾 SAVED: Player progress at Wave " << currentWave << std::endl;
}

void SaveManager::SaveHighScore(int wave, int level) {
    // 1. Load existing high scores
    std::vector<HighScoreEntry> scores = LoadHighScores();
    
    // 2. Add new score
    HighScoreEntry newEntry;
    newEntry.wave = wave;
    newEntry.level = level;
    newEntry.score = wave * 1000 + level * 100;
    newEntry.timestamp = GetTimestamp();
    
    scores.push_back(newEntry);
    
    // 3. Sort descending by score
    std::sort(scores.begin(), scores.end(), [](const HighScoreEntry& a, const HighScoreEntry& b) {
        return a.score > b.score;
    });
    
    // 4. Keep only top 5
    if (scores.size() > 5) {
        scores.resize(5);
    }
    
    // 5. Write back to file
    std::ofstream file(mHighScorePath);
    if (!file.is_open()) {
        std::cerr << "❌ ERROR: Cannot write high scores: " << mHighScorePath << std::endl;
        return;
    }
    
    for (const auto& entry : scores) {
        file << "SCORE=" << entry.score 
             << "|WAVE=" << entry.wave 
             << "|LEVEL=" << entry.level 
             << "|TIME=" << entry.timestamp << "\n";
    }
    
    file.close();
    std::cout << "🏆 HIGH SCORE SAVED: Wave " << wave << ", Level " << level 
              << " (Score: " << newEntry.score << ")" << std::endl;
}

// ============================================================================
// LOAD OPERATIONS
// ============================================================================

bool SaveManager::LoadPlayerProgress(PlayerSaveData& outData) {
    std::ifstream file(mSavePath);
    if (!file.is_open()) {
        return false; // No save file exists
    }

    std::string line;
    bool success = true;
    
    // Initialize with defaults
    outData = PlayerSaveData{1, 1, 0.0f, 100.0f, 100.0f, 100.0f, 0};
    
    while (std::getline(file, line)) {
        std::string key, value;
        if (!ReadKeyValue(line, key, value)) continue;
        
        try {
            if (key == "WAVE") outData.wave = std::stoi(value);
            else if (key == "LEVEL") outData.level = std::stoi(value);
            else if (key == "CURRENT_XP") outData.currentXP = std::stof(value);
            else if (key == "NEXT_LEVEL_XP") outData.nextLevelXP = std::stof(value);
            else if (key == "HP") outData.hp = std::stof(value);
            else if (key == "MAX_HP") outData.maxHp = std::stof(value);
            else if (key == "WEAPON") outData.weaponType = std::stoi(value);
        } catch (...) {
            std::cerr << "⚠️ WARNING: Invalid save data format in line: " << line << std::endl;
            success = false;
        }
    }
    
    file.close();
    
    if (success) {
        std::cout << "📂 LOADED: Save file (Wave " << outData.wave << ", Level " << outData.level << ")" << std::endl;
    }
    
    return success;
}

std::vector<HighScoreEntry> SaveManager::LoadHighScores() {
    std::vector<HighScoreEntry> scores;
    
    std::ifstream file(mHighScorePath);
    if (!file.is_open()) {
        return scores; // No high scores yet
    }
    
    std::string line;
    while (std::getline(file, line)) {
        HighScoreEntry entry;
        
        // Parse format: SCORE=5300|WAVE=5|LEVEL=3|TIME=2026-02-16_11:30:00
        std::istringstream ss(line);
        std::string token;
        
        while (std::getline(ss, token, '|')) {
            size_t eqPos = token.find('=');
            if (eqPos == std::string::npos) continue;
            
            std::string key = token.substr(0, eqPos);
            std::string value = token.substr(eqPos + 1);
            
            try {
                if (key == "SCORE") entry.score = std::stoi(value);
                else if (key == "WAVE") entry.wave = std::stoi(value);
                else if (key == "LEVEL") entry.level = std::stoi(value);
                else if (key == "TIME") entry.timestamp = value;
            } catch (...) {
                std::cerr << "⚠️ WARNING: Invalid high score line: " << line << std::endl;
                continue;
            }
        }
        
        scores.push_back(entry);
    }
    
    file.close();
    
    std::cout << "🏆 LOADED: " << scores.size() << " high score entries" << std::endl;
    return scores;
}

// ============================================================================
// UTILITIES
// ============================================================================

bool SaveManager::HasSaveFile() const {
    std::ifstream file(mSavePath);
    return file.good();
}

void SaveManager::DeleteSaveFile() {
    if (std::remove(mSavePath.c_str()) == 0) {
        std::cout << "🗑️ DELETED: Save file cleared" << std::endl;
    }
}

void SaveManager::EnsureSaveDirectory() {
    // Create "saves" directory if it doesn't exist
    #ifdef _WIN32
        _mkdir("saves");
    #else
        mkdir("saves", 0755);
    #endif
}

std::string SaveManager::GetTimestamp() {
    std::time_t now = std::time(nullptr);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d_%H:%M:%S", std::localtime(&now));
    return std::string(buf);
}

// ============================================================================
// HELPER METHODS
// ============================================================================

void SaveManager::WriteKeyValue(std::ofstream& file, const std::string& key, const std::string& value) {
    file << key << "=" << value << "\n";
}

bool SaveManager::ReadKeyValue(const std::string& line, std::string& outKey, std::string& outValue) {
    size_t eqPos = line.find('=');
    if (eqPos == std::string::npos) return false;
    
    outKey = line.substr(0, eqPos);
    outValue = line.substr(eqPos + 1);
    return true;
}
