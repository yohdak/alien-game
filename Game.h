#pragma once

#include "raylib.h"
#include <vector>
#include <memory>
#include <string>

// --- SUB-SYSTEM INCLUDES ---
#include "Player/Player.h"
#include "Systems/WaveManager.h"
#include "Systems/ProjectileManager.h"
#include "Systems/ItemManager.h"
#include "Managers/ParticleSystem.h"
#include "Managers/AssetManager.h"
#include "Enemies/BaseEnemy.h"


// ✅ ENEMY INCLUDES
#include "Enemies/CubeWalker.h"
#include "Enemies/SlimeJumper.h"
#include "Enemies/ShooterEnemy.h"     // ✅ BARU
#include "Enemies/ChargerEnemy.h"     // ✅ BARU
#include "Enemies/ExploderEnemy.h"    // ✅ BARU
#include "Enemies/BossEnemy.h"        // ✅ BARU

#include "Managers/UIManager.h"

// --- ENUMS & STRUCTS ---

enum class GameState {
    SPLASH,
    LOADING,
    MAIN_MENU,
    SETTINGS,   // <--- Baru
    CREDITS,
    PLAYING,
    GAME_OVER,
    PAUSED,
    VICTORY       // ✅ BARU (Wave 25 clear)
};

enum class MenuOption {
    START = 0,
    SETTINGS,
    CREDITS,
    EXIT
};

struct XPGem {
    Vector3 position;
    float value;
    bool active;
    Vector3 velocity;
};

// --- GAME CLASS ---

class Game {
public:
    Game(int width, int height);
    ~Game();

    void Run();

private:
    void ProcessInput(float dt);
    void Update(float dt);
    void Draw();
    void ResetGame();

    // ✅ SPAWN METHODS (UPDATED)
    void SpawnEnemy(EnemySpawnEntry entry, Vector3 pos = {0, 0, 0});
    void SpawnBoss(int waveNumber, Vector3 pos);
    
    Texture2D GenerateShadowTexture();

private:

    bool mGameLoaded;      // Penanda apakah aset gameplay sudah diload
    int mLoadingFrameDelay; // Hack kecil untuk memastikan UI Loading muncul sebelum freeze loading
    void LoadGameplayContent();


    float mSplashTimer;
    Texture2D mSplashLogo;

    float mLoadingTimer;
    Texture2D mMenuBg;
    Texture2D mLoadingBg;
    
    MenuOption mCurrentMenuOption; // Pilihan menu saat ini (0-3)
    bool mGameRunning;


    // --- WINDOW & STATE ---
    int mScreenWidth;
    int mScreenHeight;
    GameState mState;
    Camera3D mCamera;

    // --- SYSTEMS ---
    AssetManager mAssets;
    Player mPlayer;
    WaveManager mWaveManager;
    ProjectileManager mProjectileManager;
    ItemManager mItemManager;
    ParticleSystem mParticles;
    UIManager mUI;

    // --- RENDERING ---
    Shader mGroundShader;
    Shader mSlimeShader;
    int mLightPosGroundLoc;
    int mLightPosSlimeLoc;
    int mViewPosSlimeLoc;
    Texture2D mShadowTexture;


    // --- ENTITIES ---
    std::vector<std::unique_ptr<BaseEnemy>> mEnemies;
    std::vector<std::unique_ptr<BaseEnemy>> mPendingEnemies;
    std::vector<XPGem> mGems;

    // --- GAMEPLAY VARIABLES ---
    float mScreenShakeIntensity;
    float mShootTimer;
    bool mWaveBonusClaimed;

    Music* mBgMusic;

    // --- PIXEL MODE ---
    RenderTexture2D mTarget;
    float mRenderScale;
    bool mPixelMode;
};