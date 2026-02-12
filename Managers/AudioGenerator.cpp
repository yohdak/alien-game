#include "AudioGenerator.h"
#include <math.h>
#include <stdlib.h> // malloc/free
#include <algorithm> // min/max

// --- BAGIAN 1: DAPUR MATEMATIKA (HELPER) ---

// 0 = Square (Laser), 1 = Noise (Ledakan/Telur), 2 = Sine (Gem)
Wave GenerateRawWave(int type, int sampleRate, float frequency) {
    Wave wave = { 0 };
    wave.sampleRate = sampleRate;
    wave.sampleSize = 32; 
    wave.channels = 1;    
    wave.frameCount = sampleRate; // Default buffer 1 detik
    
    float *data = (float *)malloc(wave.frameCount * sizeof(float));

    for (unsigned int i = 0; i < wave.frameCount; i++) {
        float t = (float)i / sampleRate;
        
        if (type == 0) { // SQUARE
            if (sinf(2.0f * PI * frequency * t) >= 0) data[i] = 0.3f;
            else data[i] = -0.3f;
        }
        else if (type == 1) { // NOISE
            data[i] = ((float)rand() / RAND_MAX * 1.0f) - 0.5f;
            data[i] *= 0.5f; 
        }
        else { // SINE
            data[i] = sinf(2.0f * PI * frequency * t) * 0.5f;
        }
    }
    wave.data = data;
    return wave;
}

// Fungsi Pintar: Masak Wave -> Potong -> Fade Out -> Jadi Sound
Sound CreateProceduralSound(int type, float freq, float duration) {
    int sampleRate = 44100;
    Wave wave = GenerateRawWave(type, sampleRate, freq);
    
    // 1. POTONG DURASI
    int targetFrames = (int)(sampleRate * duration);
    if (targetFrames < wave.frameCount) wave.frameCount = targetFrames;
    
    // 2. FADE OUT (Otomatis biar gak kasar)
    float* data = (float*)wave.data;
    for (unsigned int i = 0; i < wave.frameCount; i++) {
        float progress = (float)i / wave.frameCount;
        data[i] *= (1.0f - progress); 
    }
    
    // 3. PACKING
    Sound sfx = LoadSoundFromWave(wave);
    UnloadWave(wave); // Cuci piring
    return sfx;
}


// --- BAGIAN 2: DAFTAR RESEP (MENU) ---

void AudioGenerator::GenerateAllSounds(std::map<std::string, Sound>& soundMap) {
    
    // FORMAT: CreateProceduralSound(TIPE, FREKUENSI, DURASI)
    // Tipe: 0=Laser, 1=Noise, 2=Sine
    
    // 🥚 Telur Pecah (Noise, 0 Hz, 0.05 detik)
    soundMap["egg_break"] = CreateProceduralSound(1, 0.0f, 0.20f);

    // 💥 Ledakan (Noise, 0 Hz, 0.5 detik)
    soundMap["explosion"] = CreateProceduralSound(1, 0.0f, 0.5f);

    // 🔫 Laser (Square, 400 Hz, 0.15 detik)
    soundMap["laser"] = CreateProceduralSound(0, 400.0f, 0.15f);

    // ✨ Gem/XP (Sine, 880 Hz, 0.2 detik)
    soundMap["gem"] = CreateProceduralSound(2, 880.0f, 0.2f);
    
    // Mau nambah baru? Tinggal copas satu baris di bawah ini:
    // soundMap["powerup"] = CreateProceduralSound(0, 600.0f, 0.5f); 

    TraceLog(LOG_INFO, "🔊 AUDIO GENERATOR: All procedural sounds cooked successfully!");
}