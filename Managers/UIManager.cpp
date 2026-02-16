#include "UIManager.h"
#include <string>
#include <cstdio> // Untuk snprintf/TextFormat

UIManager::UIManager() {}
UIManager::~UIManager() {}

void UIManager::DrawBar(int x, int y, int width, int height, float percentage, Color color, Color bgColor) {
    DrawRectangle(x, y, width, height, bgColor);
    DrawRectangle(x, y, (int)(width * percentage), height, color);
    DrawRectangleLines(x, y, width, height, RAYWHITE);
}

void UIManager::DrawHUD(const Player& player, const WaveManager& waveManager, int enemyCount, int screenW, int screenH) {
    // --- HP BAR ---
    float hpRatio = player.GetHp() / player.GetMaxHp();
    DrawBar(20, screenH - 40, 200, 20, hpRatio, RED, DARKGRAY);
    DrawText(TextFormat("HP: %.0f/%.0f", player.GetHp(), player.GetMaxHp()), 
             25, screenH - 37, 15, WHITE);

    // --- XP BAR ---
    float xpRatio = player.GetCurrentXP() / player.GetNextLevelXP();
    DrawRectangle(0, 0, (int)(screenW * xpRatio), 5, SKYBLUE);
    
    DrawText(TextFormat("LEVEL %d", player.GetLevel()), 20, 20, 20, WHITE);
    DrawText(TextFormat("XP: %.0f/%.0f", player.GetCurrentXP(), player.GetNextLevelXP()), 
             20, 45, 16, GRAY);

    // --- WAVE INFO ---
    WaveState wState = waveManager.GetState();
    WaveType wType = waveManager.GetWaveType();
    int wave = waveManager.GetCurrentWave();
    int centerX = screenW / 2;

    if (wState == WaveState::WAITING) {
        const char* nextWaveText = (wType == WaveType::BOSS) ? "BOSS WAVE INCOMING" : "NEXT WAVE IN";
        Color textColor = (wType == WaveType::BOSS) ? RED : YELLOW;
        
        DrawText(nextWaveText, centerX - MeasureText(nextWaveText, 30) / 2, 80, 30, textColor);
        DrawText(TextFormat("%.1f", waveManager.GetWaveTimer()), centerX - 20, 120, 40, RED);
    }
    else if (wState == WaveState::SPAWNING || wState == WaveState::FIGHTING) {
        Color waveColor = (wType == WaveType::BOSS) ? GOLD : RED;
        const char* waveLabel = (wType == WaveType::BOSS) ? "BOSS WAVE" : "WAVE";
        
        DrawText(TextFormat("%s %d/25", waveLabel, wave), centerX - 80, 20, 30, waveColor);
        DrawText(TextFormat("ENEMIES: %d", enemyCount), centerX - 70, 55, 20, WHITE);
        
        if (wState == WaveState::SPAWNING) {
            DrawText(TextFormat("Incoming: %d", waveManager.GetRemainingEnemies()), 
                     centerX - 60, 80, 16, ORANGE);
        }
    }
    else if (wState == WaveState::COMPLETED) {
        DrawText("WAVE CLEARED!", centerX - 140, screenH/2 - 60, 50, GREEN);
        DrawText(TextFormat("+%d XP BONUS", waveManager.GetWaveBonusXP()), 
                 centerX - 100, screenH/2, 30, GOLD);
        DrawText(TextFormat("Next wave in %.1f", waveManager.GetWaveTimer()), 
                 centerX - 100, screenH/2 + 50, 20, GRAY);
    }

    // --- WEAPON UI ---
    WeaponType weapon = player.GetCurrentWeapon();
    const char* weaponName = "";
    Color weaponColor = WHITE;
    
    switch (weapon) {
        case WeaponType::PISTOL: weaponName = "PISTOL"; weaponColor = GRAY; break;
        case WeaponType::SHOTGUN: weaponName = "SHOTGUN"; weaponColor = ORANGE; break;
        case WeaponType::MINIGUN: weaponName = "MINIGUN"; weaponColor = YELLOW; break;
        case WeaponType::BAZOOKA: weaponName = "BAZOOKA"; weaponColor = PURPLE; break;
    }
    
    int boxX = screenW - 220;
    int boxY = screenH - 100;
    
    DrawRectangle(boxX, boxY, 200, 80, ColorAlpha(BLACK, 0.7f));
    DrawRectangleLines(boxX, boxY, 200, 80, weaponColor);
    DrawText(weaponName, boxX + 10, boxY + 10, 25, weaponColor);
    DrawText("[1-4 or SCROLL]", boxX + 10, boxY + 45, 15, GRAY);
    
    if (player.HasMagnetBuff()) {
        DrawText("MAGNET ACTIVE", boxX + 10, boxY + 65, 14, BLUE);
    }

    DrawFPS(screenW - 80, 20);
}

