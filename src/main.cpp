// ============================================================
//  SPACE ODYSSEY — main.cpp
//  Toàn bộ logic game nằm trong 1 file để dễ quản lý.
//  Cấu trúc: globals → helpers → init → update → render → main
// ============================================================
#include "types.h"
#include <iostream>
#include <algorithm>
#include <SDL2/SDL_mixer.h>

// ============================================================
//  GLOBALS: CỬA SỔ, RENDERER, FONT
// ============================================================
SDL_Window*   window   = nullptr;
SDL_Renderer* renderer = nullptr;
TTF_Font*     font     = nullptr;

// ============================================================
//  TEXTURES: TÀU + VŨ KHÍ + PICKUP
// ============================================================
SDL_Texture* texPlayer[4]    = { nullptr, nullptr, nullptr, nullptr };
SDL_Texture* texEngineFlame  = nullptr;

// Đạn của người chơi (sprite sheet, 1 frame / nhiều frame)
SDL_Texture* texWeaponAuto   = nullptr;
SDL_Texture* texWeaponBig    = nullptr;
SDL_Texture* texWeaponRocket = nullptr;
SDL_Texture* texWeaponZapper = nullptr;

// Icon vật phẩm rơi
SDL_Texture* texIconAuto     = nullptr;
SDL_Texture* texIconBig      = nullptr;
SDL_Texture* texIconRocket   = nullptr;
SDL_Texture* texIconZapper   = nullptr;
SDL_Texture* texIconShield   = nullptr;

// Vòng sáng khiên bao quanh tàu
SDL_Texture* texPlayerShield = nullptr;

// Texture điểm nổi (render sẵn 1 lần, dùng lại nhiều lần)
SDL_Texture* texScore50  = nullptr;
SDL_Texture* texScore70  = nullptr;
SDL_Texture* texScore100 = nullptr;

// Đạn kẻ địch theo từng hạm đội
SDL_Texture* texEnemyKlaedBullet    = nullptr;
SDL_Texture* texEnemyNairanBullet   = nullptr;
SDL_Texture* texEnemyNautolanBullet = nullptr;

// Nhạc nền
Mix_Music* bgmMusic = nullptr;

// ============================================================
//  CATALOG: THƯ VIỆN QUÁI (0=Nhỏ, 1=Vừa, 2=Lớn — khớp Enum)
// ============================================================
std::vector<EnemyDef> catalogKlaed;
std::vector<EnemyDef> catalogNairan;
std::vector<EnemyDef> catalogNautolan;

// ============================================================
//  DANH SÁCH THỰC THỂ ĐANG HOẠT ĐỘNG
// ============================================================
std::vector<Star>         stars;
std::vector<Bullet>       bullets;
std::vector<Enemy>        enemies;
std::vector<Pickup>       pickups;
std::vector<FloatingText> floating_texts;
Player player;

// ============================================================
//  BIẾN TRẠNG THÁI GAME
// ============================================================
GameState    currentState  = GameState::MENU;
int          currentWave   = 1;
int          spawnTimer    = 0;
int          spawnCooldown = 120; // Frame giữa 2 lần spawn quái
int          highScore     = 0;
const Uint8* keyboardState = nullptr;

// ============================================================
//  HELPERS: TẢI ẢNH / FONT
// ============================================================

// Tải ảnh PNG → SDL_Texture
SDL_Texture* loadTexture(const std::string& path) {
    SDL_Surface* s = IMG_Load(path.c_str());
    if (!s) return nullptr;
    SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
    SDL_FreeSurface(s);
    return t;
}

// Render chuỗi text → SDL_Texture (nhớ Destroy sau khi vẽ xong!)
SDL_Texture* loadText(const std::string& text, SDL_Color color) {
    if (!font) return nullptr;
    SDL_Surface* s = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!s) return nullptr;
    SDL_Texture* t = SDL_CreateTextureFromSurface(renderer, s);
    SDL_FreeSurface(s);
    return t;
}

// Vẽ text lên renderer tại (x, y), rồi destroy texture ngay.
// scale > 1 để phóng to (dùng cho tiêu đề).
void renderText(const std::string& text, SDL_Color color, int x, int y, float scale = 1.0f) {
    SDL_Texture* tex = loadText(text, color);
    if (!tex) return;
    int tw, th;
    SDL_QueryTexture(tex, NULL, NULL, &tw, &th);
    SDL_Rect r = { x, y, (int)(tw * scale), (int)(th * scale) };
    SDL_RenderCopy(renderer, tex, NULL, &r);
    SDL_DestroyTexture(tex);
}

// Vẽ text căn giữa theo chiều ngang tại dòng y.
void renderTextCentered(const std::string& text, SDL_Color color, int y, float scale = 1.0f) {
    SDL_Texture* tex = loadText(text, color);
    if (!tex) return;
    int tw, th;
    SDL_QueryTexture(tex, NULL, NULL, &tw, &th);
    int x = SCREEN_WIDTH / 2 - (int)(tw * scale) / 2;
    SDL_Rect r = { x, y, (int)(tw * scale), (int)(th * scale) };
    SDL_RenderCopy(renderer, tex, NULL, &r);
    SDL_DestroyTexture(tex);
}

// Tính số frame từ sprite sheet hình chữ nhật nằm ngang (frame vuông).
// Ví dụ: sheet 192×48 → frame 48×48 → 4 frames.
int countFrames(SDL_Texture* tex) {
    if (!tex) return 1;
    int w, h;
    SDL_QueryTexture(tex, NULL, NULL, &w, &h);
    int n = (h > 0) ? (w / h) : 1;
    return (n > 0) ? n : 1;
}

