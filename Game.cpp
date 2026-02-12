#include "Game.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include "rlgl.h"

#include "Enemies/CubeWalker.h"
#include "Enemies/SlimeJumper.h"
#include "Enemies/ShooterEnemy.h"
#include "Enemies/ChargerEnemy.h"
#include "Enemies/ExploderEnemy.h"
#include "Enemies/BossEnemy.h"
#include "Resources/ShaderSource.h"
#include "Utils/MathUtils.h"

Game::Game(int width, int height) 
    : mScreenWidth(width), mScreenHeight(height)
    , mState(GameState::SPLASH)           // Mulai dari Splash
    , mCurrentMenuOption(MenuOption::START)
    , mGameRunning(true)
    , mSplashTimer(0.0f)
    , mLoadingTimer(0.0f)
    , mScreenShakeIntensity(0.0f)
    , mShootTimer(0.0f)
    , mWaveBonusClaimed(false)
    , mGameLoaded(false)      // Belum load aset berat
    , mLoadingFrameDelay(0)   // Reset counter frame
{
    // 1. Init System Core (Cepat)
    InitWindow(mScreenWidth, mScreenHeight, "Megabonk Engine v2.0 - 25 Wave Survival");
    SetTargetFPS(60);
    
    SetAudioStreamBufferSizeDefault(16384);
    InitAudioDevice();
    
    SetExitKey(KEY_NULL); 
    ShowCursor(); 
    SetMouseCursor(MOUSE_CURSOR_CROSSHAIR);

    // 2. Load UI Textures (Ringan - agar Splash Screen bisa langsung muncul)
    mSplashLogo = LoadTexture("assets/splash_logo.png");
    mLoadingBg  = LoadTexture("assets/loading_bg.png"); 
    mMenuBg     = LoadTexture("assets/menu_bg.png");

    // 3. Setup Pixel Mode
    mPixelMode = true; 
    mRenderScale = 0.4f;
    
    int virtualW = (int)(mScreenWidth * mRenderScale);
    int virtualH = (int)(mScreenHeight * mRenderScale);
    
    mTarget = LoadRenderTexture(virtualW, virtualH);
    SetTextureFilter(mTarget.texture, TEXTURE_FILTER_POINT);

    // 4. Setup Camera
    mCamera = { 0 };
    mCamera.position = (Vector3){ 0.0f, 30.0f, 20.0f };
    mCamera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    mCamera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    mCamera.fovy = 45.0f;
    mCamera.projection = CAMERA_PERSPECTIVE;

    // Aset berat (Model, Music, Shader) TIDAK DILOAD DISINI
    // Pindah ke LoadGameplayContent()
    
    std::cout << "🚀 SYSTEM START: WINDOW OPENED" << std::endl;
}
Game::~Game() {
    // 1. Bersihkan List Object Game DULU (karena mereka punya Texture/Model)
    mEnemies.clear();
    mPendingEnemies.clear();
    mGems.clear();
    mProjectileManager.Reset(); 
    
    // 2. Unload Texture UI
    UnloadTexture(mSplashLogo);
    UnloadTexture(mLoadingBg);
    UnloadTexture(mMenuBg);
    
    // 3. Unload Shader & RenderTexture
    UnloadRenderTexture(mTarget);
    UnloadShader(mGroundShader);
    UnloadShader(mSlimeShader);
    UnloadTexture(mShadowTexture);
    
    // 4. BARU TUTUP WINDOW (Ini harus paling terakhir)
    CloseAudioDevice(); // Tambahkan ini kalau pakai InitAudioDevice
    CloseWindow();
}
void Game::Run() {
    // Loop sekarang cek mGameRunning juga
    while (!WindowShouldClose() && mGameRunning) {
        float dt = GetFrameTime();
        ProcessInput(dt);
        Update(dt);
        Draw();
    }
}
void Game::ResetGame() {
    mState = GameState::PLAYING;
    mPlayer.Reset();
    mEnemies.clear();
    mPendingEnemies.clear();
    mGems.clear();
    mParticles.Reset();
    mWaveManager.Reset();
    mProjectileManager.Reset();
    mItemManager.Reset();
    mWaveBonusClaimed = false;
    mScreenShakeIntensity = 0.0f;
}
void Game::ProcessInput(float dt) {
    // -----------------------------------------------------------------------
    // 1. STATE: LOADING (Blokir semua input)
    // -----------------------------------------------------------------------
    if (mState == GameState::LOADING) return;

    // -----------------------------------------------------------------------
    // 2. STATE: MAIN MENU (Navigasi)
    // -----------------------------------------------------------------------
    if (mState == GameState::MAIN_MENU) {
        // Navigasi Bawah
        if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
            int next = (int)mCurrentMenuOption + 1;
            if (next > (int)MenuOption::EXIT) next = 0;
            mCurrentMenuOption = (MenuOption)next;
            PlaySound(mAssets.GetSound("select")); // Opsional: Bunyi klik
        }
        // Navigasi Atas
        if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
            int prev = (int)mCurrentMenuOption - 1;
            if (prev < 0) prev = (int)MenuOption::EXIT;
            mCurrentMenuOption = (MenuOption)prev;
            PlaySound(mAssets.GetSound("select"));
        }

        // Eksekusi Pilihan
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            PlaySound(mAssets.GetSound("confirm"));
            
            switch (mCurrentMenuOption) {
                case MenuOption::START:
                    ResetGame();
                    mState = GameState::PLAYING;
                    break;
                case MenuOption::SETTINGS:
                    mState = GameState::SETTINGS;
                    break;
                case MenuOption::CREDITS:
                    mState = GameState::CREDITS;
                    break;
                case MenuOption::EXIT:
                    mGameRunning = false; // Keluar dari loop Run()
                    break;
            }
        }
        return;
    }

    // -----------------------------------------------------------------------
    // 3. STATE: SUB-MENUS (Settings & Credits)
    // -----------------------------------------------------------------------
    if (mState == GameState::SETTINGS || mState == GameState::CREDITS) {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
            mState = GameState::MAIN_MENU;
        }
        return;
    }

    // -----------------------------------------------------------------------
    // 4. STATE: PLAYING (Gameplay & Cheat)
    // -----------------------------------------------------------------------
    if (mState == GameState::PLAYING) {
        // Toggle Pause
        if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) {
            mState = GameState::PAUSED;
        }

        // CHEAT CODE: Shift + L + J (Skip Wave)
        if (IsKeyDown(KEY_LEFT_SHIFT) && IsKeyDown(KEY_L) && IsKeyPressed(KEY_J)) {
            std::cout << "⏩ CHEAT ACTIVATED: SKIPPING WAVE!" << std::endl;
            mEnemies.clear();
            mPendingEnemies.clear();
            mWaveManager.ForceSkipWave();
        }
        return;
    }

    // -----------------------------------------------------------------------
    // 5. STATE: PAUSED
    // -----------------------------------------------------------------------
    if (mState == GameState::PAUSED) {
        if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) {
            mState = GameState::PLAYING; // Resume
        }
        if (IsKeyPressed(KEY_M)) {
            mState = GameState::MAIN_MENU; // Back to Menu
        }
        return;
    }

    // -----------------------------------------------------------------------
    // 6. STATE: GAME OVER & VICTORY
    // -----------------------------------------------------------------------
    if (mState == GameState::GAME_OVER || mState == GameState::VICTORY) {
        if (IsKeyPressed(KEY_R)) {
            ResetGame(); // Restart langsung
        }
        if (IsKeyPressed(KEY_M) || IsKeyPressed(KEY_ESCAPE)) {
            mState = GameState::MAIN_MENU; // Back to Menu
        }
    }
}
void Game::Update(float dt) {
    // 🎵 UPDATE MUSIC (Hanya jika aset sudah dimuat)
    if (mGameLoaded && mBgMusic != nullptr && mBgMusic->ctxData != nullptr) {
        UpdateMusicStream(*mBgMusic);
    }

    // ==============================================================================
    // 1. SPLASH SCREEN (TOTAL 5 DETIK: Fade In -> Wait -> Fade Out)
    // ==============================================================================
    if (mState == GameState::SPLASH) {
        mSplashTimer += dt;
        
        // Logika Transisi:
        // 0s - 1s: Fade In
        // 1s - 4s: Diam
        // 4s - 5s: Fade Out
        
        if (mSplashTimer > 5.0f) {
            mState = GameState::LOADING;
            mLoadingTimer = 0.0f; // Reset timer untuk loading
        }
        return;
    }

    // ==============================================================================
    // 2. LOADING SCREEN (Fade In UI -> Load Aset -> Wait Input)
    // ==============================================================================
    if (mState == GameState::LOADING) {
        
        // A. FASE BELUM LOAD (Animasi Fade In dulu, baru Load Berat)
        if (!mGameLoaded) {
            mLoadingTimer += dt; // Pakai timer ini untuk animasi Fade In teks/bar
            
            // Tunggu 0.5 detik agar UI Loading terlihat muncul pelan-pelan
            if (mLoadingTimer > 0.5f) {
                mLoadingFrameDelay++;
                
                // Tunggu 5 frame lagi untuk memastikan GPU menggambar UI ke layar
                if (mLoadingFrameDelay > 5) {
                    LoadGameplayContent(); // ⚠️ PROSES BERAT (FREEZE) DI SINI
                    mGameLoaded = true;    // Tandai selesai
                }
            }
            return; // Jangan lanjut ke logic lain saat fase ini
        }

        // B. FASE SUDAH LOAD (Menunggu Pemain Tekan Spasi)
        if (IsKeyPressed(KEY_SPACE)) {
            // Mainkan suara confirm jika ada
            if (mAssets.IsSoundReady("confirm")) PlaySound(mAssets.GetSound("confirm"));
            
            mState = GameState::MAIN_MENU;
        }
        return;
    }

    // ==============================================================================
    // 3. MAIN MENU & SUB-MENUS
    // ==============================================================================
    if (mState == GameState::MAIN_MENU || mState == GameState::SETTINGS || mState == GameState::CREDITS) {
        return; // Logic input menu ada di ProcessInput()
    }

    // ==============================================================================
    // 4. GAME STATE MANAGEMENT (Pause, GameOver, Victory)
    // ==============================================================================
    if (mState == GameState::PAUSED || mState == GameState::GAME_OVER || mState == GameState::VICTORY) {
        // Input Restart / Menu saat kondisi ini
        if (IsKeyPressed(KEY_R) && mState != GameState::PAUSED) ResetGame();
        if (IsKeyPressed(KEY_M)) mState = GameState::MAIN_MENU;
        return; 
    }

    // ==============================================================================
    // 5. 🔥 GAMEPLAY LOGIC (Hanya jalan saat State == PLAYING)
    // ==============================================================================

    // --- A. PLAYER MOVEMENT & MAP COLLISION ---
    Vector3 oldPos = mPlayer.GetPosition();
    mPlayer.Update(dt);
    Vector3 curPos = mPlayer.GetPosition();
    
    // Collision dengan Map (Retroactive Sliding)
    Model mapCol = mAssets.GetModel("ground");
    bool isColliding = false;
    float playerRadius = 0.5f;

    // Cek Tabrakan Awal
    for (int i = 0; i < mapCol.meshCount; i++) {
        BoundingBox box = GetMeshBoundingBox(mapCol.meshes[i]);
        if (box.max.y < 1.0f) continue; // Skip Lantai
        if (CheckCollisionBoxSphere(box, curPos, playerRadius)) {
            isColliding = true; break;
        }
    }

    // Jika nabrak, lakukan Sliding
    if (isColliding) {
        // Coba Slide Z
        Vector3 slideZ = { oldPos.x, curPos.y, curPos.z };
        bool hitZ = false;
        for (int i = 0; i < mapCol.meshCount; i++) {
            BoundingBox box = GetMeshBoundingBox(mapCol.meshes[i]);
            if (box.max.y < 1.0f) continue;
            if (CheckCollisionBoxSphere(box, slideZ, playerRadius)) { hitZ = true; break; }
        }

        if (!hitZ) mPlayer.SetPosition(slideZ); 
        else {
            // Coba Slide X
            Vector3 slideX = { curPos.x, curPos.y, oldPos.z };
            bool hitX = false;
            for (int i = 0; i < mapCol.meshCount; i++) {
                BoundingBox box = GetMeshBoundingBox(mapCol.meshes[i]);
                if (box.max.y < 1.0f) continue;
                if (CheckCollisionBoxSphere(box, slideX, playerRadius)) { hitX = true; break; }
            }
            if (!hitX) mPlayer.SetPosition(slideX);
            else mPlayer.SetPosition(oldPos); // Mentok
        }
    }

    // --- B. MANAGERS UPDATE ---
    mProjectileManager.Update(dt, mAssets, mParticles);
    mParticles.Update(dt);
    mItemManager.Update(dt);

    // --- C. SHOOTING & DASH INPUT ---
    // Ubah pengecekan right click jadi IsMouseButtonReleased
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
        Ray ray = GetScreenToWorldRay(GetMousePosition(), mCamera);
        float t = (0.0f - ray.position.y) / ray.direction.y;
        Vector3 targetOnGround = Vector3Add(ray.position, Vector3Scale(ray.direction, t));
        
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            mPlayer.TryShoot(targetOnGround, mProjectileManager, dt);
        }
        
        // 🔥 Trigger dash SAAT TOMBOL DILEPAS
        if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
            mPlayer.TryDash(targetOnGround);
        }
    }

    Vector3 playerPos = mPlayer.GetPosition();

    // --- D. CAMERA LOGIC ---
    // Screen Shake Decay
    if (mScreenShakeIntensity > 0) {
        mScreenShakeIntensity -= 5.0f * dt;
        if (mScreenShakeIntensity < 0) mScreenShakeIntensity = 0;
    }
    
    Vector3 shakeOffset = { 
        GetRandomFloat(-1, 1) * mScreenShakeIntensity, 
        GetRandomFloat(-1, 1) * mScreenShakeIntensity, 
        GetRandomFloat(-1, 1) * mScreenShakeIntensity 
    };

    // Camera Follow & Peek
    Vector3 mouseWorldPos = playerPos;
    Ray rayCam = GetScreenToWorldRay(GetMousePosition(), mCamera);
    if (rayCam.direction.y != 0) {
        float t = -rayCam.position.y / rayCam.direction.y;
        if (t >= 0) mouseWorldPos = Vector3Add(rayCam.position, Vector3Scale(rayCam.direction, t));
    }

    Vector3 lookOffset = Vector3Subtract(mouseWorldPos, playerPos);
    float maxPeekDist = 12.0f;
    if (Vector3Length(lookOffset) > maxPeekDist) lookOffset = Vector3Scale(Vector3Normalize(lookOffset), maxPeekDist);
    
    Vector3 targetPos = Vector3Add(playerPos, Vector3Scale(lookOffset, 0.15f));
    float smoothSpeed = 3.0f * dt; 
    mCamera.target.x = Lerp(mCamera.target.x, targetPos.x, smoothSpeed);
    mCamera.target.z = Lerp(mCamera.target.z, targetPos.z, smoothSpeed);
    mCamera.target.y = 0.0f;

    Vector3 finalTarget = Vector3Add(mCamera.target, shakeOffset);
    mCamera.position = Vector3Add(finalTarget, (Vector3){ 0.0f, 35.0f, 25.0f });
    mCamera.target = finalTarget;

    // --- E. WAVE MANAGER ---
    mWaveManager.Update(dt, mPlayer.GetLevel(), (int)mEnemies.size());

    if (mWaveManager.ShouldSpawn()) {
        EnemySpawnEntry entry = mWaveManager.GetNextSpawn();
        SpawnEnemy(entry, {0, 0, 0}); // 0,0,0 trigger random position logic di SpawnEnemy
        mWaveManager.ConsumeSpawnSignal();
    }

    // Wave Bonus & Victory Check
    if (mWaveManager.GetState() == WaveState::COMPLETED) {
        if (!mWaveBonusClaimed) {
            int bonusXP = mWaveManager.GetWaveBonusXP();
            mPlayer.AddXP(bonusXP);
            mParticles.SpawnExplosion(playerPos, GOLD, 50);
            mWaveBonusClaimed = true;
            
            if (mWaveManager.GetCurrentWave() >= 25) {
                mState = GameState::VICTORY;
                return;
            }
        }
    } else {
        mWaveBonusClaimed = false;
    }

    // --- F. ENEMY LOGIC & PLAYER COLLISION ---
    for (auto& e : mEnemies) {
        if (!e->IsActive()) continue;

        e->Update(dt, playerPos);

        // Boss Minion Spawn
        BossEnemy* boss = dynamic_cast<BossEnemy*>(e.get());
        if (boss && boss->ShouldSpawnMinion()) {
            Vector3 spawnPos = boss->GetPosition();
            spawnPos.x += GetRandomFloat(-3, 3);
            spawnPos.z += GetRandomFloat(-3, 3);
            mPendingEnemies.push_back(std::make_unique<CubeWalker>(1, spawnPos));
            boss->ConsumeSpawnSignal();
        }

        // Tabrakan Musuh ke Player
        if (Vector3Distance(playerPos, e->GetPosition()) < (e->GetRadius() + 0.5f)) {
            mPlayer.TakeDamage(20.0f * dt);
            mScreenShakeIntensity = 0.4f;

            if (mPlayer.IsDead()) {
                mState = GameState::GAME_OVER;
                mParticles.SpawnExplosion(playerPos, SKYBLUE, 50);
            }
        }

        // Exploder Logic
        ExploderEnemy* exploder = dynamic_cast<ExploderEnemy*>(e.get());
        if (exploder && exploder->ShouldExplode(playerPos)) {
            float dist = Vector3Distance(playerPos, exploder->GetPosition());
            if (dist < exploder->GetExplosionRadius()) {
                mPlayer.TakeDamage(exploder->GetExplosionDamage());
                mScreenShakeIntensity = 1.0f;
            }
            mParticles.SpawnExplosion(exploder->GetPosition(), GREEN, 80);
            exploder->TakeDamage(9999); // Mati instan
        }
    }

    // --- G. ENEMY PROJECTILE COLLISION ---
    for (auto& e : mEnemies) {
        if (!e->IsActive()) continue;

        // Shooter Bullets
        ShooterEnemy* shooter = dynamic_cast<ShooterEnemy*>(e.get());
        if (shooter) {
            auto& bullets = shooter->GetBullets();
            for (auto& b : bullets) {
                if (!b.active) continue;
                if (Vector3Distance(playerPos, b.position) < (b.radius + 0.5f)) {
                    mPlayer.TakeDamage(b.damage);
                    b.active = false;
                    mParticles.SpawnExplosion(b.position, RED, 10);
                    mScreenShakeIntensity = 0.3f;
                }
            }
        }
        // Boss Projectiles
        BossEnemy* boss = dynamic_cast<BossEnemy*>(e.get());
        if (boss) {
            auto& projectiles = boss->GetProjectiles();
            for (auto& p : projectiles) {
                if (!p.active) continue;
                if (Vector3Distance(playerPos, p.position) < (p.radius + 0.5f)) {
                    mPlayer.TakeDamage(p.damage);
                    p.active = false;
                    mParticles.SpawnExplosion(p.position, ORANGE, 15);
                    mScreenShakeIntensity = 0.5f;
                }
            }
        }
    }

    // --- H. PLAYER PROJECTILE COLLISION ---
    auto& projectiles = mProjectileManager.GetProjectiles();
    for (auto& b : projectiles) {
        if (!b.active) continue;
        
        for (auto& e : mEnemies) {
            if (!e->IsActive()) continue;

            if (CheckCollisionSpheres(b.position, b.radius, e->GetPosition(), e->GetRadius())) {
                e->TakeDamage(b.damage); 
                b.active = false;
                mParticles.SpawnExplosion(b.position, YELLOW, 5);
                
                if (mAssets.IsSoundReady("crack")) {
                    Sound& sfx = mAssets.GetSound("crack");
                    SetSoundPitch(sfx, GetRandomFloat(1.8f, 2.2f)); 
                    PlaySound(sfx);
                }

                // Enemy Death Logic
                if (!e->IsActive()) {
                    Color color = (e->GetTier() == 1) ? RED : ((e->GetTier() == 2) ? BLUE : GOLD);
                    mParticles.SpawnExplosion(e->GetPosition(), color, 20);
                    mScreenShakeIntensity = 0.3f;

                    // Spawn XP Orbs
                    int totalXP = e->GetXPReward();
                    int orbCount = GetRandomValue(3, 8);
                    int xpPerOrb = totalXP / orbCount;
                    int remainder = totalXP % orbCount;

                    for(int i = 0; i < orbCount; i++) {
                        Vector3 spawnPos = e->GetPosition();
                        spawnPos.y += 0.5f; // Muncul sedikit di atas tanah
                        
                        int orbValue = xpPerOrb + (i == 0 ? remainder : 0);
                        
                        // Beri kecepatan random (Muncrat ke atas & samping)
                        Vector3 randomVel = {
                            GetRandomFloat(-6.0f, 6.0f),  // X nyebar
                            GetRandomFloat(8.0f, 15.0f),  // Y lompat ke atas
                            GetRandomFloat(-6.0f, 6.0f)   // Z nyebar
                        };

                        mGems.push_back({spawnPos, (float)orbValue, true, randomVel});
                    }

                    // Loot Drop
                    SlimeJumper* slime = dynamic_cast<SlimeJumper*>(e.get());
                    if (slime && slime->HasLoot()) {
                        mItemManager.SpawnItem(e->GetPosition(), slime->GetLootType(), slime->GetWeaponDropTier());
                    }
                    
                    // Split Logic
                    if (e->CanSplit()) {
                        int childrenCount = GetRandomValue(2, 3);
                        for(int i = 0; i < childrenCount; i++) {
                            Vector3 offset = { GetRandomFloat(-1,1), 0, GetRandomFloat(-1,1) };
                            mPendingEnemies.push_back(std::make_unique<CubeWalker>(1, Vector3Add(e->GetPosition(), offset)));
                        }
                    }
                }
                break; 
            }
        }
    }

    // --- I. XP GEM PHYSICS & MAGNET ---
    float magnetRadius = mPlayer.HasMagnetBuff() ? 10.0f : 5.0f;
    
    for (auto& g : mGems) {
        if (!g.active) continue;

        // 1. FISIKA: Gravitasi & Pergerakan (Muncrat)
        if (g.position.y > 0.2f || g.velocity.y > 0) {
            g.velocity.y -= 30.0f * dt; // Tarikan Gravitasi
            g.position.x += g.velocity.x * dt;
            g.position.y += g.velocity.y * dt;
            g.position.z += g.velocity.z * dt;
        }

        // 2. FISIKA: Sentuh Tanah Langsung Berhenti
        float groundLevel = 0.2f; 
        if (g.position.y <= groundLevel) {
            g.position.y = groundLevel;
            g.velocity = {0, 0, 0}; // Langsung diam 100%
        }

        // 3. LOGIKA MAGNET
        float dist = Vector3Distance(playerPos, g.position);
        if (dist < magnetRadius) { 
            Vector3 dir = Vector3Normalize(Vector3Subtract(playerPos, g.position));
            g.position = Vector3Add(g.position, Vector3Scale(dir, 15.0f * dt));
            g.velocity = {0,0,0}; 
        }

        // 4. DIAMBIL PLAYER
        if (dist < 1.0f) {
            g.active = false;
            mPlayer.AddXP(g.value);
        }
    }

    // --- J. ITEM PICKUP ---
    int weaponTier = -1;
    ItemType picked = mItemManager.CheckPickup(playerPos, 1.5f, weaponTier);

    if (picked == ItemType::MAGNET) {
        mPlayer.ActivateMagnetBuff(10.0f);
        mParticles.SpawnExplosion(playerPos, BLUE, 30);
    }
    else if (picked == ItemType::HEALTH_PACK) {
        mPlayer.Heal(50.0f);
        mParticles.SpawnExplosion(playerPos, RED, 20);
    }
    else if (picked == ItemType::WEAPON_DROP) {
        mPlayer.SwitchWeapon((WeaponType)weaponTier);
        mParticles.SpawnExplosion(playerPos, YELLOW, 40);
    }

    // --- K. CLEANUP & PENDING ---
    mEnemies.erase(
        std::remove_if(mEnemies.begin(), mEnemies.end(), 
            [](const std::unique_ptr<BaseEnemy>& e){ return !e->IsActive(); }
        ), 
        mEnemies.end()
    );
    
    mGems.erase(
        std::remove_if(mGems.begin(), mGems.end(), 
            [](const XPGem& g){ return !g.active; }
        ), 
        mGems.end()
    );

    for (auto& pending : mPendingEnemies) {
        mEnemies.push_back(std::move(pending));
    }
    mPendingEnemies.clear();
}
void Game::LoadGameplayContent() {
    std::cout << "📦 LOADING HEAVY ASSETS (MODELS, MUSIC, SHADERS)..." << std::endl;

    // 1. Load Semua Model & Sound
    mAssets.LoadAll();

    // 2. Setup Shaders
    mGroundShader = LoadShaderFromMemory(VS_CODE, FS_GROUND_CODE);
    mSlimeShader = LoadShaderFromMemory(VS_CODE, FS_SLIME_CODE);

    mLightPosGroundLoc = GetShaderLocation(mGroundShader, "lightPos");
    mLightPosSlimeLoc = GetShaderLocation(mSlimeShader, "lightPos");
    mViewPosSlimeLoc = GetShaderLocation(mSlimeShader, "viewPos");

    Vector3 lightPos = { 100.0f, 100.0f, 50.0f };
    SetShaderValue(mGroundShader, mLightPosGroundLoc, &lightPos, SHADER_UNIFORM_VEC3);
    SetShaderValue(mSlimeShader, mLightPosSlimeLoc, &lightPos, SHADER_UNIFORM_VEC3);

    // 3. Setup Materials
    if (mAssets.GetModel("ground").meshCount > 0) {
        mAssets.GetModel("ground").materials[0].shader = mGroundShader;
        mAssets.GetModel("ground").materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = mAssets.GetTexture("ground");
    }
    if (mAssets.GetModel("slime").meshCount > 0) {
        mAssets.GetModel("slime").materials[0].shader = mSlimeShader;
    }
    if (mAssets.GetModel("cube").meshCount > 0) {
        mAssets.GetModel("cube").materials[0].shader = mGroundShader;
    }
    if (mAssets.GetModel("soto").meshCount > 0) {
        mAssets.GetModel("soto").materials[0].shader = mGroundShader;
    }

    // 4. Shadow System
    mShadowTexture = GenerateShadowTexture(); 
    if (mAssets.GetModel("shadow_plane").meshCount > 0) {
        mAssets.GetModel("shadow_plane").materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = mShadowTexture;
    }

    // 5. Setup Music
    mBgMusic = &mAssets.GetMusic("bgm");
    if (mBgMusic->ctxData != nullptr) {
        SetMusicVolume(*mBgMusic, 0.5f);
        PlayMusicStream(*mBgMusic);
    }
    
    std::cout << "✅ ASSETS LOADED COMPLETELY!" << std::endl;
}
void Game::Draw() {
    // ==============================================================================
    // PHASE 1: 3D WORLD RENDER (Hanya saat Gameplay/Pause/Result)
    // ==============================================================================
    bool isGameplayActive = (mState == GameState::PLAYING || mState == GameState::PAUSED || 
                             mState == GameState::GAME_OVER || mState == GameState::VICTORY);

    if (isGameplayActive) {
        if (mPixelMode) BeginTextureMode(mTarget);
        else BeginDrawing();

            ClearBackground((Color){ 20, 20, 25, 255 }); // Dark Blue-ish Gray

            BeginMode3D(mCamera);
                
                // 1. Ground
                if (mAssets.GetModel("ground").meshCount > 0) {
                    DrawModelEx(mAssets.GetModel("ground"), (Vector3){0, -0.05f, 0}, 
                               (Vector3){0,1,0}, 0.0f, (Vector3){1.0f, 1.0f, 1.0f}, WHITE);
                } else {
                    DrawGrid(100, 1.0f);
                }

                // 2. Player (Selalu gambar kecuali loading)
                mPlayer.Draw(mAssets.GetModel("soto"), mCamera, mShadowTexture);

                // 3. Update Shader Uniforms (Lighting Position)
                SetShaderValue(mSlimeShader, mViewPosSlimeLoc, &mCamera.position, SHADER_UNIFORM_VEC3);
                Vector3 playerPos = mPlayer.GetPosition();

                // 4. Enemies
                for (auto& e : mEnemies) {
                    if (!e->IsActive()) continue;
                    e->Draw(
                        mAssets.GetModel("slime"),    
                        mAssets.GetModel("cube"),     
                        mAssets.GetModel("magnet"),   
                        mAssets.GetModel("shadow_plane"), 
                        mCamera,
                        playerPos
                    );
                }

                // 5. Projectiles, Particles, Items
                mProjectileManager.Draw(); 
                mParticles.Draw();
                mItemManager.Draw(mAssets.GetModel("magnet"));

                // 6. XP Gems (Floating Cubes with Glow)
                BeginBlendMode(BLEND_ADDITIVE);
                for (const auto& g : mGems) {
                    if (!g.active) continue;
                    float time = GetTime();
                    
                    // Logic warna gem berdasarkan posisi sinyal
                    Color xpColor = (sinf(time * 3.0f + g.position.x) > 0) ? YELLOW : GREEN;

                    rlPushMatrix();
                        float bob = sinf(time * 8.0f + g.position.x) * 0.15f;
                        rlTranslatef(g.position.x, g.position.y + 0.3f + bob, g.position.z);
                        rlRotatef(time * 150.0f, 0, 1, 0);
                        rlRotatef(45.0f, 1, 0, 0);
                        
                        float size = 0.2f + (g.value * 0.005f); 
                        if (size > 0.4f) size = 0.4f;

                        DrawCube((Vector3){0,0,0}, size, size, size, xpColor);
                        DrawCubeWires((Vector3){0,0,0}, size, size, size, WHITE);
                    rlPopMatrix();
                }
                EndBlendMode();
                
            EndMode3D();

        if (mPixelMode) EndTextureMode();
    }

    // ==============================================================================
    // PHASE 2: UI & 2D OVERLAY
    // ==============================================================================
    BeginDrawing();
    ClearBackground(BLACK); // Dasar Hitam Penting untuk Fade Out Splash

    // A. Gambar Hasil 3D (Jika ada)
    if (mPixelMode && isGameplayActive) {
        // Source Rect (Flip Y karena OpenGL coordinates)
        Rectangle srcRect = { 0.0f, 0.0f, (float)mTarget.texture.width, -(float)mTarget.texture.height };
        Rectangle destRect = { 0.0f, 0.0f, (float)mScreenWidth, (float)mScreenHeight };
        DrawTexturePro(mTarget.texture, srcRect, destRect, (Vector2){0,0}, 0.0f, WHITE);
    }

    // B. LOGIC UI PER STATE

    // --------------------------------------------------------------------------
    // 1. SPLASH SCREEN (Cinematic Fade In/Out)
    // --------------------------------------------------------------------------
    if (mState == GameState::SPLASH) {
        float alpha = 0.0f;

        // 0s - 1s: FADE IN (0 -> 1)
        if (mSplashTimer < 1.0f) {
            alpha = mSplashTimer; 
        }
        // 1s - 4s: WAIT (Tetap 1)
        else if (mSplashTimer < 4.0f) {
            alpha = 1.0f;
        }
        // 4s - 5s: FADE OUT (1 -> 0)
        else {
            alpha = 1.0f - (mSplashTimer - 4.0f);
        }

        // Clamp Alpha
        if (alpha < 0.0f) alpha = 0.0f;
        if (alpha > 1.0f) alpha = 1.0f;

        Color tint = { 255, 255, 255, (unsigned char)(alpha * 255) };

        // Draw Full Screen (Anti Zoom)
        Rectangle source = { 0, 0, (float)mSplashLogo.width, (float)mSplashLogo.height };
        Rectangle dest = { 0, 0, (float)mScreenWidth, (float)mScreenHeight };
        DrawTexturePro(mSplashLogo, source, dest, (Vector2){0,0}, 0.0f, tint);
        
        // Teks Studio (Ikut Fade)
        const char* studio = "MEGABONK STUDIOS";
        int textW = MeasureText(studio, 20);
        DrawText(studio, mScreenWidth/2 - textW/2, mScreenHeight - 50, 20, (Color){100, 100, 100, (unsigned char)(alpha * 255)});
    }

    // --------------------------------------------------------------------------
    // 2. LOADING SCREEN
    // --------------------------------------------------------------------------
    else if (mState == GameState::LOADING) {
        float alpha = 1.0f;
        
        // FADE IN UI (0s - 0.5s)
        if (!mGameLoaded && mLoadingTimer < 0.5f) {
            alpha = mLoadingTimer / 0.5f;
            if (alpha > 1.0f) alpha = 1.0f;
        }

        Color tint = { 255, 255, 255, (unsigned char)(alpha * 255) };

        // Background Loading
        Rectangle source = { 0, 0, (float)mLoadingBg.width, (float)mLoadingBg.height };
        Rectangle dest = { 0, 0, (float)mScreenWidth, (float)mScreenHeight };
        DrawTexturePro(mLoadingBg, source, dest, (Vector2){0,0}, 0.0f, tint);

        // Setup Bar Position
        int barW = mScreenWidth - 100;
        int barH = 20;
        int barX = 50;
        int barY = mScreenHeight - 60;

        // Draw Frame Luar
        Color frameColor = { 200, 200, 200, (unsigned char)(alpha * 255) };
        DrawRectangleLines(barX, barY, barW, barH, frameColor);

        if (mGameLoaded) {
            // Loading Selesai -> Bar Penuh Putih
            DrawRectangle(barX + 2, barY + 2, barW - 4, barH - 4, WHITE);
            
            // Teks Kedip "PRESS SPACE"
            if ((int)(GetTime() * 2) % 2 == 0) {
                const char* txt = "PRESS [SPACE] TO CONTINUE";
                DrawText(txt, mScreenWidth/2 - MeasureText(txt, 20)/2, barY - 30, 20, YELLOW);
            }
        } else {
            // Masih Loading -> Bar Setengah (Pudar)
            DrawRectangle(barX + 2, barY + 2, (barW - 4) / 2, barH - 4, frameColor);
            DrawText("LOADING ASSETS...", barX, barY - 30, 20, frameColor);
        }
    }

    // --------------------------------------------------------------------------
    // 3. MAIN MENU (Posisi Kanan)
    // --------------------------------------------------------------------------
    else if (mState == GameState::MAIN_MENU) {
        // Background Menu
        Rectangle source = { 0, 0, (float)mMenuBg.width, (float)mMenuBg.height };
        Rectangle dest = { 0, 0, (float)mScreenWidth, (float)mScreenHeight };
        DrawTexturePro(mMenuBg, source, dest, (Vector2){0,0}, 0.0f, WHITE);

        // Center Point untuk Menu (75% ke kanan)
        int menuCenterX = (int)(mScreenWidth * 0.75f); 

        // Judul Game
        const char* title = "MEGABONK SURVIVAL";
        int titleW = MeasureText(title, 40);
        
        // Shadow & Text
        DrawText(title, menuCenterX - titleW/2 + 3, 103, 40, BLACK); 
        DrawText(title, menuCenterX - titleW/2, 100, 40, GOLD);

        // Menu Options
        const char* options[] = { "START GAME", "SETTINGS", "CREDITS", "EXIT" };
        int startY = mScreenHeight / 2 - 50;
        int spacing = 60;

        for (int i = 0; i < 4; i++) {
            bool isSelected = (i == (int)mCurrentMenuOption);
            Color color = isSelected ? YELLOW : RAYWHITE;
            int fontSize = isSelected ? 35 : 25; // Zoom effect selection

            int textW = MeasureText(options[i], fontSize);
            int posX = menuCenterX - textW/2;
            int posY = startY + (i * spacing);

            DrawText(options[i], posX + 2, posY + 2, fontSize, BLACK); // Shadow
            DrawText(options[i], posX, posY, fontSize, color);

            // Panah Seleksi
            if (isSelected) {
                float time = GetTime() * 10.0f;
                int offset = (int)(sinf(time) * 5.0f); // Animasi gerak
                DrawText(">", posX - 30 + offset, posY, fontSize, YELLOW);
                DrawText("<", posX + textW + 15 - offset, posY, fontSize, YELLOW);
            }
        }
        
        DrawText("v2.0 - Stable Build", mScreenWidth - 150, mScreenHeight - 20, 10, GRAY);
    }

    // --------------------------------------------------------------------------
    // 4. SETTINGS & CREDITS (Overlay)
    // --------------------------------------------------------------------------
    else if (mState == GameState::SETTINGS) {
        DrawRectangle(0, 0, mScreenWidth, mScreenHeight, (Color){0, 0, 0, 240});
        DrawText("SETTINGS", 50, 50, 40, GOLD);
        DrawText("Music Volume: 100%", 100, 150, 30, WHITE);
        DrawText("SFX Volume: 100%", 100, 200, 30, WHITE);
        DrawText("[ESC] BACK", 50, mScreenHeight - 50, 20, GRAY);
    }
    else if (mState == GameState::CREDITS) {
        DrawRectangle(0, 0, mScreenWidth, mScreenHeight, (Color){0, 0, 0, 240});
        DrawText("CREDITS", 50, 50, 40, GOLD);
        DrawText("Created by: Jesril Pratama", 100, 150, 30, WHITE);
        DrawText("Engine: Raylib C++", 100, 200, 30, WHITE);
        DrawText("[ESC] BACK", 50, mScreenHeight - 50, 20, GRAY);
    }

    // --------------------------------------------------------------------------
    // 5. GAMEPLAY HUD (Playing / Paused / Game Over)
    // --------------------------------------------------------------------------
    else if (mState == GameState::PLAYING) {
        mUI.DrawHUD(mPlayer, mWaveManager, (int)mEnemies.size(), mScreenWidth, mScreenHeight);
    }
    else if (mState == GameState::PAUSED) {
        mUI.DrawHUD(mPlayer, mWaveManager, (int)mEnemies.size(), mScreenWidth, mScreenHeight);
        mUI.DrawPause(mScreenWidth, mScreenHeight);
    }
    else if (mState == GameState::GAME_OVER) {
        mUI.DrawGameOver(mScreenWidth, mScreenHeight, mWaveManager.GetCurrentWave(), mPlayer.GetLevel());
    }
    else if (mState == GameState::VICTORY) {
        mUI.DrawVictory(mScreenWidth, mScreenHeight, mPlayer.GetLevel());
    }

    EndDrawing();
}

