// ============================================================
//  assets.cpp — Tải toàn bộ file ảnh, font, âm thanh
// ============================================================
#include "assets.h"
#include "render.h"  // Cần loadText() để tạo texture điểm nổi trong loadFiles()

// ============================================================
//  loadTexture — Tải ảnh PNG từ file vào VRAM
// ============================================================
SDL_Texture* loadTexture(const std::string& path) {
    // Bước 1: Đọc file ảnh vào RAM (SDL_Surface là vùng nhớ thông thường)
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (surface == nullptr) {
        return nullptr;  // File không tồn tại hoặc định dạng không hỗ trợ
    }

    // Bước 2: Chuyển từ RAM lên VRAM của card đồ hoạ (SDL_Texture)
    // Texture trên VRAM thì render nhanh hơn nhiều so với surface trên RAM
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    // Bước 3: Giải phóng bản copy trên RAM, chỉ giữ lại texture trên VRAM
    SDL_FreeSurface(surface);

    return texture;
}

// ============================================================
//  loadFiles — Tải toàn bộ asset khi khởi động game
// ============================================================
bool loadFiles() {
    // --- Font chữ ---
    font = TTF_OpenFont("asset/font.TTF", 24);
    if (font == nullptr) {
        return false;  // Không có font → không hiện được gì → thoát
    }

    // --- Tàu người chơi (4 mức độ máu) ---
    texPlayer[0] = loadTexture("asset/MainShip/Main Ship/Main Ship - Bases/PNGs/Main Ship - Base - Very damaged.png");
    texPlayer[1] = loadTexture("asset/MainShip/Main Ship/Main Ship - Bases/PNGs/Main Ship - Base - Damaged.png");
    texPlayer[2] = loadTexture("asset/MainShip/Main Ship/Main Ship - Bases/PNGs/Main Ship - Base - Slight damage.png");
    texPlayer[3] = loadTexture("asset/MainShip/Main Ship/Main Ship - Bases/PNGs/Main Ship - Base - Full health.png");

    texEngineFlame = loadTexture("asset/MainShip/Main Ship/Main Ship - Engine Effects/PNGs/Main Ship - Engines - Base Engine - Spritesheet.png");

    // --- Đạn người chơi ---
    texWeaponAuto   = loadTexture("asset/MainShip/Main ship weapons/PNGs/Main ship weapon - Projectile - Auto cannon bullet.png");
    texWeaponRocket = loadTexture("asset/MainShip/Main ship weapons/PNGs/Main ship weapon - Projectile - Rocket.png");

    // --- Icon vật phẩm rơi ---
    texIconAuto   = loadTexture("asset/PickupsPack/Weapons/PNGs/Pickup Icon - Weapons - Auto Cannons.png");
    texIconRocket = loadTexture("asset/PickupsPack/Weapons/PNGs/Pickup Icon - Weapons - Rocket.png");
    texIconShield = loadTexture("asset/PickupsPack/Shield Generators/PNGs/Pickup Icon - Shield Generator - All around shield.png");

    // --- Khiên người chơi ---
    texPlayerShield = loadTexture("asset/MainShip/Main Ship/Main Ship - Shields/PNGs/Main Ship - Shields - Round Shield.png");

    // --- Điểm nổi: render text ra texture 1 lần, dùng lại nhiều lần ---
    // (Tránh tạo/huỷ texture mỗi frame sẽ chậm)
    SDL_Color colorGreen  = {200, 255, 100, 255};
    SDL_Color colorPurple = {255, 100, 255, 255};
    SDL_Color colorYellow = {255, 200,  50, 255};
    texScore50  = loadText("+50",  colorGreen);
    texScore70  = loadText("+70",  colorPurple);
    texScore100 = loadText("+100", colorYellow);

    // --- Nhạc nền ---
    bgmMusic = Mix_LoadMUS("bgm/bgm.mp3");

    return true;
}