// Nạp dữ liệu 1 loại quái vào catalog.
void addEnemyDef(std::vector<EnemyDef>& cat,
                 EnemyFleet fleet, EnemySize size,
                 const char* basePath, const char* destPath,
                 SDL_Texture* bullet,
                 int hp, Uint32 shootDelay, int score, int dropChance)
{
    EnemyDef d;
    d.base        = loadTexture(basePath);
    d.dest        = loadTexture(destPath);
    d.bullet      = bullet;
    d.fleet       = fleet;
    d.size_class  = size;
    d.hp          = hp;
    d.shoot_delay = shootDelay;
    d.score_value = score;
    d.drop_chance = dropChance;
    d.w = 64; d.h = 64; // Fallback
    if (d.base) SDL_QueryTexture(d.base, NULL, NULL, &d.w, &d.h);
    cat.push_back(d);
}

// Vẽ hình tròn tô đặc (thuật toán Midpoint Circle).
// Hiện dùng để debug hitbox của player.
void drawFilledCircle(SDL_Renderer* rend, int cx, int cy, int r) {
    int x = r, y = 0, xErr = 1 - (r << 1), yErr = 0, rErr = 0;
    while (x >= y) {
        for (int i = cx - x; i <= cx + x; i++) {
            SDL_RenderDrawPoint(rend, i, cy + y);
            SDL_RenderDrawPoint(rend, i, cy - y);
        }
        for (int i = cx - y; i <= cx + y; i++) {
            SDL_RenderDrawPoint(rend, i, cy + x);
            SDL_RenderDrawPoint(rend, i, cy - x);
        }
        y++;
        rErr += yErr; yErr += 2;
        if (((rErr << 1) + xErr) > 0) { x--; rErr += xErr; xErr += 2; }
    }
}

// Tạo Bullet từ texture + thông số.
// Tự detect số frame từ sprite sheet (frame vuông nằm ngang).
Bullet createBullet(SDL_Texture* tex, float x, float y,
                    float vx, float vy, int damage, bool isPlayer)
{
    Bullet b;
    b.texture          = tex;
    b.x = x; b.y = y; b.vx = vx; b.vy = vy;
    b.damage           = damage;
    b.is_player_bullet = isPlayer;
    b.active           = true;
    b.scale            = 1.0f;
    b.w = 16; b.h = 16;
    b.anim = { 0, 1, SDL_GetTicks() };

    if (tex) {
        int bw, bh;
        SDL_QueryTexture(tex, NULL, NULL, &bw, &bh);
        // Mỗi frame là hình vuông bh×bh, ghép nằm ngang
        b.h             = bh;
        b.w             = bh;
        b.anim.total_frames = countFrames(tex);
    }
    return b;
}

