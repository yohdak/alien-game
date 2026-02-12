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
    // Di dalam class AssetManager { public: ... }
    bool IsSoundReady(std::string name);


private:
    std::map<std::string, Model> mModels;
    std::map<std::string, Texture2D> mTextures;
    std::map<std::string, Music> mMusics;
    std::map<std::string, Sound> mSounds;
};