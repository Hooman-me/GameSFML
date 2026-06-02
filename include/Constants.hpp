#pragma once

// ============================================================
//  CONSTANTS.HPP  –  Semua nilai yang bisa kamu ubah ada di sini
// ============================================================

// --- Window ---
constexpr int   WINDOW_WIDTH        = 1060;
constexpr int   WINDOW_HEIGHT       = 640;
constexpr int   UI_BAR_HEIGHT       = 110;   // tinggi panel bawah
constexpr int   GAME_AREA_HEIGHT    = WINDOW_HEIGHT - UI_BAR_HEIGHT; // 530

// --- Gameplay Global ---
constexpr int   BASE_MAX_HP         = 10;    // *** HP base player ***
constexpr int   MAX_TROOPS          = 6;     // *** maks troops di-deploy sekaligus ***
constexpr float MONEY_TICK          = 1.0f;  // detik per +1 uang otomatis
constexpr int   MONEY_START         = 10;    // uang awal

// --- Wave / Enemy Spawn ---
constexpr int   WAVE_ENEMY_COUNT    = 20;    // *** jumlah musuh per wave ***
constexpr float ENEMY_SPAWN_INTERVAL= 1.5f;  // detik antar-spawn musuh
constexpr int   MONEY_PER_KILL      = 2;     // *** uang dapat saat musuh mati ***

// --- Enemy Stats ---  ← UBAH DI SINI
constexpr float ENEMY_HP            = 50.f;  // *** HP musuh ***
constexpr float ENEMY_SPEED         = 80.f;  // *** pixel/detik musuh bergerak ***
constexpr float ENEMY_DEFENSE       = 5.f;   // *** defense flat musuh (Arknights-style) ***
constexpr float ENEMY_WAYPOINT_DIST = 2.f;   // jarak threshold ganti waypoint

// --- Troop: "Archer Fox" Stats ---  ← UBAH DI SINI
constexpr float TROOP_ATK           = 15.f;  // *** attack damage ***
constexpr float TROOP_ATTACK_RANGE  = 160.f; // *** radius serang dalam pixel ***
constexpr float TROOP_FOV_DEG       = 90.f;  // *** sudut kerucut FOV (derajat) ***
constexpr float TROOP_ATTACK_SPEED  = 1.2f;  // *** serangan per detik ***
constexpr int   TROOP_COST          = 8;     // *** harga beli troops ***
constexpr float TROOP_HP            = 1.f;   // troops tidak punya HP (Arknights: sudah di-deploy statis)

// --- Projectile ---
constexpr float PROJ_SPEED          = 320.f; // *** kecepatan peluru pixel/detik ***
constexpr float PROJ_SIZE           = 8.f;   // radius visual peluru
constexpr float EXPLODE_DURATION    = 0.25f; // detik animasi ledakan

// --- Sprite Sheet (sesuaikan dengan asset-mu) ---
// Jika kamu tidak punya spritesheet nyata, game memakai placeholder warna.
// Isi path ini jika sudah punya asset:
//   TROOP_SPRITE  → spritesheet troops, baris=arah(Down/Left/Right/Up), kolom=frame
//   ENEMY_SPRITE  → spritesheet musuh
constexpr const char* TROOP_SPRITE_PATH  = "assets/sprites/troop.png";
constexpr const char* ENEMY_SPRITE_PATH  = "assets/sprites/enemy.png";
constexpr const char* FONT_PATH          = "assets/fonts/font.ttf";

// Ukuran satu frame di spritesheet (pixel)
constexpr int SPRITE_FRAME_W   = 48;
constexpr int SPRITE_FRAME_H   = 48;
constexpr int SPRITE_FRAMES    = 4;   // jumlah kolom (frame animasi)
// Baris spritesheet: 0=Down, 1=Left, 2=Right, 3=Up
constexpr int SPRITE_ROW_DOWN  = 0;
constexpr int SPRITE_ROW_LEFT  = 1;
constexpr int SPRITE_ROW_RIGHT = 2;
constexpr int SPRITE_ROW_UP    = 3;
constexpr float ANIM_FRAME_TIME= 0.15f; // detik per frame animasi

// --- HP Bar visual ---
constexpr float HPBAR_WIDTH    = 40.f;
constexpr float HPBAR_HEIGHT   = 5.f;
constexpr float HPBAR_OFFSET_Y = 26.f; // jarak bawah sprite ke bar