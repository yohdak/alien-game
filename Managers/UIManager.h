#pragma once
#include "raylib.h"
#include "../Player/Player.h"
#include "../Systems/WaveManager.h"

class UIManager {
public:
    UIManager();
    ~UIManager();

    void DrawHUD(const Player& player, const WaveManager& waveManager, int enemyCount, int screenW, int screenH);
    void DrawPause(int screenW, int screenH);
    void DrawGameOver(int screenW, int screenH, int waveReached, int levelReached);
    void DrawVictory(int screenW, int screenH, int levelReached);
private:
    // Helper untuk menggambar bar (HP/XP)
    void DrawBar(int x, int y, int width, int height, float percentage, Color color, Color bgColor);
    
};