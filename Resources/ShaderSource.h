#pragma once

inline const char* VS_CODE = R"(
    #version 330
    layout(location = 0) in vec3 vertexPosition;
    layout(location = 1) in vec2 vertexTexCoord;
    layout(location = 2) in vec3 vertexNormal;
    
    uniform mat4 mvp;
    uniform mat4 matModel;
    uniform mat4 matNormal;

    out vec3 fragPosition;
    out vec2 fragTexCoord;
    out vec3 fragNormal;

    void main() {
        fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));
        fragTexCoord = vertexTexCoord; 
        fragNormal = normalize(vec3(matNormal * vec4(vertexNormal, 1.0)));
        gl_Position = mvp * vec4(vertexPosition, 1.0);
    }
)";

// Fragment Shader: STOCHASTIC TILING + RANDOM ROTATION
inline const char* FS_GROUND_CODE = R"(
    #version 330
    in vec3 fragPosition;
    in vec2 fragTexCoord;
    in vec3 fragNormal;

    uniform sampler2D texture0;
    uniform vec4 colDiffuse;
    uniform vec3 lightPos;

    out vec4 finalColor;

    // --- 1. Math Helper ---
    vec2 hash2(vec2 p) {
        return fract(sin(vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)))) * 43758.5453);
    }

    // Fungsi Rotasi UV
    vec2 rotate(vec2 v, float angle) {
        float s = sin(angle);
        float c = cos(angle);
        return mat2(c, -s, s, c) * v;
    }

    // --- 2. Texture Bombing Logic ---
    vec4 textureStochastic(sampler2D tex, vec2 uv) {
        // Skewing coordinate space to create Triangle Grid (Simplex)
        vec2 skewUV = uv * mat2(1.0, 0.0, 0.5, 0.866);
        
        vec2 i = floor(skewUV);
        vec2 f = fract(skewUV);

        // Barycentric weight (biar blendingnya halus di perbatasan segitiga)
        float s = step(f.y, f.x); 
        vec2 odd = vec2(s, 1.0 - s); 
        
        vec3 w = max(0.5 - vec3(dot(f,f), dot(f-1.0, f-1.0), dot(f-odd, f-odd)), 0.0);
        w = w*w*w; // Cubic smoothing

        // 3 Titik Sudut Segitiga
        vec2 v1 = i;
        vec2 v2 = i + 1.0;
        vec2 v3 = i + odd;

        // Hash Random buat tiap titik
        vec2 h1 = hash2(v1);
        vec2 h2 = hash2(v2);
        vec2 h3 = hash2(v3);

        // --- BAGIAN PENTING: ROTASI ---
        // Kita puter UV-nya berdasarkan angka random (h.x) dikali 2*PI (360 derajat)
        vec2 uv1 = rotate(uv, h1.x * 6.28) + h1;
        vec2 uv2 = rotate(uv, h2.x * 6.28) + h2;
        vec2 uv3 = rotate(uv, h3.x * 6.28) + h3;

        // Sampling texture yang sudah diputer-puter
        vec4 t1 = texture(tex, uv1);
        vec4 t2 = texture(tex, uv2);
        vec4 t3 = texture(tex, uv3);

        // Gabungin
        return (t1*w.x + t2*w.y + t3*w.z) / (w.x + w.y + w.z);
    }

    void main() {
        // Scaling diperkecil dikit (8.0) biar detailnya lebih kelihatan
        vec2 uv = fragTexCoord * 8.0; 
        
        vec4 texelColor = textureStochastic(texture0, uv);
        
        // --- Lighting Standard ---
        float ambientStrength = 0.3;
        vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);
        vec3 norm = normalize(fragNormal);
        vec3 lightDir = normalize(lightPos - fragPosition);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * vec3(1.0, 1.0, 0.9);

        vec3 result = (ambient + diffuse) * colDiffuse.rgb * texelColor.rgb;
        finalColor = vec4(result, colDiffuse.a * texelColor.a);
    }
)";

inline const char* FS_SLIME_CODE = R"(
    #version 330
    in vec3 fragPosition;
    in vec3 fragNormal;

    uniform vec4 colDiffuse; 
    uniform vec3 lightPos;
    uniform vec3 viewPos;

    out vec4 finalColor;

    void main() {
        vec3 norm = normalize(fragNormal);
        vec3 viewDir = normalize(viewPos - fragPosition);

        // --- 1. FRESNEL (RIM LIGHT) ---
        float dotNV = max(dot(norm, viewDir), 0.0);
        float fresnel = 1.0 - dotNV;
        fresnel = pow(fresnel, 4.0); // Tajam di pinggir

        // --- 2. BODY COLOR (KABUT) ---
        // Trik: Campur warna dasar dengan sedikit putih (0.2) biar jadi 'Milky/Cloudy'
        // Ini biar box di dalem gak kelihatan HD banget
        vec3 foggyBody = mix(colDiffuse.rgb, vec3(0.8, 0.9, 1.0), 0.3); 
        
        // Rim Color (Putih terang)
        vec3 rimColor = vec3(1.0, 1.0, 1.0) * fresnel * 2.0;

        // Gabungan
        vec3 finalRGB = foggyBody + rimColor;

        // --- 3. ALPHA (LOGIC BARU) ---
        // Base Alpha dinaikin jadi 0.4 (40% solid di tengah)
        // Biar box di dalem ketutup lapisan warna slime
        float alpha = 0.4 + (fresnel * 0.6); 

        finalColor = vec4(finalRGB, min(alpha, 1.0));
    }
)";