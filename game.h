#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <vector>
#include <string>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>

// Hằng số cấu hình
const int SCREEN_WIDTH  = 640;
const int SCREEN_HEIGHT = 828;
const int    PLAYER_SIZE   = 64;
const float  PLAYER_SPEED  = 6.0f;
const float  ENEMY_SPEED   = 1.5f;
const float  PICKUP_SPEED  = 2.0f;
const int    PICKUP_SIZE   = 32;
const Uint32 INVULN_TIME   = 3000;
const Uint32 SHIELD_TIME   = 10000;
const Uint32 RESPAWN_DELAY = 500;
const int    INITIAL_LIVES = 3;
const bool   GOD_MODE      = false;
const float  OFFSCREEN_MARGIN   = 100.0f;
const int    SCORE_PICKUP       = 100;
const int    SCORE_ENEMY_SMALL  = 50;
const int    SCORE_ENEMY_MEDIUM = 70;
const int    SCORE_ENEMY_LARGE  = 500;

// Enum
enum class GameState  { MENU, PLAYING, PAUSED, GAMEOVER, GAME_WON };
enum class PickupType { WEAPON, SHIELD };
enum class WeaponType { AUTO_CANNON, ROCKET };
enum class EnemyState { ALIVE, EXPLODING };
enum class EnemySize  { SMALL, MEDIUM, LARGE };
enum class AttackType { AIMED, N_WAY, BURST_360 };

// Struct
struct AnimState {
    int    current_frame  = 0;
    int    total_frames   = 1;
    Uint32 last_anim_time = 0;
};

struct FloatingText {
    float x, y;
    int   score;
    float alpha;
    bool  active;
};

struct Player {
    float x, y;
    int   lives, score;
    bool   is_dead;
    bool   is_invulnerable;
    Uint32 invuln_start_time;
    Uint32 death_time;
    WeaponType current_weapon;
    int        weapon_level;
    Uint32     last_shoot_time;
    bool      has_shield;
    Uint32    shield_start_time;
    AnimState shield_anim;
};

struct Bullet {
    float x, y, vx, vy;
    bool  active, is_player_bullet;
    int   damage;
    float scale;
    SDL_Texture* texture;
    int   w, h;
    AnimState anim;
};

struct Pickup {
    float x, y, vy;
    PickupType   type;
    WeaponType   weapon_type;
    SDL_Texture* texture;
    bool         active;
    AnimState    anim;
    int          w, h;
};

struct Enemy {
    float x, y;
    int   hp, max_hp;
    bool  active;
    EnemySize   size_class;
    EnemyState  state;
    Uint32      shoot_delay, last_shoot_time;
    int         score_value, drop_chance;
    AnimState   anim;
    SDL_Texture* texture_base;
    SDL_Texture* texture_destruction;
    SDL_Texture* texture_bullet;
    int w, h;
    float target_y   = 150;
    int   move_phase = 0;
};

// Biến toàn cục
extern SDL_Window*   window;
extern SDL_Renderer* renderer;
extern TTF_Font*     font;
extern SDL_Texture* texPlayer[4];
extern SDL_Texture* texWeaponAuto;
extern SDL_Texture* texWeaponRocket;
extern SDL_Texture* texIconAuto;
extern SDL_Texture* texIconRocket;
extern SDL_Texture* texIconShield;
extern SDL_Texture* texPlayerShield;
extern SDL_Texture* texEnemyBullet;
extern SDL_Texture* texEnemySmallBase;
extern SDL_Texture* texEnemySmallDest;
extern SDL_Texture* texEnemyMediumBase;
extern SDL_Texture* texEnemyMediumDest;
extern SDL_Texture* texEnemyLargeBase;
extern SDL_Texture* texEnemyLargeDest;
extern SDL_Texture* texBackground;
extern Mix_Music*   bgmMusic;
extern std::vector<Bullet>       bullets;
extern std::vector<Enemy>        enemies;
extern std::vector<Pickup>       pickups;
extern std::vector<FloatingText> floating_texts;
extern Player                    player;
extern GameState    currentState;
extern int          currentStage, currentWave;
extern int          enemyKillCount, waveEnemyTarget;
extern int          spawnTimer, spawnedSmall, spawnedMedium, spawnedLarge;
extern Uint32       transitionStart;
extern int          highScore;
extern const Uint8* keyboardState;
extern float        bgScrollY;

// game.cpp
SDL_Texture* loadTexture(const std::string& path);
SDL_Texture* loadText(const std::string& text, SDL_Color color);
bool loadFiles();

// main.cpp
bool updateAnim(AnimState& anim, Uint32 now, Uint32 interval);
int countFrames(SDL_Texture* tex);
Bullet createBullet(SDL_Texture* tex, float x, float y,
                    float vx, float vy, int damage, bool isPlayer);
void updateBullets();
void updatePickups();
void updateFloatingTexts();
void cleanupVectors();

// player.cpp
void updatePlayer();
void damagePlayer();
void firePlayerWeapon(float cx, float y, WeaponType wpn, int lvl);
void checkPlayerCollisions();

// enemy.cpp
void spawnEnemy(int sizeIdx);
void updateSpawner();
void updateEnemies();
void handleEnemyDeath(Enemy& e);
void fireEnemyBullets(float cx, float cy, SDL_Texture* tex,
                      AttackType type, float speed);
void checkBulletEnemyCollisions();
bool AABB(float x1, float y1, float w1, float h1,
          float x2, float y2, float w2, float h2);

// render.cpp
void render();

#endif
