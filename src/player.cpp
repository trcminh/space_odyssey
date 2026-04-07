#include "player.h"
#include "globals.h"
#include "weapon.h"
#include "update.h" // for updateAnim
#include <SDL_mixer.h>
#include <algorithm>

void updatePlayer() {
    Uint32 now = SDL_GetTicks();

    if (player.is_dead) {
        if (now - player.death_time > RESPAWN_DELAY) {
            player.is_dead = false;
            player.is_respawning = true;
            player.is_invulnerable = true;
            player.invuln_start_time = now;
            player.x = (float)(SCREEN_WIDTH / 2 - PLAYER_SIZE / 2);
            player.y = (float)(SCREEN_HEIGHT + PLAYER_SIZE);
        }
        return;
    }

    if (player.is_respawning) {
        player.y -= 3.0f;
        if (player.y <= SCREEN_HEIGHT - 100) {
            player.y = (float)(SCREEN_HEIGHT - 100);
            player.is_respawning = false;
        }
        updateAnim(player.engine_anim, now, 100);
        return;
    }

    if (player.has_shield && now - player.shield_start_time > SHIELD_TIME) {
        player.has_shield = false;
    }
    if (player.has_shield) {
        updateAnim(player.shield_anim, now, 50);
    }
    if (player.is_invulnerable && now - player.invuln_start_time > INVULN_TIME) {
        player.is_invulnerable = false;
    }

    if (keyboardState[SDL_SCANCODE_LEFT]  && player.x > 0) {
        player.x -= PLAYER_SPEED;
    }
    if (keyboardState[SDL_SCANCODE_RIGHT] && player.x < SCREEN_WIDTH-PLAYER_SIZE) {
        player.x += PLAYER_SPEED;
    }
    if (keyboardState[SDL_SCANCODE_UP]    && player.y > 0) {
        player.y -= PLAYER_SPEED;
    }
    if (keyboardState[SDL_SCANCODE_DOWN]  && player.y < SCREEN_HEIGHT-PLAYER_SIZE) {
        player.y += PLAYER_SPEED;
    }

    float half = PLAYER_SIZE / 2.0f;
    player.hitbox_x = player.x + half - player.hitbox_w / 2.0f;
    player.hitbox_y = player.y + half - player.hitbox_h / 2.0f;

    updateAnim(player.engine_anim, now, 100);

    if (keyboardState[SDL_SCANCODE_SPACE]) {
        Uint32 delay = (player.current_weapon == WeaponType::AUTO_CANNON) ? 150 : 200;
        if (now - player.last_shoot_time > delay) {
            firePlayerWeapon(player.x + 32, player.y, player.current_weapon, player.weapon_level);
            player.last_shoot_time = now;
        }
    }
}

void damagePlayer() {
    if (GOD_MODE) {
        return;
    }
    if (player.has_shield) {
        player.has_shield = false;
        return;
    }

    player.is_dead = true;
    player.death_time = SDL_GetTicks();
    player.weapon_level = std::max(1, player.weapon_level - 1);

    if (--player.lives < 0) {
        Mix_HaltMusic();
        if (player.score > highScore) {
            highScore = player.score;
        }
        currentState = GameState::GAMEOVER;
    }
}
