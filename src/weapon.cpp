#include "weapon.h"
#include "globals.h"
#include "update.h" // For countFrames, updateAnim
#include <cmath>
#include <algorithm>

static const float PI = 3.14159f;

Bullet createBullet(SDL_Texture* tex, float x, float y,
                    float vx, float vy, int damage, bool isPlayer)
{
    Bullet b;
    b.texture = tex;
    b.x = x;
    b.y = y;
    b.vx = vx;
    b.vy = vy;
    b.damage = damage;
    b.is_player_bullet = isPlayer;
    b.active = true;
    b.scale = 1.0f;
    b.w = 16;
    b.h = 16;
    b.anim = {0, 1, SDL_GetTicks()};
    if (tex) {
        int sw, sh;
        SDL_QueryTexture(tex, NULL, NULL, &sw, &sh);
        b.w = b.h = sh;
        b.anim.total_frames = countFrames(tex);
    }
    return b;
}

// Bắn 1 viên đạn theo góc (degree)
static void shootAngle(SDL_Texture* tex, float cx, float cy,
                       float angleDeg, float speed, bool isPlayer = false)
{
    float rad = angleDeg * PI / 180.0f;
    bullets.push_back(createBullet(tex, cx - 8, cy,
        std::cos(rad) * speed, std::sin(rad) * speed, 1, isPlayer));
}

void firePlayerWeapon(float cx, float y, WeaponType wpn, int lvl) {
    if (wpn == WeaponType::AUTO_CANNON) {
        if (lvl == 1) {
            bullets.push_back(createBullet(texWeaponAuto, cx-16, y,  0,-15,2,true));
        } else if (lvl == 2) {
            bullets.push_back(createBullet(texWeaponAuto, cx-32, y,  0,-15,2,true));
            bullets.push_back(createBullet(texWeaponAuto, cx,    y,  0,-15,2,true));
        } else {
            bullets.push_back(createBullet(texWeaponAuto, cx-16, y-12, 0,-15,4,true));
            bullets.push_back(createBullet(texWeaponAuto, cx-48, y+12, 0,-15,4,true));
            bullets.push_back(createBullet(texWeaponAuto, cx+16, y+12, 0,-15,4,true));
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

void fireEnemyBullets(float cx, float cy, SDL_Texture* tex,
                      AttackType type, float speed)
{
    if (type == AttackType::AIMED) {
        float dx = (player.x + PLAYER_SIZE / 2.0f) - cx;
        float dy = (player.y + PLAYER_SIZE / 2.0f) - cy;
        float len = std::sqrt(dx*dx + dy*dy);
        if (len > 0.001f) {
            bullets.push_back(createBullet(tex, cx-8, cy,
                (dx/len)*speed, (dy/len)*speed, 1, false));
        }
    }
    else if (type == AttackType::N_WAY) {
        for (int off = -1; off <= 1; off++) {
            shootAngle(tex, cx, cy, 90.0f + off * 20.0f, speed);
        }
    }
    else { // BURST_360
        for (int deg = 0; deg < 360; deg += 30) {
            shootAngle(tex, cx, cy, (float)deg, speed);
        }
    }
}

void updateBullets() {
    Uint32 now = SDL_GetTicks();
    for (Bullet& b : bullets) {
        if (!b.active) {
            continue;
        }
        b.x += b.vx; b.y += b.vy;
        if (b.is_player_bullet && b.vy < -7.0f) {
            b.vy -= 0.5f;
        }
        updateAnim(b.anim, now, 50);
        if (b.x < -OFFSCREEN_MARGIN || b.x > SCREEN_WIDTH + OFFSCREEN_MARGIN ||
            b.y < -OFFSCREEN_MARGIN || b.y > SCREEN_HEIGHT + OFFSCREEN_MARGIN) {
            b.active = false;
        }
    }
}