void UIManager::DrawPause(int screenW, int screenH, int selection) {
    // Dark overlay
    DrawRectangle(0, 0, screenW, screenH, ColorAlpha(BLACK, 0.6f));
    
    // Title
    const char* title = "PAUSED";
    int titleW = MeasureText(title, 50);
    DrawText(title, screenW/2 - titleW/2, 100, 50, WHITE);
    
    // Menu options
    const char* options[] = {"CONTINUE", "RESTART", "SETTINGS", "MAIN MENU"};
    int startY = 230;
    
    for (int i = 0; i < 4; i++) {
        Color color = (i == selection) ? YELLOW : LIGHTGRAY;
        int textW = MeasureText(options[i], 30);
        DrawText(options[i], screenW/2 - textW/2, startY + i*55, 30, color);
        
        // Selection arrows
        if (i == selection) {
            DrawText(">", screenW/2 - textW/2 - 40, startY + i*55, 30, YELLOW);
            DrawText("<", screenW/2 + textW/2 + 20, startY + i*55, 30, YELLOW);
        }
    }
    
    // Controls hint
    DrawText("[W/S] Navigate  [ENTER] Select  [P/ESC] Quick Resume", 
             screenW/2 - 250, screenH - 60, 16, GRAY);
}

void UIManager::DrawGameOver(int screenW, int screenH, int waveReached, int levelReached) {
    DrawRectangle(0, 0, screenW, screenH, (Color){0, 0, 0, 200});
    
    // ✅ FIX: Move all text higher to avoid overlap with high score table
    const char* gameOverText = "SOTO TUMPAH";
    int textWidth = MeasureText(gameOverText, 50);
    DrawText(gameOverText, screenW/2 - textWidth/2, 80, 50, RED);
    
    const char* statsText = TextFormat("SURVIVED TO WAVE %d", waveReached);
    int statsWidth = MeasureText(statsText, 25);
    DrawText(statsText, screenW/2 - statsWidth/2, 150, 25, GRAY);
    
    const char* levelText = TextFormat("FINAL LEVEL: %d", levelReached);
    int levelWidth = MeasureText(levelText, 20);
    DrawText(levelText, screenW/2 - levelWidth/2, 185, 20, SKYBLUE);
    
    const char* retryText = "Press [R] to Retry";
    int retryWidth = MeasureText(retryText, 20);
    DrawText(retryText, screenW/2 - retryWidth/2, 230, 20, WHITE);
}