// ============================================================
//  loadStageCatalog — Nạp quái theo stage (gọi khi bắt đầu/đổi stage)
//  Giải phóng texture cũ, load texture mới từ thư mục tương ứng.
// ============================================================
void loadStageCatalog(int stage) {
    // --- Giải phóng texture quái cũ nếu có ---
    safeDestroy(texEnemySmallBase);
    safeDestroy(texEnemySmallDest);
    safeDestroy(texEnemyMediumBase);
    safeDestroy(texEnemyMediumDest);
    safeDestroy(texEnemyLargeBase);
    safeDestroy(texEnemyLargeDest);
    safeDestroy(texEnemyBullet);

    if (stage == 1) {
        // === STAGE 1: Hạm đội Kla'ed ===
        texEnemyBullet = loadTexture("asset/EnemyFleet_1/Kla'ed/Projectiles/PNGs/Kla'ed - Bullet.png");

        texEnemySmallBase  = loadTexture("asset/EnemyFleet_1/Kla'ed/Base/PNGs/Kla'ed - Scout - Base.png");
        texEnemySmallDest  = loadTexture("asset/EnemyFleet_1/Kla'ed/Destruction/PNGs/Kla'ed - Scout - Destruction.png");
        
        texEnemyMediumBase = loadTexture("asset/EnemyFleet_1/Kla'ed/Base/PNGs/Kla'ed - Frigate - Base.png");
        texEnemyMediumDest = loadTexture("asset/EnemyFleet_1/Kla'ed/Destruction/PNGs/Kla'ed - Frigate - Destruction.png");
        
        texEnemyLargeBase  = loadTexture("asset/EnemyFleet_1/Kla'ed/Base/PNGs/Kla'ed - Dreadnought - Base.png");
        texEnemyLargeDest  = loadTexture("asset/EnemyFleet_1/Kla'ed/Destruction/PNGs/Kla'ed - Dreadnought - Destruction.png");
    }
    else if (stage == 2) {
        // === STAGE 2: Hạm đội Nairan ===
        texEnemyBullet = loadTexture("asset/EnemyFleet_2/Nairan/Projectiles/PNGs/Nairan - Bolt.png");

        texEnemySmallBase  = loadTexture("asset/EnemyFleet_2/Nairan/Designs - Base/PNGs/Nairan - Fighter - Base.png");
        texEnemySmallDest  = loadTexture("asset/EnemyFleet_2/Nairan/Destruction/PNGs/Nairan - Fighter -  Destruction.png");
        
        texEnemyMediumBase = loadTexture("asset/EnemyFleet_2/Nairan/Designs - Base/PNGs/Nairan - Bomber - Base.png");
        texEnemyMediumDest = loadTexture("asset/EnemyFleet_2/Nairan/Destruction/PNGs/Nairan - Bomber -  Destruction.png");
        
        texEnemyLargeBase  = loadTexture("asset/EnemyFleet_2/Nairan/Designs - Base/PNGs/Nairan - Battlecruiser - Base.png");
        texEnemyLargeDest  = loadTexture("asset/EnemyFleet_2/Nairan/Destruction/PNGs/Nairan - Battlecruiser  -  Destruction.png");
    }
    else {
        // === STAGE 3: Hạm đội Nautolan ===
        texEnemyBullet = loadTexture("asset/EnemyFleet_3/Nautolan/Projectiles/PNGs/Nautolan - Bullet.png");

        texEnemySmallBase  = loadTexture("asset/EnemyFleet_3/Nautolan/Designs - Base/PNGs/Nautolan Ship - Scout - Base.png");
        texEnemySmallDest  = loadTexture("asset/EnemyFleet_3/Nautolan/Destruction/PNGs/Nautolan Ship - Scout.png");
        
        texEnemyMediumBase = loadTexture("asset/EnemyFleet_3/Nautolan/Designs - Base/PNGs/Nautolan Ship - Frigate - Base.png");
        texEnemyMediumDest = loadTexture("asset/EnemyFleet_3/Nautolan/Destruction/PNGs/Nautolan Ship - Frigate.png");
        
        texEnemyLargeBase  = loadTexture("asset/EnemyFleet_3/Nautolan/Designs - Base/PNGs/Nautolan Ship - Dreadnought - Base.png");
        texEnemyLargeDest  = loadTexture("asset/EnemyFleet_3/Nautolan/Destruction/PNGs/Nautolan Ship - Dreadnought.png");
    }
}
