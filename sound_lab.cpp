#include "raylib.h"
#include <math.h>
#include <stdlib.h> 
#include <string.h>
#include <cstdio> // Buat sprintf/TextFormat

// --- 1. HELPER: GetRandomFloat ---
float GetRandomFloat(float min, float max) {
    return min + ((float)GetRandomValue(0, 10000) / 10000.0f) * (max - min);
}

// --- 2. HELPER: MANUAL WAVE GENERATOR ---
Wave GenerateWaveManual(int type, int sampleRate, float frequency) {
    Wave wave = { 0 };
    wave.sampleRate = sampleRate;
    wave.sampleSize = 32; 
    wave.channels = 1;    
    
    // Default buffer 1 detik
    wave.frameCount = sampleRate; 
    float *data = (float *)malloc(wave.frameCount * sizeof(float));

    for (int i = 0; i < wave.frameCount; i++) {
        float t = (float)i / sampleRate;
        
        if (type == 0) { // SQUARE (Laser)
            if (sinf(2.0f * PI * frequency * t) >= 0) data[i] = 0.3f;
            else data[i] = -0.3f;
        }
        else if (type == 1) { // NOISE (Ledakan/Telur)
            // Noise murni (acak)
            data[i] = ((float)rand() / RAND_MAX * 1.0f) - 0.5f;
            data[i] *= 0.5f; // Volume setengan biar gak pecah speaker
        }
        else { // SINE (Gem)
            data[i] = sinf(2.0f * PI * frequency * t) * 0.5f;
        }
    }

    wave.data = data;
    return wave;
}

// --- 3. GENERATOR SUARA UTAMA ---
Sound GenerateCustomSound(int type, float frequency, float duration) {
    int sampleRate = 44100;
    Wave wave = GenerateWaveManual(type, sampleRate, frequency);

    // Potong Durasi
    int newFrameCount = (int)(sampleRate * duration);
    if (newFrameCount < wave.frameCount) wave.frameCount = newFrameCount;

    // Fade Out
    float* data = (float*)wave.data;
    for (int i = 0; i < wave.frameCount; i++) {
        float progress = (float)i / wave.frameCount;
        data[i] *= (1.0f - progress); 
    }

    Sound sfx = LoadSoundFromWave(wave);
    UnloadWave(wave); 
    return sfx;
}

int main() {
    InitWindow(600, 500, "LABORATORIUM: TELUR & BOM"); // Window agak gedean
    InitAudioDevice();
    SetTargetFPS(60);

    // Default Settings
    int currentType = 0;     // 0=Laser, 1=Noise (Telur/Bom), 2=Sine
    float currentFreq = 400.0f;
    float currentDur = 0.15f;
    float currentPitch = 1.0f; // 🔥 PENTING BUAT EFEK TELUR VS BOM
    
    Sound currentSound = { 0 };
    bool soundReady = false;
    
    // Nama Preset buat display
    const char* presetName = "CUSTOM";

    while (!WindowShouldClose()) {
        // --- INPUT PRESETS (RESEP RAHASIA) ---
        
        // PRESET 1: LASER
        if (IsKeyPressed(KEY_ONE)) {
            currentType = 0; 
            currentFreq = 400.0f; 
            currentDur = 0.15f;
            currentPitch = 1.0f;
            presetName = "LASER (Square)";
        }

        // PRESET 2: TELUR PECAH (Pyak!)
        if (IsKeyPressed(KEY_TWO)) {
            currentType = 1; // Noise
            currentFreq = 0; // Gak ngaruh di noise
            currentDur = 0.05f; // Super Pendek
            currentPitch = 1.8f; // Pitch TINGGI biar cempreng
            presetName = "TELUR PECAH (High Noise)";
        }

        // PRESET 3: LEDAKAN (Duuum...)
        if (IsKeyPressed(KEY_THREE)) {
            currentType = 1; // Noise
            currentFreq = 0; 
            currentDur = 0.5f; // Panjang
            currentPitch = 0.5f; // Pitch RENDAH biar ngebass
            presetName = "LEDAKAN (Low Noise)";
        }

        // --- MANUAL TWEAKING ---
        // Atur Pitch (W/S)
        if (IsKeyDown(KEY_W)) currentPitch += 0.05f;
        if (IsKeyDown(KEY_S)) currentPitch -= 0.05f;
        if (currentPitch < 0.1f) currentPitch = 0.1f;

        // Atur Durasi (A/D)
        if (IsKeyDown(KEY_D)) currentDur += 0.01f;
        if (IsKeyDown(KEY_A)) currentDur -= 0.01f;
        if (currentDur < 0.01f) currentDur = 0.01f;

        // PLAY
        if (IsKeyPressed(KEY_SPACE)) {
            if (soundReady) UnloadSound(currentSound);
            
            // Masak suara baru
            currentSound = GenerateCustomSound(currentType, currentFreq, currentDur);
            soundReady = true;
            
            // Set Pitch sesuai settingan
            SetSoundPitch(currentSound, currentPitch);
            PlaySound(currentSound);
        }

        // --- DRAW UI ---
        BeginDrawing();
        ClearBackground((Color){30, 30, 40, 255});

        DrawText("LABORATORIUM SUARA", 20, 20, 30, WHITE);
        DrawLine(20, 60, 580, 60, GRAY);

        // Display Preset Aktif
        DrawText("PRESET AKTIF:", 20, 80, 20, LIGHTGRAY);
        DrawText(presetName, 180, 80, 20, YELLOW);

        // Menu Tombol
        DrawText("TEKAN [1] : LASER", 20, 120, 20, GREEN);
        DrawText("TEKAN [2] : TELUR PECAH (Target)", 20, 150, 20, ORANGE);
        DrawText("TEKAN [3] : LEDAKAN BOM (Target)", 20, 180, 20, RED);

        // Display Parameter
        int yStart = 240;
        DrawText("PARAMETER MANUAL:", 20, yStart, 20, GRAY);
        
        DrawText(TextFormat("DURASI [A/D] : %.3f sec", currentDur), 40, yStart+30, 20, WHITE);
        
        // Highlight Pitch karena ini kuncinya
        Color pitchColor = (currentPitch > 1.2) ? ORANGE : (currentPitch < 0.8 ? RED : WHITE);
        DrawText(TextFormat("PITCH  [W/S] : %.2f x", currentPitch), 40, yStart+60, 20, pitchColor);
        
        DrawText("(Pitch Tinggi = Cempreng, Pitch Rendah = Ngebass)", 40, yStart+85, 15, GRAY);

        // Visualisasi Play
        if (IsSoundPlaying(currentSound)) {
            DrawRectangle(40, 380, GetRandomValue(100, 500), 50, pitchColor);
            DrawText("BUNYI !!!", 250, 395, 20, BLACK);
        } else {
            DrawRectangleLines(40, 380, 520, 50, GRAY);
        }

        DrawText("[SPACE] PLAY SOUND", 180, 450, 20, SKYBLUE);

        EndDrawing();
    }

    if (soundReady) UnloadSound(currentSound);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}