// ============================================================
//  LOAD ASSETS: ĐỌC TẤT CẢ FILE ẢNH VÀ ÂM THANH
// ============================================================
bool loadFiles() {
    font = TTF_OpenFont("asset/font.TTF", 24);
    if (!font) return false;

    // Tàu người chơi (4 trạng thái)
    texPlayer[0]   = loadTexture("asset/MainShip/Main Ship/Main Ship - Bases/PNGs/Main Ship - Base - Very damaged.png");
    texPlayer[1]   = loadTexture("asset/MainShip/Main Ship/Main Ship - Bases/PNGs/Main Ship - Base - Damaged.png");
    texPlayer[2]   = loadTexture("asset/MainShip/Main Ship/Main Ship - Bases/PNGs/Main Ship - Base - Slight damage.png");
    texPlayer[3]   = loadTexture("asset/MainShip/Main Ship/Main Ship - Bases/PNGs/Main Ship - Base - Full health.png");
    texEngineFlame = loadTexture("asset/MainShip/Main Ship/Main Ship - Engine Effects/PNGs/Main Ship - Engines - Base Engine - Spritesheet.png");

    // Đạn người chơi
    texWeaponAuto   = loadTexture("asset/MainShip/Main ship weapons/PNGs/Main ship weapon - Projectile - Auto cannon bullet.png");
    texWeaponBig    = loadTexture("asset/MainShip/Main ship weapons/PNGs/Main ship weapon - Projectile - Big Space Gun.png");
    texWeaponRocket = loadTexture("asset/MainShip/Main ship weapons/PNGs/Main ship weapon - Projectile - Rocket.png");
    texWeaponZapper = loadTexture("asset/MainShip/Main ship weapons/PNGs/Main ship weapon - Projectile - Zapper.png");

    // Icon vật phẩm
    texIconAuto    = loadTexture("asset/PickupsPack/Weapons/PNGs/Pickup Icon - Weapons - Auto Cannons.png");
    texIconBig     = loadTexture("asset/PickupsPack/Weapons/PNGs/Pickup Icon - Weapons - Big Space Gun 2000.png");
    texIconRocket  = loadTexture("asset/PickupsPack/Weapons/PNGs/Pickup Icon - Weapons - Rocket.png");
    texIconZapper  = loadTexture("asset/PickupsPack/Weapons/PNGs/Pickup Icon - Weapons - Zapper.png");
    texIconShield  = loadTexture("asset/PickupsPack/Shield Generators/PNGs/Pickup Icon - Shield Generator - All around shield.png");
    texPlayerShield= loadTexture("asset/MainShip/Main Ship/Main Ship - Shields/PNGs/Main Ship - Shields - Round Shield.png");

    // Đạn kẻ địch theo hạm đội
    texEnemyKlaedBullet    = loadTexture("asset/EnemyFleet_1/Kla'ed/Projectiles/PNGs/Kla'ed - Bullet.png");
    texEnemyNairanBullet   = loadTexture("asset/EnemyFleet_2/Nairan/Projectiles/PNGs/Nairan - Bolt.png");
    texEnemyNautolanBullet = loadTexture("asset/EnemyFleet_3/Nautolan/Projectiles/PNGs/Nautolan - Bullet.png");

    // Catalog quái — thứ tự PHẢI là SMALL, MEDIUM, LARGE để khớp enum EnemySize
    // Kla'ed
    addEnemyDef(catalogKlaed, EnemyFleet::KLAED, EnemySize::SMALL,
        "asset/EnemyFleet_1/Kla'ed/Base/PNGs/Kla'ed - Scout - Base.png",
        "asset/EnemyFleet_1/Kla'ed/Destruction/PNGs/Kla'ed - Scout - Destruction.png",
        texEnemyKlaedBullet, 2, 2500, 50, 20);
    addEnemyDef(catalogKlaed, EnemyFleet::KLAED, EnemySize::MEDIUM,
        "asset/EnemyFleet_1/Kla'ed/Base/PNGs/Kla'ed - Frigate - Base.png",
        "asset/EnemyFleet_1/Kla'ed/Destruction/PNGs/Kla'ed - Frigate - Destruction.png",
        texEnemyKlaedBullet, 8, 2000, 70, 30);
    addEnemyDef(catalogKlaed, EnemyFleet::KLAED, EnemySize::LARGE,
        "asset/EnemyFleet_1/Kla'ed/Base/PNGs/Kla'ed - Dreadnought - Base.png",
        "asset/EnemyFleet_1/Kla'ed/Destruction/PNGs/Kla'ed - Dreadnought - Destruction.png",
        texEnemyKlaedBullet, 35, 1200, 100, 50);

    // Nairan
    addEnemyDef(catalogNairan, EnemyFleet::NAIRAN, EnemySize::SMALL,
        "asset/EnemyFleet_2/Nairan/Designs - Base/PNGs/Nairan - Fighter - Base.png",
        "asset/EnemyFleet_2/Nairan/Destruction/PNGs/Nairan - Fighter -  Destruction.png",
        texEnemyNairanBullet, 3, 2300, 50, 20);
    addEnemyDef(catalogNairan, EnemyFleet::NAIRAN, EnemySize::MEDIUM,
        "asset/EnemyFleet_2/Nairan/Designs - Base/PNGs/Nairan - Bomber - Base.png",
        "asset/EnemyFleet_2/Nairan/Destruction/PNGs/Nairan - Bomber -  Destruction.png",
        texEnemyNairanBullet, 12, 1800, 70, 30);
    addEnemyDef(catalogNairan, EnemyFleet::NAIRAN, EnemySize::LARGE,
        "asset/EnemyFleet_2/Nairan/Designs - Base/PNGs/Nairan - Battlecruiser - Base.png",
        "asset/EnemyFleet_2/Nairan/Destruction/PNGs/Nairan - Battlecruiser  -  Destruction.png",
        texEnemyNairanBullet, 40, 1500, 100, 50);

    // Nautolan
    addEnemyDef(catalogNautolan, EnemyFleet::NAUTOLAN, EnemySize::SMALL,
        "asset/EnemyFleet_3/Nautolan/Designs - Base/PNGs/Nautolan Ship - Scout - Base.png",
        "asset/EnemyFleet_3/Nautolan/Destruction/PNGs/Nautolan Ship - Scout.png",
        texEnemyNautolanBullet, 4, 1800, 50, 20);
    addEnemyDef(catalogNautolan, EnemyFleet::NAUTOLAN, EnemySize::MEDIUM,
        "asset/EnemyFleet_3/Nautolan/Designs - Base/PNGs/Nautolan Ship - Frigate - Base.png",
        "asset/EnemyFleet_3/Nautolan/Destruction/PNGs/Nautolan Ship - Frigate.png",
        texEnemyNautolanBullet, 15, 1500, 70, 30);
    addEnemyDef(catalogNautolan, EnemyFleet::NAUTOLAN, EnemySize::LARGE,
        "asset/EnemyFleet_3/Nautolan/Designs - Base/PNGs/Nautolan Ship - Dreadnought - Base.png",
        "asset/EnemyFleet_3/Nautolan/Destruction/PNGs/Nautolan Ship - Dreadnought.png",
        texEnemyNautolanBullet, 50, 1000, 100, 50);

    // Texture điểm nổi (pre-render 1 lần)
    texScore50  = loadText("+50",  {200, 255, 100, 255});
    texScore70  = loadText("+70",  {255, 100, 255, 255});
    texScore100 = loadText("+100", {255, 200,  50, 255});

    bgmMusic = Mix_LoadMUS("bgm/bgm.mp3");
    return true;
}

// ============================================================
//  KHỞI TẠO GAME
// ============================================================

void initPlayer() {
    player.x = SCREEN_WIDTH / 2 - 32;
    player.y = SCREEN_HEIGHT - 100;
    player.lives = 3;
    player.score = 0;

    player.is_dead = player.is_respawning = false;
    player.is_invulnerable  = true;
    player.invuln_start_time = SDL_GetTicks();

    player.current_weapon  = WeaponType::AUTO_CANNON;
    player.weapon_level    = 1;
    player.last_shoot_time = 0;

    // Hitbox 10×10 ở giữa ngực tàu
    player.hitbox_w = player.hitbox_h = 10;

    // Khiên
    player.has_shield = false;
    player.shield_start_time = 0;
    player.shield_w = player.shield_h = 64;
    player.shield_anim = { 0, 12, 0 };
    if (texPlayerShield) {
        SDL_QueryTexture(texPlayerShield, NULL, NULL, &player.shield_w, &player.shield_h);
        // Shield sprite cũng là frame vuông, nhiều frame ghép ngang
        int fw = player.shield_h; // chiều rộng 1 frame = chiều cao sheet
        player.shield_anim.total_frames = countFrames(texPlayerShield);
        player.shield_w = fw;
    }

    // Động cơ đuôi
    player.engine_anim = { 0, 4, 0 };
}

void initStars() {
    for (int i = 0; i < 100; i++) {
        Star s;
        s.x     = (float)(rand() % SCREEN_WIDTH);
        s.y     = (float)(rand() % SCREEN_HEIGHT);
        s.speed = (rand() % 4) + 1.0f;
        s.size  = (rand() % 3) + 1;
        stars.push_back(s);
    }
}

