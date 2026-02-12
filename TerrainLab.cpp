#include "raylib.h"
#include "raymath.h"
#include "rlgl.h" 
#include <vector>

// --- SHADER SOURCE CODE (Biar gak perlu file eksternal di Lab) ---
const char* VS_CODE = R"(
#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;

out vec2 fragTexCoord;
out vec3 fragNormal;
out vec3 fragPosition;

uniform mat4 mvp;
uniform mat4 matModel;

void main() {
    fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragNormal = normalize(vec3(matModel * vec4(vertexNormal, 0.0)));
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
)";

const char* FS_GROUND_CODE = R"(
#version 330
in vec2 fragTexCoord;
in vec3 fragNormal;
in vec3 fragPosition;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec3 lightPos;

void main() {
    // Simple Lighting (Ambient + Diffuse)
    vec3 lightDir = normalize(lightPos - fragPosition);
    float diff = max(dot(fragNormal, lightDir), 0.0);
    
    vec3 ambient = vec3(0.4, 0.4, 0.5); // Agak biru dikit (Sky ambient)
    vec3 diffuse = diff * vec3(1.0, 1.0, 0.9); // Matahari agak kuning

    vec4 texel = texture(texture0, fragTexCoord);
    
    // Mix Texture dengan Lighting
    vec3 result = (ambient + diffuse) * texel.rgb;
    finalColor = vec4(result, texel.a);
}
)";

// --- KONFIGURASI ---
#define MAP_RES           128
#define WORLD_SIZE        60.0f
#define PLAYER_HEIGHT     1.5f 
#define TOTAL_MESH_HEIGHT (3.0f * PLAYER_HEIGHT)
#define Y_OFFSET          (-1.0f * PLAYER_HEIGHT)

// Definisi Warna Level
const Color C_LVL_MIN_1 = BLACK;                 
const Color C_LVL_0     = (Color){85, 85, 85, 255};   
const Color C_LVL_1     = (Color){170, 170, 170, 255}; 
const Color C_LVL_2     = WHITE;                 

// --- 1. FILTER: RADIUS 5 (Pulau Luas) ---
void CleanUpTerrainLarge(Image* map, int passes) {
    Color* pixels = LoadImageColors(*map);
    Color* buffer = LoadImageColors(*map);
    int w = map->width; int h = map->height;
    int radius = 5; 

    for (int p = 0; p < passes; p++) {
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                int idx = y * w + x;
                int counts[4] = {0, 0, 0, 0}; 

                for (int dy = -radius; dy <= radius; dy++) {
                    for (int dx = -radius; dx <= radius; dx++) {
                        int ny = y + dy; int nx = x + dx;
                        if (nx < 0 || nx >= w || ny < 0 || ny >= h) continue;
                        if ((dx*dx + dy*dy) > (radius*radius)) continue; // Circular

                        int r = pixels[ny * w + nx].r;
                        if (r < 40) counts[0]++;       
                        else if (r < 120) counts[1]++; 
                        else if (r < 200) counts[2]++; 
                        else counts[3]++;              
                    }
                }
                int maxVotes = 0; int winner = -1;
                for(int i=0; i<4; i++) { if(counts[i] > maxVotes) { maxVotes = counts[i]; winner = i; } }

                Color finalColor = pixels[idx];
                if (winner == 0) finalColor = C_LVL_MIN_1;
                else if (winner == 1) finalColor = C_LVL_0;
                else if (winner == 2) finalColor = C_LVL_1;
                else if (winner == 3) finalColor = C_LVL_2;
                buffer[idx] = finalColor;
            }
        }
        for (int i = 0; i < w * h; i++) pixels[i] = buffer[i];
    }
    for (int i = 0; i < w * h; i++) ImageDrawPixel(map, i % w, i / w, pixels[i]);
    UnloadImageColors(pixels); UnloadImageColors(buffer);
}

// --- 2. FILTER: SMOOTHING (Pass = 1) ---
void SmoothTerrain(Image* map, int passes) {
    Color* pixels = LoadImageColors(*map);
    Color* buffer = LoadImageColors(*map);
    int w = map->width; int h = map->height;

    for (int p = 0; p < passes; p++) {
        for (int y = 1; y < h - 1; y++) {
            for (int x = 1; x < w - 1; x++) {
                int idx = y * w + x;
                int sumR = 0; int count = 0;
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        sumR += pixels[(y + dy) * w + (x + dx)].r;
                        count++;
                    }
                }
                int avgR = sumR / count;
                buffer[idx] = (Color){ (unsigned char)avgR, (unsigned char)avgR, (unsigned char)avgR, 255 };
            }
        }
        for (int i = 0; i < w * h; i++) pixels[i] = buffer[i];
    }
    for (int i = 0; i < w * h; i++) ImageDrawPixel(map, i % w, i / w, pixels[i]);
    UnloadImageColors(pixels); UnloadImageColors(buffer);
}

