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
    , mState(GameState::PLAYING)
    , mScreenShakeIntensity(0.0f)
    , mShootTimer(0.0f)
    , mWaveBonusClaimed(false)
{
    InitWindow(mScreenWidth, mScreenHeight, "Megabonk Engine v2.0 - 25 Wave Survival");
    SetTargetFPS(60);

    SetAudioStreamBufferSizeDefault(16384);
    InitAudioDevice();

    ShowCursor(); 
    SetMouseCursor(MOUSE_CURSOR_CROSSHAIR);

    // 🔥 SETUP PIXEL MODE
    mPixelMode = true; 
    mRenderScale = 0.4f;
    
    int virtualW = (int)(mScreenWidth * mRenderScale);
    int virtualH = (int)(mScreenHeight * mRenderScale);
    
    mTarget = LoadRenderTexture(virtualW, virtualH);
    SetTextureFilter(mTarget.texture, TEXTURE_FILTER_POINT);

    // 1. Initialize Camera
    mCamera = { 0 };
    mCamera.position = (Vector3){ 0.0f, 30.0f, 20.0f };
    mCamera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    mCamera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    mCamera.fovy = 45.0f;
    mCamera.projection = CAMERA_PERSPECTIVE;

    // 2. Load Assets
    mAssets.LoadAll();

    // 3. Setup Shaders
    mGroundShader = LoadShaderFromMemory(VS_CODE, FS_GROUND_CODE);
    mSlimeShader = LoadShaderFromMemory(VS_CODE, FS_SLIME_CODE);

    mLightPosGroundLoc = GetShaderLocation(mGroundShader, "lightPos");
    mLightPosSlimeLoc = GetShaderLocation(mSlimeShader, "lightPos");
    mViewPosSlimeLoc = GetShaderLocation(mSlimeShader, "viewPos");

    Vector3 lightPos = { 100.0f, 100.0f, 50.0f };
    SetShaderValue(mGroundShader, mLightPosGroundLoc, &lightPos, SHADER_UNIFORM_VEC3);
    SetShaderValue(mSlimeShader, mLightPosSlimeLoc, &lightPos, SHADER_UNIFORM_VEC3);

    // --- SETUP MATERIALS ---
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

    // 4. SHADOW SYSTEM SETUP
    mShadowTexture = GenerateShadowTexture(); 
    if (mAssets.GetModel("shadow_plane").meshCount > 0) {
        mAssets.GetModel("shadow_plane").materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = mShadowTexture;
    }

    // 5. SETUP BACKGROUND MUSIC
    mBgMusic = &mAssets.GetMusic("bgm");
    
    if (mBgMusic->ctxData != nullptr) {
        SetMusicVolume(*mBgMusic, 0.5f);
        PlayMusicStream(*mBgMusic);
    }

    if (mBgMusic->ctxData != nullptr) {
        std::cout << "✅ MUSIK BERHASIL DILOAD!" << std::endl;
    } else {
        std::cout << "❌ MUSIK GAGAL DILOAD!" << std::endl;
    }

    std::cout << "🎮 25 WAVE SURVIVAL MODE" << std::endl;
    std::cout << "Boss every 5 waves!" << std::endl;
}

Game::~Game() {
    UnloadRenderTexture(mTarget);
    UnloadShader(mGroundShader);
    UnloadShader(mSlimeShader);
    UnloadTexture(mShadowTexture);
    SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    mEnemies.clear();
    mPendingEnemies.clear();
    mParticles.Reset();
    CloseWindow();
}

