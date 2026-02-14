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
#include "Managers/UIManager.h"
#include "Managers/MenuManager.h" // ✅ BARU: Tambahkan ini

// ✅ ENEMY INCLUDES
#include "Enemies/BaseEnemy.h"
#include "Enemies/CubeWalker.h"
#include "Enemies/SlimeJumper.h"
#include "Enemies/ShooterEnemy.h"
#include "Enemies/ChargerEnemy.h"
#include "Enemies/ExploderEnemy.h"
#include "Enemies/BossEnemy.h"

// --- ENUMS & STRUCTS ---

enum class GameState {
    SPLASH,
    LOADING,
    MAIN_MENU,
    SETTINGS,
    CREDITS,
    PLAYING,
    GAME_OVER,
    PAUSED,
    VICTORY
};

// ❌ HAPUS: enum class MenuOption (Sudah diganti MenuManager)

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

    // ✅ SPAWN METHODS
    void SpawnEnemy(EnemySpawnEntry entry, Vector3 pos = {0, 0, 0});
    void SpawnBoss(int waveNumber, Vector3 pos);
    
    Texture2D GenerateShadowTexture();

private:
    bool mGameLoaded;
    int mLoadingFrameDelay;
    void LoadGameplayContent();

    float mSplashTimer;
    Texture2D mSplashLogo;

    float mLoadingTimer;
    Texture2D mMenuBg;
    Texture2D mLoadingBg;
    
    // ✅ GANTI: Variabel Menu lama diganti dengan Class Manager
    // MenuOption mCurrentMenuOption; <- Hapus ini
    MenuManager mMenuManager;      // <- Ganti ini
    
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