float GetTerrainHeight(Image map, Vector3 mapSize, float x, float z) {
    float normalizedX = (x + mapSize.x/2.0f) / mapSize.x;
    float normalizedZ = (z + mapSize.z/2.0f) / mapSize.z;
    int pixelX = (int)(normalizedX * map.width);
    int pixelZ = (int)(normalizedZ * map.height);
    if (pixelX < 0) pixelX = 0; if (pixelX >= map.width) pixelX = map.width - 1;
    if (pixelZ < 0) pixelZ = 0; if (pixelZ >= map.height) pixelZ = map.height - 1;
    Color c = GetImageColor(map, pixelX, pixelZ);
    return ((c.r / 255.0f) * mapSize.y) + Y_OFFSET;
}

int main() {
    InitWindow(1280, 720, "Terrain Lab - Shader & Texture Integration");
    SetTargetFPS(60);

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 20.0f, 20.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // --- 1. GENERATE & FILTER ---
    Image heightMap = GenImagePerlinNoise(MAP_RES, MAP_RES, 50, 50, 4.0f); 

    // Leveling
    for (int y = 0; y < heightMap.height; y++) {
        for (int x = 0; x < heightMap.width; x++) {
            Color c = GetImageColor(heightMap, x, y);
            if (c.r < 90)       ImageDrawPixel(&heightMap, x, y, C_LVL_MIN_1);
            else if (c.r < 140) ImageDrawPixel(&heightMap, x, y, C_LVL_0);
            else if (c.r < 190) ImageDrawPixel(&heightMap, x, y, C_LVL_1);
            else                ImageDrawPixel(&heightMap, x, y, C_LVL_2);
        }
    }
    // Radius 5
    CleanUpTerrainLarge(&heightMap, 5);
    // Smooth 1 (Sesuai request)
    SmoothTerrain(&heightMap, 1);

    // --- 2. MESH GENERATION ---
    Vector3 mapSize = { WORLD_SIZE, TOTAL_MESH_HEIGHT, WORLD_SIZE };
    Mesh mesh = GenMeshHeightmap(heightMap, mapSize);
    Model model = LoadModelFromMesh(mesh);

    // --- 3. LOAD TEXTURE (ground.png) ---
    // Pastikan file 'ground.png' ada di folder yang sama dengan executable!
    Texture2D texture;
    if (FileExists("ground.png")) {
        texture = LoadTexture("ground.png");
    } else {
        // Fallback kalau lupa naro gambar: Pake checkerboard merah
        Image fallback = GenImageChecked(64, 64, 4, 4, RED, MAROON);
        texture = LoadTextureFromImage(fallback);
        UnloadImage(fallback);
    }
    
    // Set Texture ke Model
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

    // --- 4. SETUP SHADER (FS_GROUND_CODE) ---
    Shader shader = LoadShaderFromMemory(VS_CODE, FS_GROUND_CODE);
    
    // Get Locations
    int lightPosLoc = GetShaderLocation(shader, "lightPos");
    
    // Apply Shader ke Model
    model.materials[0].shader = shader;

    Vector3 ballPos = { 0, 5, 0 };
    Vector3 lightPos = { 100.0f, 100.0f, 50.0f }; // Posisi Matahari

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        
        // Input Logic
        float speed = 10.0f;
        if (IsKeyDown(KEY_W)) ballPos.z -= speed * dt;
        if (IsKeyDown(KEY_S)) ballPos.z += speed * dt;
        if (IsKeyDown(KEY_A)) ballPos.x -= speed * dt;
        if (IsKeyDown(KEY_D)) ballPos.x += speed * dt;

        // Physics
        float groundHeight = GetTerrainHeight(heightMap, mapSize, ballPos.x, ballPos.z);
        ballPos.y = Lerp(ballPos.y, groundHeight + 0.5f, 0.2f);

        // Update Shader Uniforms
        SetShaderValue(shader, lightPosLoc, &lightPos, SHADER_UNIFORM_VEC3);

        // Camera Follow
        camera.target = ballPos;
        camera.position.x = ballPos.x;
        camera.position.y = ballPos.y + 12.0f; 
        camera.position.z = ballPos.z + 15.0f;

        BeginDrawing();
            ClearBackground(SKYBLUE);

            BeginMode3D(camera);
                Vector3 mapPos = { -mapSize.x/2, Y_OFFSET, -mapSize.z/2 };
                
                // Draw Model dengan Shader & Texture custom lu
                DrawModel(model, mapPos, 1.0f, WHITE);
                
                // Opsional: Wireframe dimatiin dulu biar texture kelihatan jelas
                // DrawModelWires(model, mapPos, 1.0f, DARKGRAY); 

                // Water
                rlDisableDepthMask();
                DrawCube((Vector3){0, -0.2f, 0}, WORLD_SIZE, 0.0f, WORLD_SIZE, (Color){0, 121, 241, 100});
                rlEnableDepthMask();

                DrawSphere(ballPos, 0.5f, RED);
            EndMode3D();

            DrawText("CUSTOM SHADER + TEXTURE", 20, 20, 20, WHITE);
            if (!FileExists("ground.png")) {
                DrawText("WARNING: 'ground.png' not found! Using fallback.", 20, 50, 20, RED);
            }
            DrawFPS(1180, 20);
        EndDrawing();
    }

    UnloadTexture(texture);
    UnloadModel(model);
    UnloadShader(shader);
    UnloadImage(heightMap);
    CloseWindow();
    return 0;
}