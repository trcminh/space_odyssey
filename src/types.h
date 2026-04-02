#ifndef TYPES_H
#define TYPES_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
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
//  CÁC ENUM TRẠNG THÁI
// ============================================================
enum class GameState  { MENU, PLAYING, GAMEOVER };
enum class PickupType { WEAPON, SHIELD };
enum class WeaponType { AUTO_CANNON, BIG_SPACE_GUN, ROCKET, ZAPPER };
enum class EnemyState { ALIVE, EXPLODING };
enum class EnemyFleet { KLAED, NAIRAN, NAUTOLAN };
enum class EnemySize  { SMALL, MEDIUM, LARGE };

// ============================================================
//  HELPER: TRẠNG THÁI HOẠT ẢNH (dùng lại cho nhiều struct)
//  Bất kỳ object nào có sprite sheet đều nhúng struct này vào.
// ============================================================
struct AnimState {
    int      current_frame  = 0;
    int      total_frames   = 1;
    Uint32   last_anim_time = 0;
};

// ============================================================
//  CATALOG: ĐỊNH NGHĨA TĨNH CỦA MỖI LOẠI QUÁI
//  Nạp 1 lần khi khởi động, dùng để clone ra Enemy khi spawn.
// ============================================================
struct EnemyDef {
    SDL_Texture* base;         // Sprite tàu sống
    SDL_Texture* dest;         // Sprite chuỗi nổ
    SDL_Texture* bullet;       // Đạn của hạm đội này
    EnemyFleet   fleet;
    EnemySize    size_class;
    int          hp;
    Uint32       shoot_delay;  // Khoảng cách giữa 2 lần bắn (ms)
    int          score_value;  // Điểm nhận khi tiêu diệt
    int          drop_chance;  // % cơ hội rơi vật phẩm (0–100)
    int          w, h;         // Kích thước sprite (query trước 1 lần)
};

// ============================================================
//  THỰC THỂ: CHỮ NỔI ĐIỂM SỐ
// ============================================================
struct FloatingText {
    float  x, y;
    int    score;
    float  alpha;  // Mờ dần từ 255 → 0
    bool   active;
};

// ============================================================
//  THỰC THỂ: NGƯỜI CHƠI (MAIN SHIP)
// ============================================================
struct Player {
    float  x, y;
    int    lives;
    int    score;

    // Trạng thái sống/chết/hồi sinh
    bool   is_dead;
    bool   is_respawning;
    bool   is_invulnerable;
    Uint32 invuln_start_time;
    Uint32 death_time;

    // Vũ khí
    WeaponType current_weapon;
    int        weapon_level;   // 1–3
    Uint32     last_shoot_time;

    // Hitbox nhỏ ở giữa thân tàu (giảm khó khăn cho người chơi)
    float  hitbox_x, hitbox_y;
    int    hitbox_w, hitbox_h;

    // Khiên năng lượng
    bool       has_shield;
    Uint32     shield_start_time;
    AnimState  shield_anim;
    int        shield_w, shield_h; // Kích thước 1 frame của shield sprite

    // Lửa động cơ đuôi tàu
    AnimState  engine_anim;
};

// ============================================================
//  THỰC THỂ: NGÔI SAO NỀN (PARALLAX)
// ============================================================
struct Star {
    float x, y;
    float speed;
    int   size;
};

// ============================================================
//  THỰC THỂ: ĐẠN
// ============================================================
struct Bullet {
    float      x, y;
    float      vx, vy;
    bool       active;
    bool       is_player_bullet;
    int        damage;
    float      scale;          // Phóng to đạn Big Gun / Zapper theo cấp độ
    SDL_Texture* texture;
    int        w, h;           // Kích thước 1 frame (dùng cho hitbox & src rect)
    AnimState  anim;
};

// ============================================================
//  THỰC THỂ: VẬT PHẨM RƠI (PICKUP)
// ============================================================
struct Pickup {
    float      x, y;
    float      vy;
    PickupType type;
    WeaponType weapon_type;    // Chỉ có ý nghĩa khi type == WEAPON
    SDL_Texture* texture;
    bool       active;
    AnimState  anim;
    int        w, h;           // Kích thước 1 frame
};

// ============================================================
//  THỰC THỂ: KẺ ĐỊCH (ENEMY)
// ============================================================
struct Enemy {
    float      x, y;
    int        hp, max_hp;
    bool       active;
    EnemyFleet fleet;
    EnemySize  size_class;
    EnemyState state;
    Uint32     shoot_delay;
    Uint32     last_shoot_time;
    int        score_value;
    int        drop_chance;
    AnimState  anim;           // Dùng cho chuỗi nổ (destruction)
    SDL_Texture* texture_base;
    SDL_Texture* texture_destruction;
    SDL_Texture* texture_bullet;
    int        w, h;
};

#endif
