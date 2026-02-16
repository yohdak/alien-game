#pragma once
#include <string>

struct GameSettings {
    float musicVolume = 0.5f;      // 0.0 - 1.0
    float sfxVolume = 0.8f;        // 0.0 - 1.0
    bool pixelMode = true;         // Toggle pixel rendering
    bool screenShake = true;       // Toggle screen shake effect
};

class SettingsManager {
public:
    SettingsManager();
    
    // Save/Load
    void SaveSettings();
    void LoadSettings();
    
    // Getters
    const GameSettings& GetSettings() const { return mSettings; }
    float GetMusicVolume() const { return mSettings.musicVolume; }
    float GetSFXVolume() const { return mSettings.sfxVolume; }
    bool IsPixelMode() const { return mSettings.pixelMode; }
    bool IsScreenShakeEnabled() const { return mSettings.screenShake; }
    
    // Setters
    void SetMusicVolume(float vol);
    void SetSFXVolume(float vol);
    void SetPixelMode(bool enabled);
    void SetScreenShake(bool enabled);
    
private:
    GameSettings mSettings;
    std::string mSettingsPath;
    
    void EnsureSettingsDirectory();
};
