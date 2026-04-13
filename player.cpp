#include "game.h"

// Cập nhật player mỗi frame
void updatePlayer() {
    Uint32 now = SDL_GetTicks();

    if (player.is_dead) {
        if (now - player.death_time > RESPAWN_DELAY) {
            player.is_dead = false;
            player.is_invulnerable = true;
            player.invuln_start_time = now;
            player.x = (float)(SCREEN_WIDTH / 2 - PLAYER_SIZE / 2);
            player.y = (float)(SCREEN_HEIGHT - 100);
        }
        return;
    }

    if (player.has_shield && now - player.shield_start_time > SHIELD_TIME)
        player.has_shield = false;
    if (player.has_shield)
        updateAnim(player.shield_anim, now, 50);
    if (player.is_invulnerable && now - player.invuln_start_time > INVULN_TIME)
        player.is_invulnerable = false;

    // Di chuyển
    if (keyboardState[SDL_SCANCODE_LEFT]  && player.x > 0)                         player.x -= PLAYER_SPEED;
    if (keyboardState[SDL_SCANCODE_RIGHT] && player.x < SCREEN_WIDTH - PLAYER_SIZE) player.x += PLAYER_SPEED;
    if (keyboardState[SDL_SCANCODE_UP]    && player.y > 0)                         player.y -= PLAYER_SPEED;
    if (keyboardState[SDL_SCANCODE_DOWN]  && player.y < SCREEN_HEIGHT - PLAYER_SIZE) player.y += PLAYER_SPEED;

    // Bắn
    if (keyboardState[SDL_SCANCODE_SPACE]) {
        Uint32 delay = (player.current_weapon == WeaponType::AUTO_CANNON) ? 150 : 200;
        if (now - player.last_shoot_time > delay) {
            firePlayerWeapon(player.x + 32, player.y, player.current_weapon, player.weapon_level);
            player.last_shoot_time = now;
        }
    }
}

// Xử lý khi player bị trúng đạn/quái
void damagePlayer() {
    if (GOD_MODE) return;
    if (player.has_shield) { player.has_shield = false; return; }

    player.is_dead = true;
    player.death_time = SDL_GetTicks();
    player.weapon_level = std::max(1, player.weapon_level - 1);
    if (--player.lives < 0) {
        Mix_HaltMusic();
        if (player.score > highScore) highScore = player.score;
        currentState = GameState::GAMEOVER;
    }
}

// Tạo đạn theo vũ khí và cấp độ
void firePlayerWeapon(float cx, float y, WeaponType wpn, int lvl) {
    if (wpn == WeaponType::AUTO_CANNON) {
        if (lvl == 1) {
            bullets.push_back(createBullet(texWeaponAuto, cx-16, y, 0,-15, 4, true));
        } else if (lvl == 2) {
            bullets.push_back(createBullet(texWeaponAuto, cx-32, y, 0,-15, 4, true));
            bullets.push_back(createBullet(texWeaponAuto, cx,    y, 0,-15, 4, true));
        } else {
            bullets.push_back(createBullet(texWeaponAuto, cx-16, y-12, 0,-15, 4, true));
            bullets.push_back(createBullet(texWeaponAuto, cx-48, y+12, 0,-15, 4, true));
            bullets.push_back(createBullet(texWeaponAuto, cx+16, y+12, 0,-15, 4, true));
        }
    }
    else if (wpn == WeaponType::ROCKET) {
        bullets.push_back(createBullet(texWeaponRocket, cx-16, y,     0,    -10, 2, true));
        bullets.push_back(createBullet(texWeaponRocket, cx-32, y+10, -2.5f,  -9, 2, true));
        bullets.push_back(createBullet(texWeaponRocket, cx,    y+10,  2.5f,  -9, 2, true));
        if (lvl >= 2) {
            bullets.push_back(createBullet(texWeaponRocket, cx-44, y+15, -3.5f, -8.5f, 2, true));
            bullets.push_back(createBullet(texWeaponRocket, cx+12, y+15,  3.5f, -8.5f, 2, true));
        }
        if (lvl >= 3) {
            bullets.push_back(createBullet(texWeaponRocket, cx-48, y+20, -4.5f, -8, 2, true));
            bullets.push_back(createBullet(texWeaponRocket, cx+16, y+20,  4.5f, -8, 2, true));
        }
    }
}

// Va chạm liên quan đến player
void checkPlayerCollisions() {
    if (player.is_dead || player.is_invulnerable) return;
    // Hitbox kiểu Touhou: điểm nhỏ 4x4 ở giữa tàu
    float hx = player.x + PLAYER_SIZE / 2.0f - 2;
    float hy = player.y + PLAYER_SIZE / 2.0f - 2;

    // Đạn quái → player
    for (Bullet& b : bullets) {
        if (!b.active || b.is_player_bullet) continue;
        if (AABB(b.x, b.y, b.w, b.h, hx, hy, 4, 4)) {
            b.active = false;
            damagePlayer();
            return;
        }
    }

    // Quái đâm player
    for (Enemy& e : enemies) {
        if (!e.active || e.state != EnemyState::ALIVE) continue;
        float ex = e.x + e.w / 2.0f - 2, ey = e.y + e.h / 2.0f - 2;
        if (AABB(hx, hy, 4, 4, ex, ey, 4, 4)) {
            damagePlayer();
            handleEnemyDeath(e);
            return;
        }
    }

    // Player nhặt pickup
    for (Pickup& p : pickups) {
        if (!p.active) continue;
        if (!AABB(player.x, player.y, PLAYER_SIZE, PLAYER_SIZE, p.x, p.y, p.w, p.h)) continue;
        if (p.type == PickupType::WEAPON) {
            player.current_weapon = p.weapon_type;
            player.weapon_level   = std::min(3, player.weapon_level + 1);
        } else {
            player.has_shield = true;
            player.shield_start_time = SDL_GetTicks();
        }
        player.score += SCORE_PICKUP;
        p.active = false;
        floating_texts.push_back({p.x, p.y - 10, SCORE_PICKUP, 255.0f, true});
    }
}