void restartGame() {
    initPlayer();
    bullets.clear();
    enemies.clear();
    pickups.clear();
    floating_texts.clear();
    currentWave = 1;
    spawnTimer = 0;
    spawnCooldown = 120;
    currentState = GameState::PLAYING;
    if (bgmMusic) Mix_PlayMusic(bgmMusic, -1); // -1 = loop vô hạn
}

void handlePlayerDeath() {
    Mix_HaltMusic();
    if (player.score > highScore) highScore = player.score;
    currentState = GameState::GAMEOVER;
}

// ============================================================
//  SPAWN QUÁI
// ============================================================
void spawnEnemy() {
    // Xác định cỡ quái theo wave hiện tại
    EnemySize sz  = EnemySize::SMALL;
    int       rng = rand() % 100;
    if (currentWave < 3) {
        if (rng > 80) sz = EnemySize::MEDIUM;
    } else if (currentWave < 6) {
        if (rng > 40) sz = EnemySize::MEDIUM;
    } else {
        if      (rng > 85) sz = EnemySize::LARGE;
        else if (rng > 40) sz = EnemySize::MEDIUM;
    }

    // Chọn ngẫu nhiên 1 trong 3 hạm đội
    const std::vector<EnemyDef>* catalogs[] = { &catalogKlaed, &catalogNairan, &catalogNautolan };
    const EnemyDef& d = (*catalogs[rand() % 3])[(int)sz];

    Enemy e;
    e.active         = true;
    e.state          = EnemyState::ALIVE;
    e.last_shoot_time = SDL_GetTicks();
    e.anim           = { 0, 0, 0 };

    e.fleet              = d.fleet;
    e.size_class         = d.size_class;
    e.texture_base       = d.base;
    e.texture_destruction = d.dest;
    e.texture_bullet     = d.bullet;
    e.hp = e.max_hp      = d.hp;
    e.shoot_delay        = d.shoot_delay;
    e.score_value        = d.score_value;
    e.drop_chance        = d.drop_chance;
    e.w = d.w; e.h = d.h;

    e.x = (float)(rand() % (SCREEN_WIDTH - e.w));
    e.y = (float)(-e.h); // Xuất phát từ trên màn hình

    enemies.push_back(e);
}

// ============================================================
//  UPDATE: CẬP NHẬT TRẠNG THÁI MỖI FRAME
// ============================================================

void updateStars() {
    for (auto& s : stars) {
        s.y += s.speed;
        if (s.y > SCREEN_HEIGHT) {
            s.y = 0;
            s.x = (float)(rand() % SCREEN_WIDTH);
        }
    }
}

void updateFloatingTexts() {
    for (auto& ft : floating_texts) {
        if (!ft.active) continue;
        ft.y    -= 1.0f;
        ft.alpha -= 5.0f;
        if (ft.alpha <= 0) ft.active = false;
    }
}

void updateBullets() {
    Uint32 now = SDL_GetTicks();
    for (auto& b : bullets) {
        if (!b.active) continue;
        b.x += b.vx;
        b.y += b.vy;

        // Đạn player tăng tốc thêm một chút (cảm giác "đẩy")
        if (b.is_player_bullet && b.vy < -7.0f) b.vy -= 0.5f;

        // Cập nhật frame animation đạn (50ms/frame)
        if (now - b.anim.last_anim_time > 50) {
            b.anim.current_frame = (b.anim.current_frame + 1) % b.anim.total_frames;
            b.anim.last_anim_time = now;
        }

        // Hủy đạn ra ngoài màn hình (buffer 100px)
        if (b.x < -100 || b.x > SCREEN_WIDTH + 100 || b.y < -100 || b.y > SCREEN_HEIGHT + 100)
            b.active = false;
    }
}

void updatePickups() {
    Uint32 now = SDL_GetTicks();
    for (auto& p : pickups) {
        if (!p.active) continue;
        p.y += p.vy;
        if (now - p.anim.last_anim_time > 80) {
            p.anim.current_frame = (p.anim.current_frame + 1) % p.anim.total_frames;
            p.anim.last_anim_time = now;
        }
        if (p.y > SCREEN_HEIGHT) p.active = false;
    }
}

void updateEnemies() {
    Uint32 now = SDL_GetTicks();
    for (auto& e : enemies) {
        if (!e.active) continue;

        if (e.state == EnemyState::EXPLODING) {
            // Phát hoạt ảnh nổ, ẩn khi hết frame
            if (now - e.anim.last_anim_time > 80) {
                e.anim.current_frame++;
                e.anim.last_anim_time = now;
                if (e.anim.current_frame >= e.anim.total_frames)
                    e.active = false;
            }
            continue;
        }

        e.y += 1.5f;
        if (e.y > SCREEN_HEIGHT) { e.active = false; continue; }

        // Bắn đạn theo kiểu riêng của từng hạm đội
        if (now - e.last_shoot_time > e.shoot_delay) {
            float cx = e.x + e.w / 2.0f;
            float cy = e.y + e.h;

            if (e.fleet == EnemyFleet::KLAED) {
                // Bắn đạn ngắm thẳng vào tàu player
                float dx = (player.x + 32) - cx;
                float dy = (player.y + 32) - cy;
                float len = std::sqrt(dx*dx + dy*dy);
                if (len > 0.001f) {
                    bullets.push_back(createBullet(e.texture_bullet, cx - 16, cy,
                                                   (dx/len)*4.0f, (dy/len)*4.0f, 1, false));
                }
            }
            else if (e.fleet == EnemyFleet::NAIRAN) {
                // Bắn 3 đạn hình quạt xuống (70°, 90°, 110°)
                float angles[] = { 70.0f, 90.0f, 110.0f };
                for (float deg : angles) {
                    float rad = deg * 3.14159f / 180.0f;
                    bullets.push_back(createBullet(e.texture_bullet, cx - 16, cy,
                                                   std::cos(rad)*3.5f, std::sin(rad)*3.5f, 1, false));
                }
            }
            else if (e.fleet == EnemyFleet::NAUTOLAN) {
                // Bắn 8 đạn toả tròn (bullet hell)
                for (int i = 0; i < 8; i++) {
                    float rad = (i * 45.0f) * 3.14159f / 180.0f;
                    bullets.push_back(createBullet(e.texture_bullet, cx - 16, cy,
                                                   std::cos(rad)*3.0f, std::sin(rad)*3.0f, 1, false));
                }
            }
            e.last_shoot_time = now;
        }
    }
}

