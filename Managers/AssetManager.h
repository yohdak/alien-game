#pragma once
#include "raylib.h"
#include <map>
#include <string>

class AssetManager {
public:
    AssetManager();
    ~AssetManager();

    // Fungsi Load Massal
    void LoadAll();
    void UnloadAll();

    // Getter untuk Model & Texture
    Model& GetModel(const std::string& name);
    Texture2D& GetTexture(const std::string& name);
    Music& GetMusic(const std::string& name);
    Sound& GetSound(const std::string& name);
    bool IsSoundReady(std::string name);
    
    // SFX Volume Control
    void SetMasterSFXVolume(float volume);  // Set master SFX volume (0.0-1.0)
    void ApplySFXVolumeToAll();  // Apply master volume to all loaded sounds


private:
    std::map<std::string, Model> mModels;
    std::map<std::string, Texture2D> mTextures;
    std::map<std::string, Music> mMusics;
    std::map<std::string, Sound> mSounds;
    
    std::string mSelectedBGMKey;  // Key of selected BGM music
    float mMasterSFXVolume;       // Master SFX volume multiplier
};