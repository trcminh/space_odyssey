// ============================================================
//  render.cpp — Toàn bộ logic vẽ lên màn hình
// ============================================================
#include "render.h"
#include <string>
#include <algorithm>

SDL_Texture* loadText(const std::string& text, SDL_Color color) {
    if (!font) {
        return nullptr;
    }
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!surface) {
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void renderText(const std::string& text, SDL_Color color, int x, int y, float scale) {
    SDL_Texture* texture = loadText(text, color);
    if (!texture) {
        return;
    }
    int w = 0, h = 0;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    SDL_Rect dest = {x, y, (int)(w * scale), (int)(h * scale)};
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    SDL_DestroyTexture(texture);
}

void renderTextCentered(const std::string& text, SDL_Color color, int y, float scale) {
    SDL_Texture* texture = loadText(text, color);
    if (!texture) {
        return;
    }
    int w = 0, h = 0;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    int scaledW = (int)(w * scale);
    SDL_Rect dest = {SCREEN_WIDTH / 2 - scaledW / 2, y, scaledW, (int)(h * scale)};
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    SDL_DestroyTexture(texture);
}

static void renderMenu() {
    renderTextCentered("SPACE ODYSSEY", {255, 230, 100, 255}, 220, 2.0f);
    if ((SDL_GetTicks() / 500) % 2 == 0)
        renderTextCentered("Press SPACE to Start", {255, 255, 255, 255}, 330, 1.0f);
    renderTextCentered("HIGH SCORE: " + std::to_string(highScore), {50, 255, 100, 255}, 380, 1.0f);
}

static void renderGameOver() {
    renderTextCentered("GAME OVER!", {255, 50, 50, 255}, 250, 2.0f);
    renderTextCentered("Final Score: " + std::to_string(player.score), {255, 255, 255, 255}, 350, 1.0f);
    if ((SDL_GetTicks() / 500) % 2 == 0)
        renderTextCentered("Press SPACE to Restart", {150, 150, 150, 255}, 450, 1.0f);
}

static void renderGameWon() {
    renderTextCentered("CLEARED!", {255, 230, 50, 255}, 250, 2.5f);
    renderTextCentered("Total Score: " + std::to_string(player.score), {255, 255, 255, 255}, 350, 1.0f);
    if ((SDL_GetTicks() / 500) % 2 == 0)
        renderTextCentered("Press SPACE to Return to Menu", {150, 150, 150, 255}, 450, 1.0f);
}



static void renderEntities() {
    for (int i = 0; i < (int)bullets.size(); i++) {
        const Bullet& b = bullets[i];
        if (!b.active) {
            continue;
        }
        SDL_Rect src = {b.anim.current_frame * b.w, 0, b.w, b.h};
        SDL_Rect dest = {(int)b.x, (int)b.y, (int)(b.w * b.scale), (int)(b.h * b.scale)};
        SDL_RenderCopy(renderer, b.texture, &src, &dest);
    }
    for (int i = 0; i < (int)pickups.size(); i++) {
        const Pickup& p = pickups[i];
        if (!p.active) {
            continue;
        }
        SDL_Rect src = {p.anim.current_frame * p.w, 0, p.w, p.h};
        SDL_Rect dest = {(int)p.x, (int)p.y, PICKUP_SIZE, PICKUP_SIZE};
        SDL_RenderCopy(renderer, p.texture, &src, &dest);
    }
    for (int i = 0; i < (int)enemies.size(); i++) {
        const Enemy& e = enemies[i];
        if (!e.active) {
            continue;
        }
        SDL_Rect dest = {(int)e.x, (int)e.y, e.w, e.h};
        if (e.state == EnemyState::ALIVE) {
            SDL_RenderCopy(renderer, e.texture_base, NULL, &dest);
        } else if (e.texture_destruction != nullptr) {
            int tw, th;
            SDL_QueryTexture(e.texture_destruction, NULL, NULL, &tw, &th);
            SDL_Rect src = {e.anim.current_frame * th, 0, th, th};
            dest.w = dest.h = th;
            SDL_RenderCopy(renderer, e.texture_destruction, &src, &dest);
        }
    }
}

static void renderPlayer() {
    if (player.is_dead) {
        return;
    }
    bool shouldBlink = player.is_invulnerable && ((SDL_GetTicks() - player.invuln_start_time) / 150) % 2 == 0;
    if (shouldBlink) {
        return;
    }
    
    int hpIdx = player.lives;
    if (hpIdx < 0) {
        hpIdx = 0;
    }
    if (hpIdx > INITIAL_LIVES) {
        hpIdx = INITIAL_LIVES;
    }

    SDL_Rect dest = {(int)player.x, (int)player.y, PLAYER_SIZE, PLAYER_SIZE};
    SDL_RenderCopy(renderer, texPlayer[hpIdx], NULL, &dest);
    
    SDL_Rect eSrc = {player.engine_anim.current_frame * 48, 0, 48, 48};
    SDL_Rect eDest = {(int)player.x, (int)player.y + 5, PLAYER_SIZE, PLAYER_SIZE};
    SDL_RenderCopy(renderer, texEngineFlame, &eSrc, &eDest);
    
    if (player.has_shield && texPlayerShield) {
        SDL_Rect sSrc = {player.shield_anim.current_frame * player.shield_w, 0, player.shield_w, player.shield_h};
        SDL_Rect sDest = {(int)player.x - 10, (int)player.y - 10, 84, 84};
        SDL_RenderCopy(renderer, texPlayerShield, &sSrc, &sDest);
    }
}

static void renderFloatingTexts() {
    for (int i = 0; i < (int)floating_texts.size(); i++) {
        const FloatingText& ft = floating_texts[i];
        if (!ft.active) {
            continue;
        }
        SDL_Texture* t = (ft.score == 50) ? texScore50 : (ft.score == 70) ? texScore70 : texScore100;
        if (!t) {
            continue;
        }
        SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(t, (Uint8)ft.alpha);
        int tw, th;
        SDL_QueryTexture(t, NULL, NULL, &tw, &th);
        SDL_Rect dest = {(int)ft.x, (int)ft.y, tw, th};
        SDL_RenderCopy(renderer, t, NULL, &dest);
    }
}

static void renderHUD() {
    renderText("SCORE: " + std::to_string(player.score), {255, 255, 255, 255}, 10, 10, 1.0f);
    renderText(std::to_string(currentStage) + "-" + std::to_string(currentWave), {255, 230, 100, 255}, SCREEN_WIDTH - 45, 10, 1.0f);
    renderText("POWER LV " + std::to_string(player.weapon_level), {100, 255, 255, 255}, 10, SCREEN_HEIGHT - 35, 1.0f);
    renderText("LIVES: " + std::to_string(player.lives), {255, 80, 80, 255}, SCREEN_WIDTH - 95, SCREEN_HEIGHT - 35, 1.0f);
}

void render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

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
        case GameState::PLAYING:
            renderEntities();
            renderPlayer();
            renderFloatingTexts();
            if (transitionStart > 0) {
                renderTextCentered("STAGE " + std::to_string(currentStage + 1), {100, 255, 100, 255}, 300, 2.0f);
            }
            renderHUD();
            break;
    }
    SDL_RenderPresent(renderer);
}
