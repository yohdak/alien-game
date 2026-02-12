#pragma once
#include "raylib.h"
#include <map>
#include <string>

namespace AudioGenerator {
    // Fungsi utama yang dipanggil AssetManager
    void GenerateAllSounds(std::map<std::string, Sound>& soundMap);
}