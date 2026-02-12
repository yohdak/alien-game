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

void UIManager::DrawPause(int screenW, int screenH) {
    // 1. Overlay Gelap Transparan
    DrawRectangle(0, 0, screenW, screenH, ColorAlpha(BLACK, 0.5f));

    // 2. Teks PAUSED
    const char* text = "PAUSED";
    int fontSize = 60;
    int textW = MeasureText(text, fontSize);
    
    // Draw Text Centered
    DrawText(text, screenW/2 - textW/2, screenH/2 - 30, fontSize, WHITE);
    
    // Instruksi Resume
    const char* subText = "Press [P] to Resume";
    int subW = MeasureText(subText, 20);
    DrawText(subText, screenW/2 - subW/2, screenH/2 + 40, 20, LIGHTGRAY);
}

void UIManager::DrawGameOver(int screenW, int screenH, int waveReached, int levelReached) {
    DrawRectangle(0, 0, screenW, screenH, (Color){0, 0, 0, 200});
    
    const char* gameOverText = "SOTO TUMPAH";
    int textWidth = MeasureText(gameOverText, 50);
    DrawText(gameOverText, screenW/2 - textWidth/2, screenH/2 - 60, 50, RED);
    
    const char* statsText = TextFormat("SURVIVED TO WAVE %d", waveReached);
    int statsWidth = MeasureText(statsText, 25);
    DrawText(statsText, screenW/2 - statsWidth/2, screenH/2, 25, GRAY);
    
    const char* levelText = TextFormat("FINAL LEVEL: %d", levelReached);
    int levelWidth = MeasureText(levelText, 20);
    DrawText(levelText, screenW/2 - levelWidth/2, screenH/2 + 40, 20, SKYBLUE);
    
    const char* retryText = "Press [R] to Retry";
    int retryWidth = MeasureText(retryText, 20);
    DrawText(retryText, screenW/2 - retryWidth/2, screenH/2 + 100, 20, WHITE);
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