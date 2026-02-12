#include "raylib.h"
#include "raymath.h"

int main() {
    InitWindow(800, 600, "Test Collision - Map Only");
    SetTargetFPS(60);

    // Setup Kamera
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 10.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Load Map Collider
    // Pastikan di Blender sudah "Apply Transform" sebelum export!
    Model mapCol = LoadModel("map_col.glb");

    // Setup Player
    Vector3 playerPos = { 0.0f, 2.0f, 0.0f }; // Mulai agak tinggi biar gak nyangkut
    float playerRadius = 0.5f;
    float speed = 5.0f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // 1. Kontrol Player (Simple WASD)
        Vector3 velocity = { 0.0f, 0.0f, 0.0f };
        if (IsKeyDown(KEY_W)) velocity.z -= speed * dt;
        if (IsKeyDown(KEY_S)) velocity.z += speed * dt;
        if (IsKeyDown(KEY_A)) velocity.x -= speed * dt;
        if (IsKeyDown(KEY_D)) velocity.x += speed * dt;

        // Prediksi posisi selanjutnya
        Vector3 nextPos = Vector3Add(playerPos, velocity);
        
        // 2. Cek Collision
        bool hit = false;
        
        // Loop cek semua mesh yang ada di dalam map_col
        for (int i = 0; i < mapCol.meshCount; i++) {
            // Ambil kotak pembatas (Bounding Box) dari mesh tersebut
            BoundingBox box = GetMeshBoundingBox(mapCol.meshes[i]);
            
            // Cek apakah bola player akan menabrak kotak ini
            if (CheckCollisionBoxSphere(box, nextPos, playerRadius)) {
                hit = true;
                break; // Jika nabrak satu, stop pengecekan
            }
        }

        // 3. Update Posisi (Hanya gerak kalau tidak nabrak)
        if (!hit) {
            playerPos = nextPos;
        }

        // Update Kamera mengikuti player (Opsional)
        camera.target = playerPos;
        camera.position = Vector3Add(playerPos, (Vector3){0, 15, 15});

        // 4. Draw
        BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);
                
                // Gambar Map Collider sebagai Garis (Wireframe) warna abu-abu
                DrawModelWires(mapCol, (Vector3){0,0,0}, 1.0f, DARKGRAY);
                
                // Gambar Player
                DrawSphere(playerPos, playerRadius, RED);
                
                DrawGrid(20, 1.0f);
            EndMode3D();
            
            DrawText("WASD: Gerak | Merah: Player | Abu-abu: Wall/Floor", 10, 10, 20, BLACK);
        EndDrawing();
    }

    UnloadModel(mapCol);
    CloseWindow();
    return 0;
}