void Game::Run() {
    while (!WindowShouldClose()) {
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

    if (IsKeyDown(KEY_LEFT_SHIFT) && IsKeyDown(KEY_L) && IsKeyPressed(KEY_J)) {
        
        std::cout << "⏩ CHEAT ACTIVATED: SKIPPING WAVE " << mWaveManager.GetCurrentWave() << "!" << std::endl;

        // 1. Matikan semua musuh yang ada di layar (Insta-kill)
        mEnemies.clear();
        mPendingEnemies.clear(); // Hapus antrian spawn boss/minion
        
        // 2. Hapus semua peluru musuh (biar aman)
        // (Opsional, tapi bagus biar gak mati pas skip)
        // Anda mungkin perlu nambahin fungsi Clear() di mProjectileManager kalau mau perfect,
        // tapi mEnemies.clear() biasanya cukup karena musuh hilang.

        // 3. Trigger logic wave selesai
        mWaveManager.ForceSkipWave();

        // 4. (Opsional) Heal player full biar testing makin cepet
        // mPlayer.Heal(9999); 

    }

    if (IsKeyPressed(KEY_P)) {
        if (mState == GameState::PLAYING) {
            mState = GameState::PAUSED;
            // Opsional: Pause music kalau mau hening
            // PauseMusicStream(*mBgMusic); 
        } 
        else if (mState == GameState::PAUSED) {
            mState = GameState::PLAYING;
            // ResumeMusicStream(*mBgMusic);
        }
    }

    // Kalau sedang pause atau game over, input player (tembak/jalan) tidak diproses
    if (mState != GameState::PLAYING) return;


    if (mState == GameState::GAME_OVER || mState == GameState::VICTORY) return;
}

void Game::Update(float dt) {

    // 🔥 UPDATE MUSIC STREAM
    if (mBgMusic != nullptr && mBgMusic->ctxData != nullptr) {
        UpdateMusicStream(*mBgMusic);
    }

    if (mState == GameState::PAUSED || mState == GameState::GAME_OVER || mState == GameState::VICTORY) {
        if (IsKeyPressed(KEY_R) && mState != GameState::PAUSED) ResetGame(); // Restart logic
        return; 
    }

    // --- 1. Entity Updates ---
    mPlayer.Update(dt);
    mProjectileManager.Update(dt, mAssets, mParticles);
    mParticles.Update(dt);
    mItemManager.Update(dt);

    // --- 2. Shooting Logic ---
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        Ray ray = GetScreenToWorldRay(GetMousePosition(), mCamera);
        float t = (0.0f - ray.position.y) / ray.direction.y;
        Vector3 targetOnGround = Vector3Add(ray.position, Vector3Scale(ray.direction, t));
        mPlayer.TryShoot(targetOnGround, mProjectileManager, dt);
    }

    Vector3 playerPos = mPlayer.GetPosition();

    // --- 3. CAMERA LOGIC ---

    // A. Shake Logic (Decay)
    if (mScreenShakeIntensity > 0) {
        mScreenShakeIntensity -= 5.0f * dt;
        if (mScreenShakeIntensity < 0) mScreenShakeIntensity = 0;
    }
    
    Vector3 shakeOffset = { 
        GetRandomFloat(-1, 1) * mScreenShakeIntensity, 
        GetRandomFloat(-1, 1) * mScreenShakeIntensity, 
        GetRandomFloat(-1, 1) * mScreenShakeIntensity 
    };

    // B. Mouse World Position
    Vector3 mouseWorldPos = playerPos;
    Ray rayCam = GetScreenToWorldRay(GetMousePosition(), mCamera);
    
    if (rayCam.direction.y != 0) {
        float t = -rayCam.position.y / rayCam.direction.y;
        if (t >= 0) {
            mouseWorldPos = Vector3Add(rayCam.position, Vector3Scale(rayCam.direction, t));
        }
    }

    // C. Lazy Peek
    Vector3 lookOffset = Vector3Subtract(mouseWorldPos, playerPos);
    float maxPeekDist = 12.0f;
    if (Vector3Length(lookOffset) > maxPeekDist) {
        lookOffset = Vector3Scale(Vector3Normalize(lookOffset), maxPeekDist);
    }
    
    Vector3 targetPos = Vector3Add(playerPos, Vector3Scale(lookOffset, 0.15f));

    // D. Map Boundaries Clamping
    float mapHalfSize = 50.0f; 
    float viewMargin = 18.0f;
    float minLimit = -mapHalfSize + viewMargin;
    float maxLimit = mapHalfSize - viewMargin;

    targetPos.x = Clamp(targetPos.x, minLimit, maxLimit);
    targetPos.z = Clamp(targetPos.z, minLimit, maxLimit);

    // E. Smooth Movement
    float smoothSpeed = 3.0f * dt; 
    mCamera.target.x = Lerp(mCamera.target.x, targetPos.x, smoothSpeed);
    mCamera.target.z = Lerp(mCamera.target.z, targetPos.z, smoothSpeed);
    mCamera.target.y = 0.0f;

    // F. Final Position
    Vector3 finalTarget = Vector3Add(mCamera.target, shakeOffset);
    mCamera.position = Vector3Add(finalTarget, (Vector3){ 0.0f, 35.0f, 25.0f });
    mCamera.target = finalTarget;

    // --- 4. 🔥 WAVE MANAGER ---
    mWaveManager.Update(dt, mPlayer.GetLevel(), (int)mEnemies.size());

    // ✅ SPAWN LOGIC
    if (mWaveManager.ShouldSpawn()) {
        EnemySpawnEntry entry = mWaveManager.GetNextSpawn();
        SpawnEnemy(entry, {0, 0, 0});
        mWaveManager.ConsumeSpawnSignal();
    }

    // ✅ WAVE BONUS XP
    if (mWaveManager.GetState() == WaveState::COMPLETED) {
        if (!mWaveBonusClaimed) {
            int bonusXP = mWaveManager.GetWaveBonusXP();
            mPlayer.AddXP(bonusXP);
            mParticles.SpawnExplosion(playerPos, GOLD, 50);
            mWaveBonusClaimed = true;
            
            // ✅ CHECK VICTORY
            if (mWaveManager.GetCurrentWave() >= 25) {
                mState = GameState::VICTORY;
                return;
            }
        }
    } else {
        mWaveBonusClaimed = false;
    }

    // --- 5. Enemy Logic & Player Collision ---
    for (auto& e : mEnemies) {
        if (!e->IsActive()) continue;

        e->Update(dt, playerPos);

        // ✅ BOSS MINION SPAWN
        BossEnemy* boss = dynamic_cast<BossEnemy*>(e.get());
        if (boss && boss->ShouldSpawnMinion()) {
            Vector3 spawnPos = boss->GetPosition();
            spawnPos.x += GetRandomFloat(-3, 3);
            spawnPos.z += GetRandomFloat(-3, 3);
            
            mPendingEnemies.push_back(std::make_unique<CubeWalker>(1, spawnPos));
            boss->ConsumeSpawnSignal();
        }

        // Player collision
        if (Vector3Distance(playerPos, e->GetPosition()) < (e->GetRadius() + 0.5f)) {
            mPlayer.TakeDamage(20.0f * dt);
            mScreenShakeIntensity = 0.4f;

            if (mPlayer.IsDead()) {
                mState = GameState::GAME_OVER;
                mParticles.SpawnExplosion(playerPos, SKYBLUE, 50);
            }
        }

        // ✅ EXPLODER CHECK
        ExploderEnemy* exploder = dynamic_cast<ExploderEnemy*>(e.get());
        if (exploder && exploder->ShouldExplode(playerPos)) {
            float dist = Vector3Distance(playerPos, exploder->GetPosition());
            if (dist < exploder->GetExplosionRadius()) {
                mPlayer.TakeDamage(exploder->GetExplosionDamage());
                mScreenShakeIntensity = 1.0f;
            }
            
            mParticles.SpawnExplosion(exploder->GetPosition(), GREEN, 80);
            exploder->TakeDamage(9999);
        }
    }

    // --- 6. 🔥 ENEMY BULLET COLLISION ---
    for (auto& e : mEnemies) {
        if (!e->IsActive()) continue;

        // Check Shooter bullets
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

        // Check Boss projectiles
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

    // --- 7. Player Projectile Collision ---
    auto& projectiles = mProjectileManager.GetProjectiles();
    for (auto& b : projectiles) {
        if (!b.active) continue;
        
        for (auto& e : mEnemies) {
            if (!e->IsActive()) continue;

            if (CheckCollisionSpheres(b.position, b.radius, e->GetPosition(), e->GetRadius())) {
                e->TakeDamage(b.damage); 
                b.active = false;
                mParticles.SpawnExplosion(b.position, YELLOW, 5);
                
                Sound& sfx = mAssets.GetSound("egg_break");
                SetSoundPitch(sfx, GetRandomFloat(1.8f, 2.2f)); 
                PlaySound(sfx);

                // --- ENEMY DEATH LOGIC ---
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
                        Vector3 offset = {
                            GetRandomFloat(-0.8f, 0.8f),
                            GetRandomFloat(0.2f, 0.5f),
                            GetRandomFloat(-0.8f, 0.8f)
                        };
                        Vector3 orbPos = Vector3Add(e->GetPosition(), offset);
                        int orbValue = xpPerOrb + (i == 0 ? remainder : 0);
                        mGems.push_back({orbPos, (float)orbValue, true});
                    }

                    // ✅ LOOT DROP
                    SlimeJumper* slime = dynamic_cast<SlimeJumper*>(e.get());
                    if (slime && slime->HasLoot()) {
                        ItemType lootType = slime->GetLootType();
                        int weaponTier = slime->GetWeaponDropTier();
                        mItemManager.SpawnItem(e->GetPosition(), lootType, weaponTier);
                    }
                    
                    // Splitting logic
                    if (e->CanSplit()) {
                        int childrenCount = GetRandomValue(2, 3);
                        for(int i = 0; i < childrenCount; i++) {
                            Vector3 offset = { GetRandomFloat(-1,1), 0, GetRandomFloat(-1,1) };
                            Vector3 spawnPos = Vector3Add(e->GetPosition(), offset);
                            mPendingEnemies.push_back(std::make_unique<CubeWalker>(1, spawnPos));
                        }
                    }
                }
                break; 
            }
        }
    }

    // --- 8. XP Gem Logic ---
    float magnetRadius = mPlayer.HasMagnetBuff() ? 10.0f : 5.0f;
    
    for (auto& g : mGems) {
        if (!g.active) continue;
        
        float dist = Vector3Distance(playerPos, g.position);
        
        if (dist < magnetRadius) { 
            Vector3 dir = Vector3Normalize(Vector3Subtract(playerPos, g.position));
            g.position = Vector3Add(g.position, Vector3Scale(dir, 15.0f * dt));
        }
        
        if (dist < 1.0f) {
            g.active = false;
            mPlayer.AddXP(g.value);
        }
    }

    // --- 9. ✅ ITEM PICKUP LOGIC ---
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
        WeaponType newWeapon = (WeaponType)weaponTier;
        mPlayer.SwitchWeapon(newWeapon);
        mParticles.SpawnExplosion(playerPos, YELLOW, 40);
    }

    // --- 10. Cleanup ---
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

    // Transfer pending enemies
    for (auto& pending : mPendingEnemies) {
        mEnemies.push_back(std::move(pending));
    }
    mPendingEnemies.clear();
}