void updatePlayer() {
    Uint32 now = SDL_GetTicks();

    if (player.is_dead) {
        // Chờ 500ms rồi bắt đầu hồi sinh (bay lên từ dưới màn hình)
        if (now - player.death_time > 500) {
            player.is_dead        = false;
            player.is_respawning  = true;
            player.is_invulnerable = true;
            player.invuln_start_time = now;
            player.x = SCREEN_WIDTH / 2 - 32;
            player.y = SCREEN_HEIGHT + 64; // Bắt đầu ngoài màn hình phía dưới
        }
        return;
    }

    if (player.is_respawning) {
        player.y -= 3.0f;
        if (player.y <= SCREEN_HEIGHT - 100) {
            player.y = SCREEN_HEIGHT - 100;
            player.is_respawning = false;
        }
        // Cập nhật lửa động cơ ngay cả lúc đang hồi sinh
        if (now - player.engine_anim.last_anim_time > 100) {
            player.engine_anim.current_frame = (player.engine_anim.current_frame + 1) % player.engine_anim.total_frames;
            player.engine_anim.last_anim_time = now;
        }
        return;
    }

    // Hết thời gian khiên (10 giây)
    if (player.has_shield && now - player.shield_start_time > 10000)
        player.has_shield = false;

    // Cập nhật hoạt ảnh khiên (50ms/frame)
    if (player.has_shield && now - player.shield_anim.last_anim_time > 50) {
        player.shield_anim.current_frame = (player.shield_anim.current_frame + 1) % player.shield_anim.total_frames;
        player.shield_anim.last_anim_time = now;
    }

    // Hết bất khả xâm phạm (3 giây sau hồi sinh)
    if (player.is_invulnerable && now - player.invuln_start_time > 3000)
        player.is_invulnerable = false;

    // Di chuyển
    const float speed = 6.0f;
    if (keyboardState[SDL_SCANCODE_LEFT]  && player.x > 0)                player.x -= speed;
    if (keyboardState[SDL_SCANCODE_RIGHT] && player.x < SCREEN_WIDTH - 64) player.x += speed;
    if (keyboardState[SDL_SCANCODE_UP]    && player.y > 0)                player.y -= speed;
    if (keyboardState[SDL_SCANCODE_DOWN]  && player.y < SCREEN_HEIGHT - 64) player.y += speed;

    // Hitbox căn giữa tàu
    player.hitbox_x = player.x + 32 - player.hitbox_w / 2.0f;
    player.hitbox_y = player.y + 32 - player.hitbox_h / 2.0f;

    // Lửa động cơ
    if (now - player.engine_anim.last_anim_time > 100) {
        player.engine_anim.current_frame = (player.engine_anim.current_frame + 1) % player.engine_anim.total_frames;
        player.engine_anim.last_anim_time = now;
    }

    // Bắn đạn khi giữ Space
    if (keyboardState[SDL_SCANCODE_SPACE]) {
        float shootDelay = 200.0f;
        if      (player.current_weapon == WeaponType::AUTO_CANNON)  shootDelay = 150.0f;
        else if (player.current_weapon == WeaponType::BIG_SPACE_GUN) shootDelay = 400.0f;
        else if (player.current_weapon == WeaponType::ZAPPER)        shootDelay = 100.0f;

        if (now - player.last_shoot_time > shootDelay) {
            float cx = player.x + 32; // Tâm ngang của tàu
            int   lv = player.weapon_level;

            if (player.current_weapon == WeaponType::AUTO_CANNON) {
                if (lv == 1) {
                    bullets.push_back(createBullet(texWeaponAuto, cx - 16, player.y, 0, -15, 2, true));
                } else if (lv == 2) {
                    bullets.push_back(createBullet(texWeaponAuto, cx - 32, player.y, 0, -15, 2, true));
                    bullets.push_back(createBullet(texWeaponAuto, cx,      player.y, 0, -15, 2, true));
                } else { // lv 3: 3 đạn hình tam giác
                    bullets.push_back(createBullet(texWeaponAuto, cx - 16, player.y - 12, 0, -15, 4, true));
                    bullets.push_back(createBullet(texWeaponAuto, cx - 48, player.y + 12, 0, -15, 4, true));
                    bullets.push_back(createBullet(texWeaponAuto, cx + 16, player.y + 12, 0, -15, 4, true));
                }
            }
            else if (player.current_weapon == WeaponType::BIG_SPACE_GUN) {
                float sc = (lv == 3) ? 2.0f : (lv == 2) ? 1.5f : 1.0f;
                Bullet b = createBullet(texWeaponBig, cx - 16*sc, player.y, 0, -15, 15, true);
                b.scale = sc;
                bullets.push_back(b);
            }
            else if (player.current_weapon == WeaponType::ROCKET) {
                // Lv1: 3 đạn / Lv2: 4 đạn / Lv3: 5 đạn (hình rẻ quạt rộng dần)
                if (lv == 1) {
                    bullets.push_back(createBullet(texWeaponRocket, cx - 16, player.y,      0.0f, -10, 2, true));
                    bullets.push_back(createBullet(texWeaponRocket, cx - 32, player.y + 10, -2.5f, -9, 2, true));
                    bullets.push_back(createBullet(texWeaponRocket, cx,      player.y + 10,  2.5f, -9, 2, true));
                } else if (lv == 2) {
                    bullets.push_back(createBullet(texWeaponRocket, cx - 24, player.y + 5,  -1.0f, -9.5f, 2, true));
                    bullets.push_back(createBullet(texWeaponRocket, cx -  8, player.y + 5,   1.0f, -9.5f, 2, true));
                    bullets.push_back(createBullet(texWeaponRocket, cx - 44, player.y + 15, -3.5f, -8.5f, 2, true));
                    bullets.push_back(createBullet(texWeaponRocket, cx + 12, player.y + 15,  3.5f, -8.5f, 2, true));
                } else {
                    bullets.push_back(createBullet(texWeaponRocket, cx - 16, player.y,       0.0f, -10,   2, true));
                    bullets.push_back(createBullet(texWeaponRocket, cx - 32, player.y + 10, -2.5f, -9,    2, true));
                    bullets.push_back(createBullet(texWeaponRocket, cx,      player.y + 10,  2.5f, -9,    2, true));
                    bullets.push_back(createBullet(texWeaponRocket, cx - 48, player.y + 20, -4.5f, -8,    2, true));
                    bullets.push_back(createBullet(texWeaponRocket, cx + 16, player.y + 20,  4.5f, -8,    2, true));
                }
            }
            else if (player.current_weapon == WeaponType::ZAPPER) {
                // 2 đạn toả ra hai bên (hình chữ V)
                float sc = (lv == 3) ? 2.0f : (lv == 2) ? 1.5f : 1.0f;
                Bullet b1 = createBullet(texWeaponZapper, cx - 16*sc, player.y, -2.0f, -15, 3, true);
                Bullet b2 = createBullet(texWeaponZapper, cx - 16*sc, player.y,  2.0f, -15, 3, true);
                b1.scale = b2.scale = sc;
                bullets.push_back(b1);
                bullets.push_back(b2);
            }
            player.last_shoot_time = now;
        }
    }
}

