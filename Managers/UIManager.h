#include "raylib.h"
#include "../Player/Player.h"
#include "../Systems/WaveManager.h"
#include "../Managers/SaveManager.h" // For HighScoreEntry
#include <vector>

class UIManager {
public:
    UIManager();
    ~UIManager();

    void DrawHUD(const Player& player, const WaveManager& waveManager, int enemyCount, int screenW, int screenH);
    void DrawPause(int screenW, int screenH, int selection);
    void DrawGameOver(int screenW, int screenH, int waveReached, int levelReached);
    void DrawVictory(int screenW, int screenH, int levelReached);
    
    // ✅ NEW: Feature 4 - High Score Display
    void DrawHighScores(const std::vector<HighScoreEntry>& scores, int screenW, int screenH);
    
    // ✅ NEW: Feature 2 - Settings UI
    void DrawSettings(int screenW, int screenH, float musicVol, float sfxVol, bool pixelMode, bool screenShake, int selectedOption);
    
    // ✅ NEW: Feature 3 - Tutorial Screen
    void DrawTutorial(int screenW, int screenH);
    
private:
    // Helper untuk menggambar bar (HP/XP)
    void DrawBar(int x, int y, int width, int height, float percentage, Color color, Color bgColor);
    
};