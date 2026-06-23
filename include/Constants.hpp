#pragma once
// ============================================================
//  CONSTANTS.HPP
// ============================================================
constexpr int   WINDOW_WIDTH        = 1060;
constexpr int   WINDOW_HEIGHT       = 640;
constexpr int   UI_BAR_HEIGHT       = 112;
constexpr int   GAME_AREA_HEIGHT    = 528;
 
constexpr int   BASE_MAX_HP         = 10;
constexpr int   MAX_TROOPS          = 6;
constexpr float MONEY_TICK          = 1.0f;
constexpr int   MONEY_START         = 10;
 
constexpr int   WAVE_ENEMY_COUNT    = 20;
// *** DIUBAH: Spawn interval diperbesar untuk spacing yang lebih lebar antar slime ***
constexpr float ENEMY_SPAWN_INTERVAL= 2.0f;  // Was 1.5f, now 2.0f untuk spacing
constexpr int   MONEY_PER_KILL      = 2;
 
// *** ENEMY SPEED diperlambat ke "level 1" ***
constexpr float ENEMY_HP            = 80.f;
constexpr float ENEMY_SPEED         = 35.f;
constexpr float ENEMY_DEFENSE       = 3.f;
constexpr float ENEMY_WAYPOINT_DIST = 3.f;
 
// GiantSlime sprite: 62x52px per frame
constexpr int   SLIME_FRAME_W       = 62;
constexpr int   SLIME_FRAME_H       = 52;
constexpr int   SLIME_IDLE_FRAMES   = 5;
constexpr int   SLIME_HURT_FRAMES   = 5;
constexpr int   SLIME_WALK_FRAMES   = 13;
constexpr float SLIME_ANIM_TIME     = 0.12f;
 
// CatBlaze: 48x48px
constexpr float TROOP_ATK           = 15.f;
constexpr float TROOP_ATTACK_RANGE  = 180.f;
constexpr float TROOP_FOV_DEG       = 90.f;
constexpr float TROOP_ATTACK_SPEED  = 1.2f;
constexpr int   TROOP_COST          = 8;
constexpr int   CAT_FRAME_W         = 48;
constexpr int   CAT_FRAME_H         = 48;
constexpr int   CAT_IDLE_FRAMES     = 4;
constexpr int   CAT_ATTACK_FRAMES   = 4;
constexpr int   CAT_HURT_FRAMES     = 2;
constexpr int   CAT_DEATH_FRAMES    = 4;
constexpr float CAT_ANIM_TIME       = 0.15f;
constexpr float CAT_ATTACK_ANIM_TIME= 0.08f;
 
// Fireball: 16x16px x 4 frames
constexpr float PROJ_SPEED          = 300.f;
constexpr float EXPLODE_DURATION    = 0.3f;
constexpr int   FIREBALL_FRAME_W    = 16;
constexpr int   FIREBALL_FRAME_H    = 16;
constexpr int   FIREBALL_FRAMES     = 4;
constexpr float FIREBALL_ANIM_TIME  = 0.08f;
constexpr float FIREBALL_SCALE      = 2.5f;
 
// HP Bar
constexpr float HPBAR_WIDTH         = 44.f;
constexpr float HPBAR_HEIGHT        = 5.f;
constexpr float HPBAR_OFFSET_Y      = 28.f;
 
// Asset Paths
constexpr const char* CAT_IDLE_PATH    = "assets/CatBlaze/Idle.png";
constexpr const char* CAT_ATTACK_PATH  = "assets/CatBlaze/Attack.png";
constexpr const char* CAT_HURT_PATH    = "assets/CatBlaze/Hurt.png";
constexpr const char* CAT_DEATH_PATH   = "assets/CatBlaze/Death.png";
constexpr const char* SLIME_IDLE_PATH  = "assets/Ninja Pack/Actor/Boss/GiantSlime/Idle.png";
constexpr const char* SLIME_HURT_PATH  = "assets/Ninja Pack/Actor/Boss/GiantSlime/Hit.png";
constexpr const char* SLIME_WALK_PATH  = "assets/Ninja Pack/Actor/Boss/GiantSlime/Jump.png";
constexpr const char* FIREBALL_PATH    = "assets/Ninja Pack/FX/Projectile/Fireball.png";
constexpr const char* MAP_TMJ_PATH     = "assets/Map/Map 1.2.tmj";
constexpr const char* TILESET_FLOOR_PATH  = "assets/Ninja Pack/Backgrounds/Tilesets/TilesetFloor.png";
constexpr const char* TILESET_WATER_PATH  = "assets/Ninja Pack/Backgrounds/Tilesets/TilesetWater.png";
constexpr const char* TILESET_NATURE_PATH = "assets/Ninja Pack/Backgrounds/Tilesets/TilesetNature.png";
constexpr const char* TILESET_HOUSE_PATH  = "assets/Ninja Pack/Backgrounds/Tilesets/TilesetHouse.png";
constexpr const char* FONT_REAL_PATH   = "assets/font.ttf";
 
// Base (rumah) posisi di map: house tile cols 54-57 rows 11-15
constexpr float BASE_CENTER_X = 896.f;
constexpr float BASE_CENTER_Y = 216.f;
constexpr float BASE_SIZE_W   = 80.f;
constexpr float BASE_SIZE_H   = 96.f;