void UIManager::DrawVictory(int screenW, int screenH, int levelReached) {
    DrawRectangle(0, 0, screenW, screenH, (Color){0, 0, 0, 200});
    
    const char* victoryText = "VICTORY!";
    int textWidth = MeasureText(victoryText, 60);
    DrawText(victoryText, screenW/2 - textWidth/2, screenH/2 - 80, 60, GOLD);
    
    const char* completeText = "ALL 25 WAVES COMPLETED!";
    int completeWidth = MeasureText(completeText, 30);
    DrawText(completeText, screenW/2 - completeWidth/2, screenH/2 - 10, 30, GREEN);
    
    const char* levelText = TextFormat("FINAL LEVEL: %d", levelReached);
    int levelWidth = MeasureText(levelText, 25);
    DrawText(levelText, screenW/2 - levelWidth/2, screenH/2 + 40, 25, SKYBLUE);
    
    const char* retryText = "Press [R] to Play Again";
    int retryWidth = MeasureText(retryText, 20);
    DrawText(retryText, screenW/2 - retryWidth/2, screenH/2 + 100, 20, WHITE);
}
// ============================================================================
// FEATURE 4: HIGH SCORE DISPLAY
// ============================================================================
void UIManager::DrawHighScores(const std::vector<HighScoreEntry>& scores, int screenW, int screenH) {
    if (scores.empty()) return;
    
    // ✅ FIX: Position table at center screen
    int startY = 300;  // Center position
    
    // Title
    const char* title = "TOP 5 HIGH SCORES";
    DrawText(title, screenW/2 - MeasureText(title, 25)/2, startY, 25, GOLD);
    
    // Table Header
    startY += 40;
    DrawText("RANK", 100, startY, 18, GRAY);
    DrawText("SCORE", 180, startY, 18, GRAY);
    DrawText("WAVE", 280, startY, 18, GRAY);
    DrawText("LEVEL", 360, startY, 18, GRAY);
    DrawText("DATE", 450, startY, 18, GRAY);
    
    // Scores
    startY += 30;
    for (int i = 0; i < scores.size() && i < 5; i++) {
        Color rankColor = (i == 0) ? GOLD : ((i == 1) ? LIGHTGRAY : WHITE);
        
        DrawText(TextFormat("#%d", i+1), 100, startY + i*30, 18, rankColor);
        DrawText(TextFormat("%d", scores[i].score), 180, startY + i*30, 18, rankColor);
        DrawText(TextFormat("%d", scores[i].wave), 280, startY + i*30, 18, rankColor);
        DrawText(TextFormat("%d", scores[i].level), 360, startY + i*30, 18, rankColor);
        DrawText(scores[i].timestamp.c_str(), 450, startY + i*30, 14, GRAY);
    }
}

// ============================================================================
// FEATURE 2: SETTINGS MENU UI
// ============================================================================
void UIManager::DrawSettings(int screenW, int screenH, float musicVol, float sfxVol, 
                              bool pixelMode, bool screenShake, int selectedOption) {
    // Background
    DrawRectangle(0, 0, screenW, screenH, ColorAlpha(BLACK, 0.8f));
    
    const char* title = "SETTINGS";
    DrawText(title, screenW/2 - MeasureText(title, 40)/2, 80, 40, GOLD);
    
    int startY = 200;
    int spacing = 80;
    
    // Music Volume Slider
    const char* musicLabel = TextFormat("MUSIC VOLUME: %d%%", (int)(musicVol * 100));
    Color musicColor = (selectedOption == 0) ? YELLOW : WHITE;
    DrawText(musicLabel, 200, startY, 25, musicColor);
    DrawRectangle(200, startY + 35, 400, 15, DARKGRAY);
    DrawRectangle(200, startY + 35, (int)(400 * musicVol), 15, GREEN);
    if (selectedOption == 0) DrawText("<  >", 620, startY, 25, YELLOW);
    
    // SFX Volume Slider
    const char* sfxLabel = TextFormat("SFX VOLUME: %d%%", (int)(sfxVol * 100));
    Color sfxColor = (selectedOption == 1) ? YELLOW : WHITE;
    DrawText(sfxLabel, 200, startY + spacing, 25, sfxColor);
    DrawRectangle(200, startY + spacing + 35, 400, 15, DARKGRAY);
    DrawRectangle(200, startY + spacing + 35, (int)(400 * sfxVol), 15, BLUE);
    if (selectedOption == 1) DrawText("<  >", 620, startY + spacing, 25, YELLOW);
    
    // Pixel Mode Toggle
    const char* pixelLabel = TextFormat("PIXEL MODE: %s", pixelMode ? "ON" : "OFF");
    Color pixelColor = (selectedOption == 2) ? YELLOW : WHITE;
    DrawText(pixelLabel, 200, startY + spacing*2, 25, pixelColor);
    if (selectedOption == 2) DrawText("<  >", 550, startY + spacing*2, 25, YELLOW);
    
    // Screen Shake Toggle
    const char* shakeLabel = TextFormat("SCREEN SHAKE: %s", screenShake ? "ON" : "OFF");
    Color shakeColor = (selectedOption == 3) ? YELLOW : WHITE;
    DrawText(shakeLabel, 200, startY + spacing*3, 25, shakeColor);
    if (selectedOption == 3) DrawText("<  >", 570, startY + spacing*3, 25, YELLOW);
    
    // Instructions
    DrawText("Use [W/S] to navigate, [A/D] or [Left/Right] to change values", 
             screenW/2 - 350, screenH - 100, 20, GRAY);
    DrawText("Press [ESC] or [ENTER] to return", screenW/2 - 200, screenH - 60, 20, LIGHTGRAY);
}

