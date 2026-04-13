#include "game.h"

// Vẽ text tại vị trí (x, y)
static void renderText(const std::string& text, SDL_Color color, int x, int y, float scale = 1.0f) {
    SDL_Texture* texture = loadText(text, color);
    if (!texture) {
        return;
    }
    int w, h;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    SDL_Rect dest = {x, y, (int)(w * scale), (int)(h * scale)};
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    SDL_DestroyTexture(texture);
}

// Vẽ text căn giữa màn hình
static void renderTextCentered(const std::string& text, SDL_Color color, int y, float scale = 1.0f) {
    SDL_Texture* texture = loadText(text, color);
    if (!texture) {
        return;
    }
    int w, h;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    int sw = (int)(w * scale);
    SDL_Rect dest = {SCREEN_WIDTH / 2 - sw / 2, y, sw, (int)(h * scale)};
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    SDL_DestroyTexture(texture);
}

// Vẽ nền tile cuộn
static void renderBackground() {
    if (!texBackground) {
        return;
    }
    int tw, th;
    SDL_QueryTexture(texBackground, NULL, NULL, &tw, &th);
    if (tw <= 0 || th <= 0) {
        return;
    }
    for (int y = ((int)bgScrollY % th) - th; y < SCREEN_HEIGHT; y += th) {
        for (int x = 0; x < SCREEN_WIDTH; x += tw) {
            SDL_Rect dest = {x, y, tw, th};
            SDL_RenderCopy(renderer, texBackground, NULL, &dest);
        }
    }
}

static void renderMenu() {
    renderTextCentered("SPACE ODYSSEY", {255, 230, 100, 255}, 220, 2.0f);
    if ((SDL_GetTicks() / 500) % 2 == 0) {
        renderTextCentered("Press SPACE to Start", {255, 255, 255, 255}, 330, 1.0f);
    }
    renderTextCentered("HIGH SCORE: " + std::to_string(highScore), {50, 255, 100, 255}, 380, 1.0f);
}

static void renderGameOver() {
    renderTextCentered("GAME OVER!", {255, 50, 50, 255}, 250, 2.0f);
    renderTextCentered("Final Score: " + std::to_string(player.score), {255, 255, 255, 255}, 350, 1.0f);
    if ((SDL_GetTicks() / 500) % 2 == 0) {
        renderTextCentered("Press SPACE to Restart", {150, 150, 150, 255}, 450, 1.0f);
    }
}

static void renderGameWon() {
    renderTextCentered("CLEARED!", {255, 230, 50, 255}, 250, 2.5f);
    renderTextCentered("Total Score: " + std::to_string(player.score), {255, 255, 255, 255}, 350, 1.0f);
    if ((SDL_GetTicks() / 500) % 2 == 0) {
        renderTextCentered("Press SPACE to Return to Menu", {150, 150, 150, 255}, 450, 1.0f);
    }
}

static void renderPaused() {
    renderTextCentered("PAUSED", {255, 255, 255, 255}, 350, 2.5f);
    if ((SDL_GetTicks() / 500) % 2 == 0) {
        renderTextCentered("Press ESC to Resume", {150, 150, 150, 255}, 420, 1.0f);
    }
}

// Vẽ đạn, vật phẩm, quái
static void renderEntities() {
    for (const Bullet& b : bullets) {
        if (!b.active) {
            continue;
        }
        SDL_Rect src = {0, 0, b.w, b.h};
        SDL_Rect dest = {(int)b.x, (int)b.y, (int)(b.w * b.scale), (int)(b.h * b.scale)};
        SDL_RenderCopy(renderer, b.texture, &src, &dest);
    }

    for (const Pickup& p : pickups) {
        if (!p.active) {
            continue;
        }
        SDL_Rect src = {p.anim.current_frame * p.w, 0, p.w, p.h};
        SDL_Rect dest = {(int)p.x, (int)p.y, PICKUP_SIZE, PICKUP_SIZE};
        SDL_RenderCopy(renderer, p.texture, &src, &dest);
    }

    for (const Enemy& e : enemies) {
        if (!e.active) {
            continue;
        }
        SDL_Rect dest = {(int)e.x, (int)e.y, e.w, e.h};
        if (e.state == EnemyState::ALIVE) {
            SDL_RenderCopy(renderer, e.texture_base, NULL, &dest);
        } else if (e.texture_destruction) {
            int tw, th;
            SDL_QueryTexture(e.texture_destruction, NULL, NULL, &tw, &th);
            SDL_Rect src = {e.anim.current_frame * th, 0, th, th};
            dest.w = th;
            dest.h = th;
            SDL_RenderCopy(renderer, e.texture_destruction, &src, &dest);
        }
    }
}

