#include "game.h"

// Biến toàn cục
SDL_Window*   window   = nullptr;
SDL_Renderer* renderer = nullptr;
TTF_Font*     font     = nullptr;
SDL_Texture* texPlayer[4] = {};
SDL_Texture* texWeaponAuto   = nullptr;
SDL_Texture* texWeaponRocket = nullptr;
SDL_Texture* texIconAuto   = nullptr;
SDL_Texture* texIconRocket = nullptr;
SDL_Texture* texIconShield = nullptr;
SDL_Texture* texPlayerShield = nullptr;
SDL_Texture* texEnemyBullet = nullptr;
SDL_Texture* texEnemySmallBase  = nullptr;
SDL_Texture* texEnemySmallDest  = nullptr;
SDL_Texture* texEnemyMediumBase = nullptr;
SDL_Texture* texEnemyMediumDest = nullptr;
SDL_Texture* texEnemyLargeBase  = nullptr;
SDL_Texture* texEnemyLargeDest  = nullptr;
SDL_Texture* texBackground = nullptr;
Mix_Music*   bgmMusic = nullptr;
std::vector<Bullet>       bullets;
std::vector<Enemy>        enemies;
std::vector<Pickup>       pickups;
std::vector<FloatingText> floating_texts;
Player                    player;
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
float        bgScrollY       = 0;

// Tải ảnh PNG vào VRAM
SDL_Texture* loadTexture(const std::string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) return nullptr;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

// Render chuỗi text thành SDL_Texture
SDL_Texture* loadText(const std::string& text, SDL_Color color) {
    if (!font) return nullptr;
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!surface) return nullptr;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

// Tải toàn bộ asset khi khởi động
bool loadFiles() {
    font = TTF_OpenFont("asset/font.TTF", 24);
    if (!font) return false;

    texPlayer[0] = loadTexture("asset/MainShip/Main Ship/Main Ship - Bases/PNGs/Main Ship - Base - Very damaged.png");
    texPlayer[1] = loadTexture("asset/MainShip/Main Ship/Main Ship - Bases/PNGs/Main Ship - Base - Damaged.png");
    texPlayer[2] = loadTexture("asset/MainShip/Main Ship/Main Ship - Bases/PNGs/Main Ship - Base - Slight damage.png");
    texPlayer[3] = loadTexture("asset/MainShip/Main Ship/Main Ship - Bases/PNGs/Main Ship - Base - Full health.png");
    texWeaponAuto   = loadTexture("asset/MainShip/Main ship weapons/PNGs/Main ship weapon - Projectile - Auto cannon bullet.png");
    texWeaponRocket = loadTexture("asset/MainShip/Main ship weapons/PNGs/Main ship weapon - Projectile - Rocket.png");
    texIconAuto   = loadTexture("asset/PickupsPack/Weapons/PNGs/Pickup Icon - Weapons - Auto Cannons.png");
    texIconRocket = loadTexture("asset/PickupsPack/Weapons/PNGs/Pickup Icon - Weapons - Rocket.png");
    texIconShield = loadTexture("asset/PickupsPack/Shield Generators/PNGs/Pickup Icon - Shield Generator - All around shield.png");
    texPlayerShield = loadTexture("asset/MainShip/Main Ship/Main Ship - Shields/PNGs/Main Ship - Shields - Round Shield.png");
    texEnemyBullet     = loadTexture("asset/EnemyFleet_1/Kla'ed/Projectiles/PNGs/Kla'ed - Bullet.png");
    texEnemySmallBase  = loadTexture("asset/EnemyFleet_1/Kla'ed/Base/PNGs/Kla'ed - Scout - Base.png");
    texEnemySmallDest  = loadTexture("asset/EnemyFleet_1/Kla'ed/Destruction/PNGs/Kla'ed - Scout - Destruction.png");
    texEnemyMediumBase = loadTexture("asset/EnemyFleet_1/Kla'ed/Base/PNGs/Kla'ed - Frigate - Base.png");
    texEnemyMediumDest = loadTexture("asset/EnemyFleet_1/Kla'ed/Destruction/PNGs/Kla'ed - Frigate - Destruction.png");
    texEnemyLargeBase  = loadTexture("asset/EnemyFleet_1/Kla'ed/Base/PNGs/Kla'ed - Dreadnought - Base.png");
    texEnemyLargeDest  = loadTexture("asset/EnemyFleet_1/Kla'ed/Destruction/PNGs/Kla'ed - Dreadnought - Destruction.png");
    texBackground = loadTexture("asset/bgr.png");
    bgmMusic = Mix_LoadMUS("bgm/bgm.mp3");
    return true;
}
