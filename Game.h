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
#include "Managers/LevelManager.h"

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
    VICTORY,
    STORY_MODE  // ✅ NEW
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
    // --- STATE INITIALIZATION ORDER ---
    int mScreenWidth;
    int mScreenHeight;
    GameState mState;
    bool mGameRunning;
    
    float mSplashTimer;
    float mLoadingTimer;
    float mScreenShakeIntensity;
    float mShootTimer;
    bool mWaveBonusClaimed;
    
    bool mGameLoaded;
    int mLoadingFrameDelay;

    // --- SYSTEMS ---
    void LoadGameplayContent(); // Helper func
    
    // Assets & Resources
    Texture2D mSplashLogo;
    Texture2D mMenuBg;
    Texture2D mLoadingBg;
    
    // Managers
    MenuManager mMenuManager;      
    
    Camera3D mCamera;
    AssetManager mAssets;
    Player mPlayer;
    WaveManager mWaveManager;
    ProjectileManager mProjectileManager;
    ItemManager mItemManager;
    ParticleSystem mParticles;
    UIManager mUI;
    LevelManager mLevelManager;

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

    // --- AUDIO ---
    Music* mBgMusic;

    // --- PIXEL MODE ---
    RenderTexture2D mTarget;
    float mRenderScale;
    bool mPixelMode;
};