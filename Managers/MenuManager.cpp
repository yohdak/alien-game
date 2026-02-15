#include "MenuManager.h"
#include <cmath>

MenuManager::MenuManager() 
    : mCurrentPage(MenuPage::MAIN), mSelectionIndex(0), mLastAction(MenuAction::NONE) {}

MenuAction MenuManager::GetLastAction() { return mLastAction; }
void MenuManager::ResetAction() { mLastAction = MenuAction::NONE; }

void MenuManager::Update() {
    int max = (mCurrentPage == MenuPage::MAIN) ? mMainOptions.size() : mNewGameOptions.size();
    ProcessNavigation(max);

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        if (mCurrentPage == MenuPage::MAIN) {
            switch (mSelectionIndex) {
                case 0: mCurrentPage = MenuPage::NEW_GAME_SELECT; mSelectionIndex = 0; break; // New Game
                case 1: /* TODO: Load Logic */ break; // Continue
                case 2: mLastAction = MenuAction::OPEN_SETTINGS; break;
                case 3: mLastAction = MenuAction::OPEN_CREDITS; break;
                case 4: mLastAction = MenuAction::EXIT_GAME; break;
            }
        }
        else if (mCurrentPage == MenuPage::NEW_GAME_SELECT) {
            switch (mSelectionIndex) {
                case 0: mLastAction = MenuAction::START_ADVENTURE; break;
                case 1: mLastAction = MenuAction::START_SURVIVAL; break;
                case 2: mCurrentPage = MenuPage::MAIN; mSelectionIndex = 0; break; // Back
            }
        }
    }
    
    // Tombol Back (Escape) saat di sub-menu
    if (IsKeyPressed(KEY_ESCAPE) && mCurrentPage == MenuPage::NEW_GAME_SELECT) {
        mCurrentPage = MenuPage::MAIN;
        mSelectionIndex = 0;
    }
}

void MenuManager::ProcessNavigation(int maxOptions) {
    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
        mSelectionIndex--;
        if (mSelectionIndex < 0) mSelectionIndex = maxOptions - 1;
    }
    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
        mSelectionIndex++;
        if (mSelectionIndex >= maxOptions) mSelectionIndex = 0;
    }
}

void MenuManager::Draw(int screenW, int screenH, Texture2D bgTexture) {
    // Background
    DrawTexturePro(bgTexture, (Rectangle){0,0,(float)bgTexture.width,(float)bgTexture.height}, 
                   (Rectangle){0,0,(float)screenW,(float)screenH}, (Vector2){0,0}, 0.0f, WHITE);

    std::vector<const char*>& currentOptions = (mCurrentPage == MenuPage::MAIN) ? mMainOptions : mNewGameOptions;
    
    int menuCenterX = (int)(screenW * 0.75f); 
    const char* title = (mCurrentPage == MenuPage::MAIN) ? "MEGABONK SURVIVAL" : "SELECT MODE";
    
    DrawText(title, menuCenterX - MeasureText(title, 40)/2 + 3, 103, 40, BLACK);
    DrawText(title, menuCenterX - MeasureText(title, 40)/2, 100, 40, GOLD);

    int startY = screenH / 2 - 50;
    int spacing = 60;

    for (int i = 0; i < currentOptions.size(); i++) {
        bool isSelected = (i == mSelectionIndex);
        Color color = isSelected ? YELLOW : RAYWHITE;
        if (mCurrentPage == MenuPage::MAIN && i == 1) color = GRAY; // Must view file first to know line numbers.m dulu
        
        int fontSize = isSelected ? 35 : 25;
        int textW = MeasureText(currentOptions[i], fontSize);
        int posX = menuCenterX - textW/2;
        int posY = startY + (i * spacing);

        DrawText(currentOptions[i], posX + 2, posY + 2, fontSize, BLACK);
        DrawText(currentOptions[i], posX, posY, fontSize, color);

        if (isSelected) {
            float time = (float)GetTime() * 10.0f;
            int offset = (int)(sinf(time) * 5.0f);
            DrawText(">", posX - 30 + offset, posY, fontSize, color);
            DrawText("<", posX + textW + 15 - offset, posY, fontSize, color);
        }
    }
}