// Xóa các entity inactive ra khỏi vector (gọi cuối frame)
void cleanupVectors() {
    auto inactive = [](auto& obj) { return !obj.active; };
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(), inactive), bullets.end());
    enemies.erase(std::remove_if(enemies.begin(), enemies.end(), inactive), enemies.end());
    pickups.erase(std::remove_if(pickups.begin(), pickups.end(), inactive), pickups.end());
    floating_texts.erase(std::remove_if(floating_texts.begin(), floating_texts.end(), inactive), floating_texts.end());
}

// ============================================================
//  COLLISION DETECTION
// ============================================================

// AABB cơ bản: trả về true nếu 2 hình chữ nhật giao nhau.
bool AABB(float x1, float y1, float w1, float h1,
          float x2, float y2, float w2, float h2) {
    return x1 < x2+w2 && x1+w1 > x2 && y1 < y2+h2 && y1+h1 > y2;
}

// Kích hoạt hiệu ứng nổ cho enemy và lấy số frame từ sprite nổ.
void triggerExplosion(Enemy& e) {
    e.state = EnemyState::EXPLODING;
    e.anim.current_frame  = 0;
    e.anim.last_anim_time = SDL_GetTicks();
    e.anim.total_frames   = e.texture_destruction ? countFrames(e.texture_destruction) : 1;
}

// Xử lý player bị trúng đòn (đạn hoặc va chạm).
// Nếu có khiên → phá khiên, không trừ mạng. Ngược lại trừ mạng.
void damagePlayer() {
    if (player.has_shield) {
        player.has_shield = false; // Khiên chặn 1 đòn
    } else {
        player.is_dead    = true;
        player.death_time = SDL_GetTicks();
        player.weapon_level = std::max(1, player.weapon_level - 1); // Giảm cấp vũ khí
        player.lives--;
        if (player.lives < 0) handlePlayerDeath();
    }
}

void getEnemyHitbox(const Enemy& e, float& ex, float& ey, float& ew, float& eh) {
    if (e.size_class == EnemySize::SMALL) {
        ex = e.x + e.w * 0.25f; ey = e.y + e.h * 0.25f;
        ew = e.w * 0.50f;       eh = e.h * 0.50f;
    } else if (e.size_class == EnemySize::MEDIUM) {
        ex = e.x + e.w * 0.20f; ey = e.y + e.h * 0.20f;
        ew = e.w * 0.60f;       eh = e.h * 0.60f;
    } else {
        ex = e.x + e.w * 0.10f; ey = e.y + e.h * 0.10f;
        ew = e.w * 0.80f;       eh = e.h * 0.80f;
    }
}

