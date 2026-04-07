// ============================================================
//  main.cpp — Điểm vào chương trình
//
//  File này chỉ chứa:
//  - Khởi tạo player
//  - Hàm restart game và xử lý chết
//  - Vòng lặp game chính (game loop)
//  - Khởi tạo và dọn dẹp SDL
//
//  Logic gameplay → update.cpp
//  Vẽ màn hình    → render.cpp
//  Tải assets     → assets.cpp
//  Biến global    → globals.cpp
// ============================================================
#include "globals.h"
#include "assets.h"
#include "render.h"
#include "update.h"

// ============================================================
//  initPlayer — Đặt lại trạng thái player về mặc định
// ============================================================
void initPlayer() {
    player.x     = (float)(SCREEN_WIDTH  / 2 - PLAYER_SIZE / 2);
    player.y     = (float)(SCREEN_HEIGHT - 100);
    player.lives = INITIAL_LIVES;
    player.score = 0;

    // Trạng thái sống
    player.is_dead         = false;
    player.is_respawning   = false;
    player.is_invulnerable = true;           // Bất tử ngay từ đầu (buffer an toàn)
    player.invuln_start_time = SDL_GetTicks();

    // Vũ khí
    player.current_weapon  = WeaponType::AUTO_CANNON;
    player.weapon_level    = 1;
    player.last_shoot_time = 0;

    // Hitbox 10×10 đặt ở giữa ngực tàu
    player.hitbox_w = 10;
    player.hitbox_h = 10;

    // Khiên
    player.has_shield        = false;
    player.shield_start_time = 0;
    player.shield_w          = 64;
    player.shield_h          = 64;
    player.shield_anim.current_frame  = 0;
    player.shield_anim.total_frames   = 12;
    player.shield_anim.last_anim_time = 0;

    if (texPlayerShield != nullptr) {
        SDL_QueryTexture(texPlayerShield, NULL, NULL, &player.shield_w, &player.shield_h);
        // Shield sprite sheet: mỗi frame vuông = chiều cao sheet
        int frameWidth        = player.shield_h;
        player.shield_anim.total_frames = countFrames(texPlayerShield);
        player.shield_w       = frameWidth;
    }

    // Lửa động cơ đuôi
    player.engine_anim.current_frame  = 0;
    player.engine_anim.total_frames   = 4;
    player.engine_anim.last_anim_time = 0;
}



// ============================================================
//  restartGame — Bắt đầu/bắt đầu lại ván chơi
// ============================================================
void restartGame() {
    initPlayer();

    // Xoá toàn bộ entities trên màn hình
    bullets.clear();
    enemies.clear();
    pickups.clear();
    floating_texts.clear();

    // Reset về stage 1, wave 1
    currentStage    = 1;
    currentWave     = 1;
    enemyKillCount  = 0;
    waveEnemyTarget = currentWave * 2;
    spawnTimer      = 0;
    spawnedSmall    = 0;
    spawnedMedium   = 0;

    loadStageCatalog(1);
    currentState = GameState::PLAYING;

    if (bgmMusic != nullptr) {
        Mix_PlayMusic(bgmMusic, -1);  // -1 = lặp vô hạn
    }
}

// ============================================================
//  MAIN — Điểm bắt đầu chương trình
// ============================================================
int main(int argc, char* args[]) {
    // Khởi tạo số ngẫu nhiên theo thời gian thực
    srand((unsigned int)time(NULL));

    // === Khởi tạo SDL ===
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    // === Tạo cửa sổ và renderer ===
    window = SDL_CreateWindow(
        "Space Odyssey",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        0
    );
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // === Tải assets ===
    bool loadOK = loadFiles();
    if (loadOK == false) {
        return -1;  // Không tìm thấy font → không chạy được
    }

    // === Khởi tạo game ===

    initPlayer();

    // === Vòng lặp game ===
    bool isRunning = true;
    SDL_Event event;

    while (isRunning == true) {
        Uint32 frameStart = SDL_GetTicks();

        // Đọc trạng thái bàn phím mỗi frame (dùng cho di chuyển / bắn)
        keyboardState = SDL_GetKeyboardState(NULL);

        // =============================================
        //  XỬ LÝ SỰ KIỆN (chuột, bàn phím, thoát...)
        // =============================================
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
                break;
            }

            if (event.type == SDL_KEYDOWN) {
                SDL_Keycode key = event.key.keysym.sym;
                bool isActionKey = (key == SDLK_SPACE || key == SDLK_RETURN);

                if (isActionKey == true) {
                    if (currentState == GameState::MENU) {
                        restartGame();
                    }
                    else if (currentState == GameState::GAMEOVER) {
                        // Về menu sau khi game over
                        bullets.clear();
                        enemies.clear();
                        pickups.clear();
                        floating_texts.clear();
                        currentState = GameState::MENU;
                    }
                    else if (currentState == GameState::GAME_WON) {
                        // Về menu sau khi phá đảo
                        bullets.clear();
                        enemies.clear();
                        pickups.clear();
                        floating_texts.clear();
                        currentState = GameState::MENU;
                    }
                }
            }
        }

        // =============================================
        //  CẬP NHẬT TRẠNG THÁI GAME
        // =============================================
        if (currentState == GameState::PLAYING) {
            if (transitionStart > 0) {
                Uint32 timeInTransition = SDL_GetTicks() - transitionStart;
                if (timeInTransition > 5000) {
                    currentStage++;
                    currentWave     = 1;
                    enemyKillCount  = 0;
                    waveEnemyTarget = currentWave * 2;
                    spawnTimer      = 0;
                    spawnedSmall    = 0;
                    spawnedMedium   = 0;
                    transitionStart = 0;

                    bullets.clear();
                    enemies.clear();
                    pickups.clear();
                    floating_texts.clear();

                    loadStageCatalog(currentStage);
                }
            }

            // Cập nhật toàn bộ gameplay mỗi frame
            updatePlayer();
            updateBullets();
            updateEnemies();
            updatePickups();
            updateFloatingTexts();
            checkCollisions();
            cleanupVectors();
            updateSpawner();
        }


        // =============================================
        //  VẼ MÀN HÌNH
        // =============================================
        render();

        // =============================================
        //  GIỚI HẠN ~60 FPS (16ms mỗi frame)
        // =============================================
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < 16) {
            SDL_Delay(16 - frameTime);
        }
    }

    // =============================================
    //  DỌN DẸP TÀI NGUYÊN TRƯỚC KHI THOÁT
    // =============================================
    TTF_CloseFont(font);

    // Giải phóng tất cả texture
    SDL_Texture* allTextures[] = {
        texPlayer[0], texPlayer[1], texPlayer[2], texPlayer[3],
        texEngineFlame,
        texWeaponAuto, texWeaponRocket,
        texIconAuto, texIconRocket, texIconShield,
        texPlayerShield,
        texScore50, texScore70, texScore100,
        texEnemyBullet
    };

    int textureCount = sizeof(allTextures) / sizeof(allTextures[0]);
    for (int i = 0; i < textureCount; i++) {
        safeDestroy(allTextures[i]);
    }

    safeDestroy(texEnemySmallBase);
    safeDestroy(texEnemySmallDest);
    safeDestroy(texEnemyMediumBase);
    safeDestroy(texEnemyMediumDest);
    safeDestroy(texEnemyLargeBase);
    safeDestroy(texEnemyLargeDest);
    safeDestroy(texEnemyBullet);

    // Giải phóng nhạc và đóng SDL
    if (bgmMusic != nullptr) {
        Mix_FreeMusic(bgmMusic);
    }

    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
