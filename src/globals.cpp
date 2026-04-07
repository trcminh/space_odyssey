// ============================================================
//  globals.cpp — Định nghĩa thực sự của các biến toàn cục
//
//  File này chỉ có 1 mục đích: "tạo ra" các biến toàn cục.
//  globals.h chỉ "thông báo" chúng tồn tại (extern),
//  còn globals.cpp mới là nơi bộ nhớ thực sự được cấp phát.
// ============================================================
#include "globals.h"

// Cửa sổ và renderer
SDL_Window*   window   = nullptr;
SDL_Renderer* renderer = nullptr;
TTF_Font*     font     = nullptr;

// Textures tàu người chơi
SDL_Texture* texPlayer[4]   = { nullptr, nullptr, nullptr, nullptr };
SDL_Texture* texEngineFlame = nullptr;

// Textures đạn người chơi
SDL_Texture* texWeaponAuto   = nullptr;
SDL_Texture* texWeaponRocket = nullptr;

// Textures icon vật phẩm
SDL_Texture* texIconAuto   = nullptr;
SDL_Texture* texIconRocket = nullptr;
SDL_Texture* texIconShield = nullptr;

// Textures khác
SDL_Texture* texPlayerShield = nullptr;
SDL_Texture* texScore50      = nullptr;
SDL_Texture* texScore70      = nullptr;
SDL_Texture* texScore100     = nullptr;
SDL_Texture* texEnemyBullet  = nullptr;

// Nhạc nền
Mix_Music* bgmMusic = nullptr;

SDL_Texture* texEnemySmallBase  = nullptr;
SDL_Texture* texEnemySmallDest  = nullptr;
SDL_Texture* texEnemyMediumBase = nullptr;
SDL_Texture* texEnemyMediumDest = nullptr;
SDL_Texture* texEnemyLargeBase  = nullptr;
SDL_Texture* texEnemyLargeDest  = nullptr;

// Danh sách entities đang hoạt động
std::vector<Bullet>       bullets;
std::vector<Enemy>        enemies;
std::vector<Pickup>       pickups;
std::vector<FloatingText> floating_texts;
Player                    player;

// Trạng thái game
GameState    currentState    = GameState::MENU;
int          currentStage    = 1;
int          currentWave     = 1;
int          enemyKillCount  = 0;
int          waveEnemyTarget = 0;
int          spawnTimer      = 0;
int          spawnedSmall    = 0;
int          spawnedMedium   = 0;
Uint32       transitionStart = 0;
int          highScore       = 0;
const Uint8* keyboardState   = nullptr;