// Vẽ tàu người chơi
static void renderPlayer() {
    if (player.is_dead) {
        return;
    }
    if (player.is_invulnerable) {
        Uint32 elapsed = SDL_GetTicks() - player.invuln_start_time;
        if ((elapsed / 150) % 2 == 0) {
            return;
        }
    }
    int hpIdx = std::max(0, std::min(player.lives, INITIAL_LIVES));
    SDL_Rect dest = {(int)player.x, (int)player.y, PLAYER_SIZE, PLAYER_SIZE};
    SDL_RenderCopy(renderer, texPlayer[hpIdx], NULL, &dest);

    if (player.has_shield && texPlayerShield) {
        int sw, sh;
        SDL_QueryTexture(texPlayerShield, NULL, NULL, &sw, &sh);
        int fw = sh;
        SDL_Rect sSrc = {player.shield_anim.current_frame * fw, 0, fw, sh};
        SDL_Rect sDest = {(int)player.x - 10, (int)player.y - 10, 84, 84};
        SDL_RenderCopy(renderer, texPlayerShield, &sSrc, &sDest);
    }
}

// Vẽ chữ nổi điểm số
static void renderFloatingTexts() {
    for (const FloatingText& ft : floating_texts) {
        if (!ft.active) {
            continue;
        }
        SDL_Texture* tex = loadText("+" + std::to_string(ft.score), {255, 255, 100, 255});
        if (!tex) {
            continue;
        }
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(tex, (Uint8)ft.alpha);
        int tw, th;
        SDL_QueryTexture(tex, NULL, NULL, &tw, &th);
        SDL_Rect dest = {(int)ft.x, (int)ft.y, tw, th};
        SDL_RenderCopy(renderer, tex, NULL, &dest);
        SDL_DestroyTexture(tex);
    }
}

// Vẽ HUD
static void renderHUD() {
    renderText("SCORE: " + std::to_string(player.score),
               {255, 255, 255, 255}, 10, 10);
    renderText(std::to_string(currentStage) + "-" + std::to_string(currentWave),
               {255, 230, 100, 255}, SCREEN_WIDTH - 45, 10);
    renderText("POWER LV " + std::to_string(player.weapon_level),
               {100, 255, 255, 255}, 10, SCREEN_HEIGHT - 35);
    renderText("LIVES: " + std::to_string(player.lives),
               {255, 80, 80, 255}, SCREEN_WIDTH - 95, SCREEN_HEIGHT - 35);
}

// Vẽ toàn bộ 1 frame
void render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    renderBackground();

    switch (currentState) {
        case GameState::MENU:
            renderMenu();
            break;
        case GameState::GAMEOVER:
            renderGameOver();
            break;
        case GameState::GAME_WON:
            renderGameWon();
            break;
        case GameState::PAUSED:
            renderEntities();
            renderPlayer();
            renderFloatingTexts();
            renderHUD();
            renderPaused();
            break;
        case GameState::PLAYING:
            renderEntities();
            renderPlayer();
            renderFloatingTexts();
            if (transitionStart > 0) {
                renderTextCentered("STAGE " + std::to_string(currentStage + 1),
                                   {100, 255, 100, 255}, 300, 2.0f);
            }
            renderHUD();
            break;
    }
    SDL_RenderPresent(renderer);
}
