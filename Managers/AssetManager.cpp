#include "AssetManager.h"
#include <cstdio>
#include "raymath.h"
#include "AudioGenerator.h"

// --- HELPER FUNCTION ---
static Model LoadModelSafe(const char* path, Mesh fallbackMesh) {
    if (FileExists(path)) {
        return LoadModel(path);
    }
    TraceLog(LOG_WARNING, "ASSET MISSING: '%s' fallback used.", path);
    return LoadModelFromMesh(fallbackMesh);
}

static Texture2D LoadTextureSafe(const char* path) {
    if (FileExists(path)) {
        return LoadTexture(path);
    }
    TraceLog(LOG_WARNING, "ASSET MISSING: '%s' dummy used.", path);
    Image img = GenImageChecked(64, 64, 8, 8, DARKGRAY, GRAY);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}
// -----------------------

AssetManager::AssetManager() {}
AssetManager::~AssetManager() { UnloadAll(); }

void AssetManager::LoadAll() {
    // 1. MODELS
    mModels["ayam"]   = LoadModelSafe("Resources/Models/Chicken.glb", GenMeshCylinder(0.5f, 1.0f, 16));
    mModels["magnet"] = LoadModelSafe("Resources/Models/Magnet.glb",  GenMeshCube(0.5f, 0.5f, 0.5f));
    
    // 2. PROCEDURAL & DIRECT LOAD
    mModels["cube"]        = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    mModels["slime"]       = LoadModelFromMesh(GenMeshSphere(1.0f, 32, 32));
    mModels["ground"]      = LoadModelFromMesh(GenMeshPlane(100.0f, 100.0f, 1, 1));
    mModels["shadow_plane"] = LoadModelFromMesh(GenMeshPlane(1.0f, 1.0f, 1, 1));

    // 3. TEXTURES
    mTextures["ground"]    = LoadTextureSafe("ground.png");

    // 4. AUDIO
    mMusics["bgm"] = LoadMusicStream("Resources/.mp3");
    mMusics["bgm"].looping = true;

    AudioGenerator::GenerateAllSounds(mSounds);

    for (auto& entry : mSounds) {
        SetSoundVolume(entry.second, 0.2f);
    }

    mSounds["gun"] = LoadSound("Resources/gunshoot.mp3");
    SetSoundVolume(mSounds["gun"], 0.02f);

    mSounds["crack"] = LoadSound("Resources/zapsplat_food_egg_raw_crack_open_yolk_squelch_110912.mp3");
    SetSoundVolume(mSounds["crack"], 0.02f);


}

void AssetManager::UnloadAll() {
    for (auto& pair : mModels) UnloadModel(pair.second);
    for (auto& pair : mTextures) UnloadTexture(pair.second);
    for (auto& pair : mMusics) UnloadMusicStream(pair.second);
    mModels.clear();
    mTextures.clear();
    mMusics.clear();
}

Model& AssetManager::GetModel(const std::string& name) {
    if (mModels.find(name) == mModels.end()) return mModels["cube"]; 
    return mModels[name];
}

Texture2D& AssetManager::GetTexture(const std::string& name) {
    if (mTextures.find(name) == mTextures.end()) return mTextures["crosshair"];
    return mTextures[name];
}

Music& AssetManager::GetMusic(const std::string& name) {
    if (mMusics.find(name) == mMusics.end()) {
        static Music dummy = { 0 };
        return dummy;
    }
    return mMusics[name];
}

Sound& AssetManager::GetSound(const std::string& name) {
    if (mSounds.find(name) == mSounds.end()) {
        static Sound dummy = { 0 }; 
        return dummy;
    }
    return mSounds[name];
}

bool AssetManager::IsSoundReady(std::string name) {
    // 1. Cek apakah key string ada di dalam map
    if (mSounds.find(name) == mSounds.end()) {
        return false;
    }

    // 2. Cek manual apakah sound memiliki data (frameCount > 0)
    // Ini cara manual pengganti ::IsSoundReady()
    return (mSounds[name].frameCount > 0);
}