void Game::Draw() {
    // -----------------------------------------------------------
    // PHASE 1: RENDER 3D WORLD (Ke Texture atau Layar)
    // -----------------------------------------------------------
    
    if (mPixelMode) {
        BeginTextureMode(mTarget);
    } else {
        BeginDrawing();
    }

        ClearBackground((Color){ 20, 20, 25, 255 }); // Dark Blue-ish Gray background

        BeginMode3D(mCamera);
            
            // 1. Ground Render
            if (mAssets.GetModel("ground").meshCount > 0) {
                DrawModelEx(mAssets.GetModel("ground"), (Vector3){0, -0.05f, 0}, 
                           (Vector3){0,1,0}, 0.0f, (Vector3){1.0f, 1.0f, 1.0f}, WHITE);
            } else {
                DrawGrid(100, 1.0f);
            }

            // 2. Player Render
            // Player digambar kalau game state playing atau victory (biar pas menang gak ilang)
            if (mState == GameState::PLAYING || mState == GameState::VICTORY) {
                mPlayer.Draw(mAssets.GetModel("soto"), mCamera, mShadowTexture);
            }

            // 3. Update Shader Uniforms (Lighting)
            // Update posisi cahaya/kamera untuk shader slime & ground
            SetShaderValue(mSlimeShader, mViewPosSlimeLoc, &mCamera.position, SHADER_UNIFORM_VEC3);
            Vector3 playerPos = mPlayer.GetPosition();

            // 4. Enemies Render
            // Kita pass semua model aset ke fungsi Draw musuh
            for (auto& e : mEnemies) {
                if (!e->IsActive()) continue;

                e->Draw(
                    mAssets.GetModel("slime"),    // Untuk SlimeJumper
                    mAssets.GetModel("cube"),     // Untuk CubeWalker, Shooter, Charger
                    mAssets.GetModel("magnet"),   // Untuk Slime Variant Magnet
                    mAssets.GetModel("shadow_plane"), // Bayangan
                    mCamera,
                    playerPos
                );
            }

            // 5. Projectiles (Peluru Player & Musuh)
            mProjectileManager.Draw(); 

            // 6. Particle Effects
            mParticles.Draw();

            // 7. Items (Health packs, Magnets, Weapons)
            mItemManager.Draw(mAssets.GetModel("magnet"));

            // 8. XP Gems (Floating Cubes)
            // Pakai Additive Blending biar kelihatan "bersinar"
            BeginBlendMode(BLEND_ADDITIVE);
            for (const auto& g : mGems) {
                if (!g.active) continue;

                float time = GetTime();
                float hue = sinf(time * 3.0f + g.position.x);
                Color xpColor = LIME;

                if (hue > 0.5f) xpColor = YELLOW;
                else if (hue < -0.5f) xpColor = GREEN;

                // Matrix transformation manual untuk animasi spin
                rlPushMatrix();
                    float bob = sinf(time * 8.0f + g.position.x) * 0.15f;
                    rlTranslatef(g.position.x, g.position.y + 0.3f + bob, g.position.z);
                    rlRotatef(time * 150.0f, 0, 1, 0);
                    rlRotatef(45.0f, 1, 0, 0);
                    rlRotatef(45.0f, 0, 0, 1);

                    float size = 0.2f + (g.value * 0.005f); 
                    if (size > 0.4f) size = 0.4f;

                    DrawCube((Vector3){0,0,0}, size, size, size, xpColor);
                    DrawCubeWires((Vector3){0,0,0}, size, size, size, WHITE);
                rlPopMatrix();
            }
            EndBlendMode();
            
        EndMode3D();

    if (mPixelMode) {
        EndTextureMode();
    }

    // -----------------------------------------------------------
    // PHASE 2: COMPOSITING & UI (2D Overlay)
    // -----------------------------------------------------------
    
    // Jika Pixel Mode aktif, gambar texture hasil render tadi ke layar full
    if (mPixelMode) {
        BeginDrawing();
        ClearBackground(BLACK);

        // Source Rect (Flip Y karena OpenGL coordinates)
        Rectangle srcRect = { 0.0f, 0.0f, (float)mTarget.texture.width, -(float)mTarget.texture.height };
        // Dest Rect (Full Screen)
        Rectangle destRect = { 0.0f, 0.0f, (float)mScreenWidth, (float)mScreenHeight };
        
        DrawTexturePro(mTarget.texture, srcRect, destRect, (Vector2){0,0}, 0.0f, WHITE);
    }
    
    // --- UI MANAGER DELEGATION ---
    
    // UI Logic
    if (mState == GameState::PLAYING) {
        mUI.DrawHUD(mPlayer, mWaveManager, (int)mEnemies.size(), mScreenWidth, mScreenHeight);
    }
    else if (mState == GameState::PAUSED) {
        // Tetap gambar HUD biar kelihatan stat-nya di belakang overlay
        mUI.DrawHUD(mPlayer, mWaveManager, (int)mEnemies.size(), mScreenWidth, mScreenHeight);
        
        // 🔥 GAMBAR LAYAR PAUSE DI ATASNYA
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