void Game::SpawnEnemy(EnemySpawnEntry entry, Vector3 pos) {
    if (pos.x == 0 && pos.z == 0) {
        if (entry.type == EnemySpawnType::BOSS) {
            pos = {0, 0, 0};
            mParticles.SpawnExplosion(pos, RED, 150);
            mScreenShakeIntensity = 2.0f;
        } else {
            float angle = GetRandomFloat(0, 360) * DEG2RAD;
            float dist = GetRandomFloat(30, 50);
            Vector3 playerPos = mPlayer.GetPosition();
            pos = Vector3Add(playerPos, { cosf(angle) * dist, 0, sinf(angle) * dist });
        }
    }

    switch (entry.type) {
        case EnemySpawnType::CUBE_WALKER:
            mEnemies.push_back(std::make_unique<CubeWalker>(entry.tier, pos));
            break;
            
        case EnemySpawnType::SHOOTER:
            mEnemies.push_back(std::make_unique<ShooterEnemy>(entry.tier, pos));
            break;
            
        case EnemySpawnType::CHARGER:
            mEnemies.push_back(std::make_unique<ChargerEnemy>(entry.tier, pos));
            break;
            
        case EnemySpawnType::EXPLODER:
            mEnemies.push_back(std::make_unique<ExploderEnemy>(entry.tier, pos));
            break;
            
        case EnemySpawnType::SLIME_JUMPER:
            mEnemies.push_back(std::make_unique<SlimeJumper>(entry.tier, pos));
            break;
            
        case EnemySpawnType::BOSS:
            SpawnBoss(mWaveManager.GetCurrentWave(), pos);
            break;
            
        case EnemySpawnType::MINI_BOSS:
            mEnemies.push_back(std::make_unique<CubeWalker>(3, pos));
            break;
    }
}
void Game::SpawnBoss(int waveNumber, Vector3 pos) {
    BossType bossType;
    
    if (waveNumber == 5) {
        bossType = BossType::TANK_BOSS;
    } else if (waveNumber == 10) {
        bossType = BossType::SUMMONER_BOSS;
    } else if (waveNumber == 15) {
        bossType = BossType::ARTILLERY_BOSS;
    } else if (waveNumber == 20) {
        bossType = BossType::TELEPORTER_BOSS;
    } else {
        bossType = BossType::ULTIMATE_BOSS;
    }
    
    mEnemies.push_back(std::make_unique<BossEnemy>(bossType, pos, waveNumber));
}
Texture2D Game::GenerateShadowTexture() {
    Image img = GenImageGradientRadial(64, 64, 0.5f, (Color){0, 0, 0, 200}, (Color){0, 0, 0, 0});
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}