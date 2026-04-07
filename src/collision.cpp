#include "collision.h"
#include "globals.h"
#include "enemy.h"
#include "player.h"
#include <algorithm>

bool AABB(float x1, float y1, float w1, float h1,
          float x2, float y2, float w2, float h2)
{
    return !(x1+w1 <= x2 || x1 >= x2+w2 || y1+h1 <= y2 || y1 >= y2+h2);
}

void getEnemyHitbox(const Enemy& e, float& ex, float& ey, float& ew, float& eh) {
    float pad = (e.size_class == EnemySize::SMALL)  ? 0.25f :
                (e.size_class == EnemySize::MEDIUM) ? 0.20f : 0.10f;
    float siz = (e.size_class == EnemySize::SMALL)  ? 0.50f :
                (e.size_class == EnemySize::MEDIUM) ? 0.60f : 0.80f;
    ex = e.x + e.w * pad;
    ey = e.y + e.h * pad;
    ew = e.w * siz;
    eh = e.h * siz;
}

void checkCollisions() {
    // 1. Đạn player → quái
    for (Bullet& b : bullets) {
        if (!b.active || !b.is_player_bullet) {
            continue;
        }
        float bw = b.w * b.scale, bh = b.h * b.scale;
        float bhx = b.x + bw/4, bhy = b.y + bh/4;

        for (Enemy& e : enemies) {
            if (!e.active || e.state != EnemyState::ALIVE) {
                continue;
            }
            float ex, ey, ew, eh;
            getEnemyHitbox(e, ex, ey, ew, eh);
            if (!AABB(bhx, bhy, bw/2, bh/2, ex, ey, ew, eh)) {
                continue;
            }
            b.active = false;
            e.hp -= b.damage;
            if (e.hp <= 0) {
                handleEnemyDeath(e);
            }
            break;
        }
    }

    if (player.is_dead || player.is_invulnerable || player.is_respawning) {
        return;
    }

    // 2. Đạn quái → player
    for (Bullet& b : bullets) {
        if (!b.active || b.is_player_bullet) {
            continue;
        }
        float bw = b.w * b.scale, bh = b.h * b.scale;
        if (AABB(b.x+bw/4, b.y+bh/4, bw/2, bh/2,
                 player.hitbox_x, player.hitbox_y, player.hitbox_w, player.hitbox_h)) {
            b.active = false;
            damagePlayer();
            break;
        }
    }

    // 3. Quái đâm player
    for (Enemy& e : enemies) {
        if (!e.active || e.state != EnemyState::ALIVE) {
            continue;
        }
        float ex, ey, ew, eh;
        getEnemyHitbox(e, ex, ey, ew, eh);
        if (AABB(player.x, player.y, PLAYER_SIZE, PLAYER_SIZE, ex, ey, ew, eh)) {
            damagePlayer();
            triggerExplosion(e);
            break;
        }
    }

    // 4. Player nhặt pickup
    for (Pickup& p : pickups) {
        if (!p.active) {
            continue;
        }
        if (!AABB(player.x, player.y, PLAYER_SIZE, PLAYER_SIZE, p.x, p.y, p.w, p.h)) {
            continue;
        }
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
