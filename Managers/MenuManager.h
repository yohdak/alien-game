#pragma once
#include "raylib.h"
#include <vector>
#include <string>

// Enum untuk komunikasi ke Game.cpp
enum class MenuAction {
    NONE,
    CONTINUE_GAME,     // ✅ SAVE: Load saved game
    START_SURVIVAL,    // Masuk ke Survival Mode
    START_ADVENTURE,   // Masuk ke Adventure Mode (Coming Soon)
    OPEN_SETTINGS,
    OPEN_CREDITS,
    EXIT_GAME
};

enum class MenuPage {
    MAIN,
    NEW_GAME_SELECT
};

class MenuManager {
public:
    MenuManager();
    void Update();
    void Draw(int screenW, int screenH, Texture2D bgTexture);
    
    MenuAction GetLastAction();
    void ResetAction();
    
    // ✅ SAVE SYSTEM: Set apakah Continue button aktif
    void SetHasSaveFile(bool hasSave) { mHasSaveFile = hasSave; }

private:
    MenuPage mCurrentPage;
    int mSelectionIndex;
    MenuAction mLastAction;
    bool mHasSaveFile;     // ✅ SAVE: Apakah Continue option aktif

    // Data Menu
    std::vector<const char*> mMainOptions = { "NEW GAME", "CONTINUE", "SETTINGS", "CREDITS", "EXIT" };
    std::vector<const char*> mNewGameOptions = { "ADVENTURE (STORY)", "SURVIVAL (WAVE)", "BACK" };

    void ProcessNavigation(int maxOptions);
};