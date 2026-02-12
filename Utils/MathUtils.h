#pragma once
#include "raylib.h"

// Inline function biar gak kena multiple definition
inline float GetRandomFloat(float min, float max) {
    return min + ((float)GetRandomValue(0, 10000) / 10000.0f) * (max - min);
}