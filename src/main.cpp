#include "game.h"

// Cập nhật animation, trả true khi hết vòng
bool updateAnim(AnimState& anim, Uint32 now, Uint32 interval) {
    if (now - anim.last_anim_time <= interval) return false;
    anim.last_anim_time = now;
    if (++anim.current_frame >= anim.total_frames) {
        anim.current_frame = 0;
        return true;
    }
    return false;
}

// Đếm số frame trong sprite sheet (mỗi frame vuông)
int countFrames(SDL_Texture* tex) {
    if (!tex) return 1;
    int w, h;
    SDL_QueryTexture(tex, NULL, NULL, &w, &h);
    int frames = (h > 0) ? w / h : 1;
    return (frames > 0) ? frames : 1;
}

// Tạo 1 viên đạn (dùng chung cho player và quái)
Bullet createBullet(SDL_Texture* tex, float x, float y,
                    float vx, float vy, int damage, bool isPlayer)
{
    Bullet b;
    b.texture = tex;
    b.x = x; b.y = y;
    b.vx = vx; b.vy = vy;
    b.damage = damage;
    b.is_player_bullet = isPlayer;
    b.active = true;
    b.scale = 1.0f;
    b.w = 16; b.h = 16;
    b.anim = {0, 1, SDL_GetTicks()};
    if (tex) {
        int sw, sh;
        SDL_QueryTexture(tex, NULL, NULL, &sw, &sh);
        b.w = sh; b.h = sh;
        // Đạn player không animation, đạn quái có animation
        if (!isPlayer) b.anim.total_frames = countFrames(tex);
    }
    return b;
}

// Cập nhật đạn mỗi frame
void updateBullets() {
    Uint32 now = SDL_GetTicks();
    for (Bullet& b : bullets) {
        if (!b.active) continue;
        b.x += b.vx;
        b.y += b.vy;
        if (b.is_player_bullet && b.vy < -7.0f) b.vy -= 0.5f;
        updateAnim(b.anim, now, 50);
        if (b.x < -OFFSCREEN_MARGIN || b.x > SCREEN_WIDTH + OFFSCREEN_MARGIN ||
            b.y < -OFFSCREEN_MARGIN || b.y > SCREEN_HEIGHT + OFFSCREEN_MARGIN)
            b.active = false;
    }
}

// Cập nhật vật phẩm rơi
void updatePickups() {
    Uint32 now = SDL_GetTicks();
    for (Pickup& p : pickups) {
        if (!p.active) continue;
        p.y += p.vy;
        updateAnim(p.anim, now, 80);
        if (p.y > SCREEN_HEIGHT) p.active = false;
    }
}

// Cập nhật chữ nổi điểm
void updateFloatingTexts() {
    for (FloatingText& ft : floating_texts) {
        if (!ft.active) continue;
        ft.y -= 1.0f;
        ft.alpha -= 5.0f;
        if (ft.alpha <= 0) ft.active = false;
    }
}

// Xoá entity chết khỏi danh sách
void cleanupVectors() {
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
        [](const Bullet& x) { return !x.active; }), bullets.end());
    enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
        [](const Enemy& x) { return !x.active; }), enemies.end());
    pickups.erase(std::remove_if(pickups.begin(), pickups.end(),
        [](const Pickup& x) { return !x.active; }), pickups.end());
    floating_texts.erase(std::remove_if(floating_texts.begin(), floating_texts.end(),
        [](const FloatingText& x) { return !x.active; }), floating_texts.end());
}

// Tính số quái cần giết mỗi wave
static int calcWaveTarget() {
    return (currentStage == 3) ? currentWave : currentWave * 2;
}

// Khởi tạo player
static void initPlayer() {
    player.x     = (float)(SCREEN_WIDTH  / 2 - PLAYER_SIZE / 2);
    player.y     = (float)(SCREEN_HEIGHT - 100);
    player.lives = INITIAL_LIVES;
    player.score = 0;
    player.is_dead         = false;
    player.is_invulnerable = true;
    player.invuln_start_time = SDL_GetTicks();
    player.current_weapon  = WeaponType::AUTO_CANNON;
    player.weapon_level    = 1;
    player.last_shoot_time = 0;
    player.has_shield        = false;
    player.shield_start_time = 0;
    player.shield_anim = {0, 1, 0};
    if (texPlayerShield) {
        player.shield_anim.total_frames = countFrames(texPlayerShield);
    }
}