void checkCollisions() {
    // 1. Đạn player bắn vào quái
    for (auto& b : bullets) {
        if (!b.active || !b.is_player_bullet) continue;
        float bw = b.w * b.scale, bh = b.h * b.scale;

        for (auto& e : enemies) {
            if (!e.active || e.state != EnemyState::ALIVE) continue;
            
            float ex, ey, ew, eh;
            getEnemyHitbox(e, ex, ey, ew, eh);

            if (AABB(b.x + bw/4, b.y + bh/4, bw/2, bh/2, ex, ey, ew, eh)) {
                b.active = false;
                e.hp    -= b.damage;
                if (e.hp <= 0) {
                    triggerExplosion(e);
                    player.score += e.score_value;

                    // Hiển thị điểm nổi tại vị trí quái chết
                    floating_texts.push_back({ e.x + e.w/2.0f - 20, e.y + e.h/2.0f - 10,
                                               e.score_value, 255.0f, true });

                    // Rơi vật phẩm theo xác suất
                    if (rand() % 100 < e.drop_chance) {
                        Pickup p;
                        p.x  = e.x + e.w/2.0f - 16;
                        p.y  = e.y + e.h/2.0f;
                        p.vy = 2.0f;
                        p.active = true;
                        p.anim.current_frame  = 0;
                        p.anim.last_anim_time = SDL_GetTicks();

                        // 20% khiên, 80% vũ khí
                        if (rand() % 100 < 20) {
                            p.type    = PickupType::SHIELD;
                            p.texture = texIconShield;
                        } else {
                            p.type = PickupType::WEAPON;
                            SDL_Texture* weaponTex[] = { texIconAuto, texIconBig, texIconRocket, texIconZapper };
                            WeaponType   weaponType[] = { WeaponType::AUTO_CANNON, WeaponType::BIG_SPACE_GUN,
                                                          WeaponType::ROCKET, WeaponType::ZAPPER };
                            int t = rand() % 4;
                            p.weapon_type = weaponType[t];
                            p.texture     = weaponTex[t];
                        }

                        p.anim.total_frames = countFrames(p.texture);
                        SDL_QueryTexture(p.texture, NULL, NULL, &p.w, &p.h);
                        p.w = p.h; // Frame vuông: rộng = cao
                        pickups.push_back(p);
                    }
                }
                break; // 1 đạn chỉ trúng 1 quái
            }
        }
    }

    if (player.is_dead || player.is_invulnerable || player.is_respawning) return;

    // 2. Đạn quái bắn vào player
    for (auto& b : bullets) {
        if (!b.active || b.is_player_bullet) continue;
        float bw = b.w * b.scale, bh = b.h * b.scale;
        if (AABB(b.x + bw/4, b.y + bh/4, bw/2, bh/2,
                 player.hitbox_x, player.hitbox_y, player.hitbox_w, player.hitbox_h)) {
            b.active = false;
            damagePlayer();
            break;
        }
    }

    // 3. Quái đâm thẳng vào tàu player
    for (auto& e : enemies) {
        if (!e.active || e.state != EnemyState::ALIVE) continue;
        
        float ex, ey, ew, eh;
        getEnemyHitbox(e, ex, ey, ew, eh);
        if (AABB(player.x, player.y, 64, 64, ex, ey, ew, eh)) {
            damagePlayer();
            triggerExplosion(e); // Quái nổ khi đâm vào tàu
            break;
        }
    }

    // 4. Player nhặt vật phẩm
    for (auto& p : pickups) {
        if (!p.active || !AABB(player.x, player.y, 64, 64, p.x, p.y, p.w, p.h)) continue;
        if (p.type == PickupType::WEAPON) {
            player.current_weapon = p.weapon_type;
            player.weapon_level   = std::min(3, player.weapon_level + 1);
        } else {
            player.has_shield        = true;
            player.shield_start_time = SDL_GetTicks();
        }
        player.score += 100;
        p.active = false;
        floating_texts.push_back({ p.x, p.y - 10, 100, 255.0f, true });
    }
}