// ============================================================================
// FEATURE 3: TUTORIAL/HELP SCREEN
// ============================================================================
void UIManager::DrawTutorial(int screenW, int screenH) {
    // Background
    DrawRectangle(0, 0, screenW, screenH, ColorAlpha(BLACK, 0.9f));
    
    const char* title = "HOW TO PLAY";
    DrawText(title, screenW/2 - MeasureText(title, 50)/2, 50, 50, GOLD);
    
    int col1X = 100;
    int col2X = 600;
    int startY = 150;
    
    // Column 1: Controls
    DrawText("=== CONTROLS ===", col1X, startY, 25, SKYBLUE);
    DrawText("[W A S D]  - Move", col1X+20, startY+40, 20, WHITE);
    DrawText("[MOUSE]    - Aim", col1X+20, startY+70, 20, WHITE);
    DrawText("[L-CLICK]  - Shoot", col1X+20, startY+100, 20, WHITE);
    DrawText("[R-CLICK]  - Dash (cooldown)", col1X+20, startY+130, 20, WHITE);
    DrawText("[1 2 3 4]  - Switch weapon", col1X+20, startY+160, 20, WHITE);
    DrawText("[SCROLL]   - Switch weapon", col1X+20, startY+190, 20, WHITE);
    DrawText("[P / ESC]  - Pause", col1X+20, startY+220, 20, WHITE);
    
    // Column 2: Gameplay
    DrawText("=== GAMEPLAY ===", col2X, startY, 25, SKYBLUE);
    DrawText("Survive 25 waves of enemies!", col2X+20, startY+40, 20, WHITE);
    DrawText("Kill enemies to gain XP", col2X+20, startY+70, 20, WHITE);
    DrawText("Level up to increase max HP", col2X+20, startY+100, 20, WHITE);
    DrawText("Collect yellow XP orbs", col2X+20, startY+130, 20, YELLOW);
    DrawText("Every 5 waves = Boss Fight", col2X+20, startY+160, 20, RED);
    DrawText("", col2X+20, startY+190, 20, WHITE);
    
    // Tips
    DrawText("=== TIPS  ===", col1X, startY+280, 25, GREEN);
    DrawText("• Use DASH to dodge enemy attacks (I-frames!)", col1X+20, startY+320, 18, WHITE);
    DrawText("• Magnet buff attracts XP orbs from far away", col1X+20, startY+350, 18, BLUE);
    DrawText("• Red walls can be destroyed with bullets", col1X+20, startY+380, 18, RED);
    DrawText("• Shotgun is best for close range swarms", col1X+20, startY+410, 18, ORANGE);
    DrawText("• Bazooka deals explosive AOE damage", col1X+20, startY+440, 18, PURPLE);
    
    DrawText("Press [ENTER] or [ESC] to close", screenW/2 - 200, screenH - 60, 20, GRAY);
}
