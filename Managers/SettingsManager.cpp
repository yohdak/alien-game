#include "SettingsManager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/stat.h>
#include <algorithm>

SettingsManager::SettingsManager() 
    : mSettingsPath("saves/settings.txt")
{
    EnsureSettingsDirectory();
    LoadSettings(); // Auto-load on startup
}

// ============================================================================
// SAVE/LOAD
// ============================================================================

void SettingsManager::SaveSettings() {
    std::ofstream file(mSettingsPath);
    if (!file.is_open()) {
        std::cerr << "❌ ERROR: Cannot save settings to " << mSettingsPath << std::endl;
        return;
    }
    
    file << "MUSIC_VOLUME=" << mSettings.musicVolume << "\n";
    file << "SFX_VOLUME=" << mSettings.sfxVolume << "\n";
    file << "PIXEL_MODE=" << (mSettings.pixelMode ? "1" : "0") << "\n";
    file << "SCREEN_SHAKE=" << (mSettings.screenShake ? "1" : "0") << "\n";
    
    file.close();
    std::cout << "⚙️ SETTINGS SAVED" << std::endl;
}

void SettingsManager::LoadSettings() {
    std::ifstream file(mSettingsPath);
    if (!file.is_open()) {
        std::cout << "⚙️ No settings file, using defaults" << std::endl;
        return; // Use defaults
    }
    
    std::string line;
    while (std::getline(file, line)) {
        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;
        
        std::string key = line.substr(0, eqPos);
        std::string value = line.substr(eqPos + 1);
        
        try {
            if (key == "MUSIC_VOLUME") {
                mSettings.musicVolume = std::stof(value);
                mSettings.musicVolume = std::clamp(mSettings.musicVolume, 0.0f, 1.0f);
            }
            else if (key == "SFX_VOLUME") {
                mSettings.sfxVolume = std::stof(value);
                mSettings.sfxVolume = std::clamp(mSettings.sfxVolume, 0.0f, 1.0f);
            }
            else if (key == "PIXEL_MODE") {
                mSettings.pixelMode = (value == "1");
            }
            else if (key == "SCREEN_SHAKE") {
                mSettings.screenShake = (value == "1");
            }
        } catch (...) {
            std::cerr << "⚠️ WARNING: Invalid setting: " << line << std::endl;
        }
    }
    
    file.close();
    std::cout << "⚙️ SETTINGS LOADED (Music: " << (int)(mSettings.musicVolume * 100) 
              << "%, SFX: " << (int)(mSettings.sfxVolume * 100) << "%)" << std::endl;
}

// ============================================================================
// SETTERS
// ============================================================================

void SettingsManager::SetMusicVolume(float vol) {
    mSettings.musicVolume = std::clamp(vol, 0.0f, 1.0f);
    SaveSettings();
}

void SettingsManager::SetSFXVolume(float vol) {
    mSettings.sfxVolume = std::clamp(vol, 0.0f, 1.0f);
    SaveSettings();
}

void SettingsManager::SetPixelMode(bool enabled) {
    mSettings.pixelMode = enabled;
    SaveSettings();
}

void SettingsManager::SetScreenShake(bool enabled) {
    mSettings.screenShake = enabled;
    SaveSettings();
}

// ============================================================================
// UTILITIES
// ============================================================================

void SettingsManager::EnsureSettingsDirectory() {
    #ifdef _WIN32
        _mkdir("saves");
    #else
        mkdir("saves", 0755);
    #endif
}