// Bắt đầu ván mới
static void restartGame() {
    initPlayer();
    bullets.clear(); enemies.clear(); pickups.clear(); floating_texts.clear();
    currentStage = 1; currentWave = 1;
    enemyKillCount = 0; waveEnemyTarget = calcWaveTarget();
    spawnTimer = 0; spawnedSmall = 0; spawnedMedium = 0;
    transitionStart = 0;
    currentState = GameState::PLAYING;
    if (bgmMusic) Mix_PlayMusic(bgmMusic, -1);
}

// Kiểm tra tiến trình wave/stage
static void checkWaveProgress() {
    if (transitionStart > 0 || enemyKillCount < waveEnemyTarget) return;

    if (currentWave == 10) {
        if (currentStage >= 3) {
            if (player.score > highScore) highScore = player.score;
            currentState = GameState::GAME_WON;
        } else {
            transitionStart = SDL_GetTicks();
        }
    } else {
        currentWave++;
        enemyKillCount = 0;
        spawnedSmall = 0; spawnedMedium = 0;
        waveEnemyTarget = calcWaveTarget();
    }
}

// MAIN
int main(int argc, char* args[]) {
    srand((unsigned int)time(NULL));
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    window = SDL_CreateWindow("Space Odyssey",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!loadFiles()) return -1;
    initPlayer();

    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {
        Uint32 frameStart = SDL_GetTicks();
        keyboardState = SDL_GetKeyboardState(NULL);

        // Xử lý sự kiện
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) { isRunning = false; break; }
            if (event.type != SDL_KEYDOWN) continue;
            SDL_Keycode key = event.key.keysym.sym;

            if (key == SDLK_ESCAPE) {
                if (currentState == GameState::PLAYING)      currentState = GameState::PAUSED;
                else if (currentState == GameState::PAUSED)  currentState = GameState::PLAYING;
            }

            if (key == SDLK_SPACE || key == SDLK_RETURN) {
                if (currentState == GameState::MENU) {
                    restartGame();
                } else if (currentState == GameState::GAMEOVER || currentState == GameState::GAME_WON) {
                    bullets.clear(); enemies.clear(); pickups.clear(); floating_texts.clear();
                    currentState = GameState::MENU;
                }
            }
        }

        // Cập nhật trạng thái game
        if (currentState == GameState::PLAYING) {
            bgScrollY += 0.5f;

            if (transitionStart > 0 && SDL_GetTicks() - transitionStart > 5000) {
                currentStage++;
                currentWave = 1; enemyKillCount = 0;
                spawnTimer = 0; spawnedSmall = 0; spawnedMedium = 0;
                transitionStart = 0;
                waveEnemyTarget = calcWaveTarget();
                bullets.clear(); enemies.clear(); pickups.clear(); floating_texts.clear();
            }

            updatePlayer();
            updateBullets();
            updateEnemies();
            updatePickups();
            updateFloatingTexts();
            checkBulletEnemyCollisions();
            checkPlayerCollisions();
            cleanupVectors();
            updateSpawner();
            checkWaveProgress();
        }

        render();

        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < 16) SDL_Delay(16 - frameTime);
    }

    // Dọn dẹp tài nguyên
    TTF_CloseFont(font);
    SDL_Texture* allTextures[] = {
        texPlayer[0], texPlayer[1], texPlayer[2], texPlayer[3],
        texWeaponAuto, texWeaponRocket,
        texIconAuto, texIconRocket, texIconShield, texPlayerShield,
        texEnemyBullet, texEnemySmallBase, texEnemySmallDest,
        texEnemyMediumBase, texEnemyMediumDest,
        texEnemyLargeBase, texEnemyLargeDest, texBackground
    };
    for (int i = 0; i < (int)(sizeof(allTextures)/sizeof(allTextures[0])); i++)
        if (allTextures[i]) SDL_DestroyTexture(allTextures[i]);
    if (bgmMusic) Mix_FreeMusic(bgmMusic);
    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit(); IMG_Quit(); SDL_Quit();
    return 0;
}
