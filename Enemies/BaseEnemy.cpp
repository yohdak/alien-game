#include "BaseEnemy.h"

BaseEnemy::BaseEnemy(int tierInput, Vector3 startPos) 
    : position(startPos)
    , velocity({0, 0, 0})
    , hp(10.0f)
    , maxHp(10.0f)
    , speed(1.0f)
    , radius(1.0f)
    , tier(tierInput)
    , active(true)
    , xpReward(10)
    , flashTimer(0.0f) // Reset timer saat spawn
{
}

BaseEnemy::~BaseEnemy() {
    // Destructor kosong, tapi perlu didefinisikan agar vtable aman
}

void BaseEnemy::TakeDamage(float amount) {
    hp -= amount;
    
    // 🔥 TRIGGER KEDIP
    // Set timer ke 0.1 detik (100ms) setiap kali kena damage
    flashTimer = 0.1f; 
    
    if (hp <= 0) {
        active = false;
    }
}

void BaseEnemy::UpdateFlash(float dt) {
    // Kurangi timer setiap frame
    if (flashTimer > 0) {
        flashTimer -= dt;
    }
}
Color BaseEnemy::GetRenderColor(Color originalColor) {
    // 🔥 Jika kena hit
    if (flashTimer > 0) {
        // LOGIKA: "Hampir Putih"
        // Kita campur 80% Putih + 20% Warna Asli
        // Hasilnya flash terang tapi tidak "buta"
        
        float mixFactor = 0.8f; // Ubah angka ini (0.0 - 1.0) untuk atur keputihan
        
        unsigned char r = (unsigned char)(255 * mixFactor + originalColor.r * (1.0f - mixFactor));
        unsigned char g = (unsigned char)(255 * mixFactor + originalColor.g * (1.0f - mixFactor));
        unsigned char b = (unsigned char)(255 * mixFactor + originalColor.b * (1.0f - mixFactor));
        
        return (Color){r, g, b, 255};
    }
    
    // Jika normal
    return originalColor;
}