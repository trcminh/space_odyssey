#ifndef TYPES_H
#define TYPES_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <vector>
#include <string>
#include <cmath>
#include <ctime>
#include <cstdlib>

// ============================================================
//  CẤU HÌNH MÀN HÌNH
// ============================================================
const int SCREEN_WIDTH  = 640;
const int SCREEN_HEIGHT = 828; // Tỉ lệ dọc chuẩn Shoot-em-up (~9:16)

// ============================================================
//  HẰNG SỐ CẤU HÌNH GAMEPLAY
//  Tập trung tại 1 chỗ để dễ chỉnh khi cần
// ============================================================
const int    PLAYER_SIZE    = 64;       // Kích thước vẽ tàu player (px)
const float  PLAYER_SPEED   = 6.0f;    // Tốc độ di chuyển player (px/frame)
const float  ENEMY_SPEED    = 1.5f;    // Tốc độ rơi quái (px/frame)
const float  PICKUP_SPEED   = 2.0f;    // Tốc độ rơi vật phẩm
const int    PICKUP_SIZE    = 32;       // Kích thước vẽ vật phẩm
const Uint32 INVULN_TIME    = 3000;    // Thời gian bất tử sau hồi sinh (ms)
const Uint32 SHIELD_TIME    = 10000;   // Thời gian khiên tồn tại (ms)
const Uint32 RESPAWN_DELAY  = 500;     // Chờ trước khi hồi sinh (ms)
const int    INITIAL_LIVES  = 3;
const bool   GOD_MODE       = false;   // true = bất tử (để test), đổi false khi chơi thật

const float  OFFSCREEN_MARGIN     = 100.0f;
const int    SCORE_PICKUP         = 100;
const int    SCORE_ENEMY_SMALL    = 50;
const int    SCORE_ENEMY_MEDIUM   = 70;
const int    SCORE_ENEMY_LARGE    = 500;

// ============================================================
//  CÁC ENUM TRẠNG THÁI
// ============================================================

// Trạng thái tổng thể của game loop
enum class GameState {
    MENU,              // Màn hình chính
    PLAYING,           // Đang chơi (wave 1–15)
    GAMEOVER,          // Hết mạng
    GAME_WON           // Phá đảo xong Stage 3
};

enum class PickupType  { WEAPON, SHIELD };
enum class WeaponType  { AUTO_CANNON, ROCKET };
enum class EnemyState  { ALIVE, EXPLODING };
enum class EnemySize   { SMALL, MEDIUM, LARGE };

// Kiểu pattern đạn — dùng cho hàm fireEnemyBullets()
enum class AttackType  { AIMED, N_WAY, BURST_360 };


//  HELPER: TRẠNG THÁI HOẠT ẢNH (dùng lại cho nhiều struct)
//  Bất kỳ object nào có sprite sheet đều nhúng struct này vào.
// ============================================================
struct AnimState {
    int    current_frame  = 0;
    int    total_frames   = 1;
    Uint32 last_anim_time = 0;
};

// ============================================================
//  THỰC THỂ: CHỮ NỔI ĐIỂM SỐ
// ============================================================
struct FloatingText {
    float x, y;
    int   score;
    float alpha;   // Mờ dần từ 255 → 0
    bool  active;
};

// ============================================================
//  THỰC THỂ: NGƯỜI CHƠI (MAIN SHIP)
// ============================================================
struct Player {
    float x, y;
    int   lives;
    int   score;

    // Trạng thái sống / chết / hồi sinh
    bool   is_dead;
    bool   is_respawning;
    bool   is_invulnerable;
    Uint32 invuln_start_time;
    Uint32 death_time;

    // Vũ khí
    WeaponType current_weapon;
    int        weapon_level;   // 1–3
    Uint32     last_shoot_time;

    // Hitbox nhỏ ở giữa thân tàu (giảm độ khó cho người chơi)
    float hitbox_x, hitbox_y;
    int   hitbox_w, hitbox_h;

    // Khiên năng lượng
    bool      has_shield;
    Uint32    shield_start_time;
    AnimState shield_anim;
    int       shield_w, shield_h; // Kích thước 1 frame của shield sprite

    // Lửa động cơ đuôi tàu
    AnimState engine_anim;
};



// ============================================================
//  THỰC THỂ: ĐẠN
// ============================================================
struct Bullet {
    float x, y;
    float vx, vy;
    bool  active;
    bool  is_player_bullet;
    int   damage;
    float scale;           // Phóng to đạn Big Gun / Zapper theo cấp độ
    SDL_Texture* texture;
    int   w, h;            // Kích thước 1 frame (dùng cho hitbox & src rect)
    AnimState anim;
};

// ============================================================
//  THỰC THỂ: VẬT PHẨM RƠI (PICKUP)
// ============================================================
struct Pickup {
    float x, y;
    float vy;
    PickupType   type;
    WeaponType   weapon_type;   // Chỉ có ý nghĩa khi type == WEAPON
    SDL_Texture* texture;
    bool         active;
    AnimState    anim;
    int          w, h;          // Kích thước 1 frame
};

// ============================================================
//  THỰC THỂ: KẺ ĐỊCH (ENEMY)
// ============================================================
struct Enemy {
    float x, y;
    int   hp, max_hp;
    bool  active;
    EnemySize   size_class;
    EnemyState  state;
    Uint32      shoot_delay;
    Uint32      last_shoot_time;
    int         score_value;
    int         drop_chance;
    AnimState   anim;            // Dùng cho chuỗi nổ (destruction)
    SDL_Texture* texture_base;
    SDL_Texture* texture_destruction;
    SDL_Texture* texture_bullet;
    int w, h;

    // === DI CHUYỂN ===
    float        start_x          = 0;    // Vị trí X lúc mới spawn (dùng cho sine wave)
    float        start_y          = 0;    // Vị trí Y lúc mới spawn
    Uint32       spawn_time       = 0;    // Thời điểm spawn (ms) — dùng tính quỹ đạo
    float        target_y         = 150;  // Vị trí Y mà quái dừng lại
    float        vx               = 0;    // Vận tốc X (Kamikaze / thoát)
    float        vy_custom        = 0;    // Vận tốc Y tùy chỉnh (Kamikaze / thoát)
    int          move_phase       = 0;    // Pha: 0=bay vào, 1=đang bắn
    Uint32       phase_start_time = 0;    // Thời điểm bắt đầu pha hiện tại
};

#endif