// ============================================================
//  RENDER
// ============================================================
void render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // --- Nền sao ---
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    for (auto& s : stars) {
        SDL_Rect r = { (int)s.x, (int)s.y, s.size, s.size };
        SDL_RenderFillRect(renderer, &r);
    }
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    // --- MENU ---
    if (currentState == GameState::MENU) {
        renderTextCentered("SPACE ODYSSEY", {255, 230, 100, 255}, 220, 2.0f);
        // Nhấp nháy 500ms
        if ((SDL_GetTicks() / 500) % 2 == 0)
            renderTextCentered("Press SPACE to Start", {255, 255, 255, 255}, 330);
        renderTextCentered("HIGH SCORE: " + std::to_string(highScore), {50, 255, 100, 255}, 380);
    }

    // --- GAME OVER ---
    else if (currentState == GameState::GAMEOVER) {
        renderTextCentered("GAME OVER!", {255, 50, 50, 255}, 250, 2.0f);
        renderTextCentered("Final Score: " + std::to_string(player.score), {255, 255, 255, 255}, 350);
        if ((SDL_GetTicks() / 500) % 2 == 0)
            renderTextCentered("Press SPACE to Save and Exit", {150, 150, 150, 255}, 450);
    }

    // --- PLAYING ---
    else if (currentState == GameState::PLAYING) {
        // Vẽ đạn
        for (auto& b : bullets) {
            if (!b.active) continue;
            float bw = b.w * b.scale, bh = b.h * b.scale;
            SDL_Rect src  = { b.anim.current_frame * b.w, 0, b.w, b.h };
            SDL_Rect dest = { (int)b.x, (int)b.y, (int)bw, (int)bh };
            SDL_RenderCopy(renderer, b.texture, &src, &dest);
        }

        // Vẽ vật phẩm
        for (auto& p : pickups) {
            if (!p.active) continue;
            SDL_Rect src  = { p.anim.current_frame * p.w, 0, p.w, p.h };
            SDL_Rect dest = { (int)p.x, (int)p.y, 32, 32 };
            SDL_RenderCopy(renderer, p.texture, &src, &dest);
        }

        // Vẽ quái (hoặc hoạt ảnh nổ)
        for (auto& e : enemies) {
            if (!e.active) continue;
            SDL_Rect dest = { (int)e.x, (int)e.y, e.w, e.h };
            if (e.state == EnemyState::ALIVE) {
                SDL_RenderCopy(renderer, e.texture_base, NULL, &dest);
            } else if (e.texture_destruction) {
                int tw, th;
                SDL_QueryTexture(e.texture_destruction, NULL, NULL, &tw, &th);
                int fw = th; // Frame vuông
                SDL_Rect src = { e.anim.current_frame * fw, 0, fw, th };
                dest.w = fw; dest.h = th;
                SDL_RenderCopy(renderer, e.texture_destruction, &src, &dest);
            }
        }

        // Vẽ player (nhấp nháy khi bất khả xâm phạm)
        if (!player.is_dead) {
            bool blink = player.is_invulnerable &&
                         (SDL_GetTicks() - player.invuln_start_time) / 150 % 2 == 0;
            if (!blink) {
                SDL_Rect dest = { (int)player.x, (int)player.y, 64, 64 };
                int healthIndex = std::max(0, std::min(3, player.lives));
                SDL_RenderCopy(renderer, texPlayer[healthIndex], NULL, &dest);

                // Lửa động cơ (thụt vào sau thân tàu 5px)
                SDL_Rect engSrc  = { player.engine_anim.current_frame * 48, 0, 48, 48 };
                SDL_Rect engDest = { (int)player.x, (int)player.y + 5, 64, 64 };
                SDL_RenderCopy(renderer, texEngineFlame, &engSrc, &engDest);

                // Vòng sáng khiên (to hơn tàu 10px mỗi bên)
                if (player.has_shield && texPlayerShield) {
                    SDL_Rect src  = { player.shield_anim.current_frame * player.shield_w, 0,
                                      player.shield_w, player.shield_h };
                    SDL_Rect dest2 = { (int)player.x - 10, (int)player.y - 10, 84, 84 };
                    SDL_RenderCopy(renderer, texPlayerShield, &src, &dest2);
                }

                // Debug: vẽ hitbox đỏ (comment dòng dưới khi build release)
                SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
                drawFilledCircle(renderer,
                    (int)(player.hitbox_x + player.hitbox_w/2),
                    (int)(player.hitbox_y + player.hitbox_h/2),
                    player.hitbox_w / 2);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            }
        }

        // Vẽ điểm nổi
        for (auto& ft : floating_texts) {
            if (!ft.active) continue;
            SDL_Texture* tex = (ft.score == 50) ? texScore50 :
                               (ft.score == 70) ? texScore70 : texScore100;
            if (!tex) continue;
            SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
            SDL_SetTextureAlphaMod(tex, (Uint8)ft.alpha);
            int tw, th;
            SDL_QueryTexture(tex, NULL, NULL, &tw, &th);
            SDL_Rect r = { (int)ft.x, (int)ft.y, tw, th };
            SDL_RenderCopy(renderer, tex, NULL, &r);
        }

        // HUD: điểm + wave + cấp vũ khí + mạng
        renderText("SCORE: " + std::to_string(player.score),         {255,255,255,255},  10, 10);
        renderText("WAVE: "  + std::to_string(currentWave),          {255,255,100,255},  SCREEN_WIDTH - 100, 10);
        renderText("POWER LV " + std::to_string(player.weapon_level), {100,255,255,255}, 10, SCREEN_HEIGHT - 35);
        renderText("LIVES: " + std::to_string(player.lives),          {255, 80, 80,255}, SCREEN_WIDTH - 110, SCREEN_HEIGHT - 35);
    }

    SDL_RenderPresent(renderer);
}

// ============================================================
//  MAIN: VÒNG LẶP GAME
// ============================================================
int main(int argc, char* args[]) {
    srand((unsigned)time(NULL));

    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    window   = SDL_CreateWindow("Space Odyssey", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!loadFiles()) return -1;

    initStars();
    initPlayer();

    bool run = true;
    SDL_Event event;

    while (run) {
        Uint32 frameStart = SDL_GetTicks();
        keyboardState = SDL_GetKeyboardState(NULL);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) { run = false; break; }
            if (event.type == SDL_KEYDOWN) {
                SDL_Keycode key = event.key.keysym.sym;
                if (key == SDLK_SPACE || key == SDLK_RETURN) {
                    if (currentState == GameState::MENU) {
                        restartGame();
                    } else if (currentState == GameState::GAMEOVER) {
                        bullets.clear(); enemies.clear(); pickups.clear(); floating_texts.clear();
                        currentState = GameState::MENU;
                    }
                }
            }
        }

        if (currentState == GameState::PLAYING) {
            updateStars();
            updatePlayer();
            updateBullets();
            updateEnemies();
            updatePickups();
            updateFloatingTexts();
            checkCollisions();
            cleanupVectors();

            if (++spawnTimer >= spawnCooldown) {
                spawnEnemy();
                spawnTimer = 0;
                currentWave   = 1 + (player.score / 500);
                spawnCooldown = std::max(20, 120 - currentWave * 5);
            }
        } else {
            updateStars(); // Menu / GameOver: sao vẫn trôi
        }

        render();

        // Giữ ~60 FPS (16ms/frame)
        Uint32 elapsed = SDL_GetTicks() - frameStart;
        if (elapsed < 16) SDL_Delay(16 - elapsed);
    }

    // Giải phóng tài nguyên
    TTF_CloseFont(font);

    SDL_Texture* allTextures[] = {
        texPlayer[0], texPlayer[1], texPlayer[2], texPlayer[3], texEngineFlame,
        texWeaponAuto, texWeaponBig, texWeaponRocket, texWeaponZapper,
        texIconAuto, texIconBig, texIconRocket, texIconZapper, texIconShield,
        texPlayerShield,
        texScore50, texScore70, texScore100,
        texEnemyKlaedBullet, texEnemyNairanBullet, texEnemyNautolanBullet
    };
    for (auto t : allTextures) SDL_DestroyTexture(t);

    for (auto& d : catalogKlaed)    { SDL_DestroyTexture(d.base); SDL_DestroyTexture(d.dest); }
    for (auto& d : catalogNairan)   { SDL_DestroyTexture(d.base); SDL_DestroyTexture(d.dest); }
    for (auto& d : catalogNautolan) { SDL_DestroyTexture(d.base); SDL_DestroyTexture(d.dest); }

    if (bgmMusic) Mix_FreeMusic(bgmMusic);
    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit(); IMG_Quit(); SDL_Quit();

    return 0;
}
