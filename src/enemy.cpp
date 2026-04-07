#include "enemy.h"
#include "globals.h"
#include "weapon.h"
#include "update.h" // for updateAnim, countFrames
#include <algorithm>
#include <cmath>
#include <SDL_mixer.h> // If needed later, though mix not used directly here except by globals implicitly? Wait, death doesn't play sound.

void spawnEnemy(int sizeIdx) {
    if (texEnemyBullet == nullptr) {
        return;
    }

    Enemy e;
    e.active = true;
    e.state = EnemyState::ALIVE;
    e.last_shoot_time = SDL_GetTicks();
    e.anim = {0, 1, 0};
    e.texture_bullet = texEnemyBullet;

    float hpMul = 1.0f + (currentStage - 1) * 0.5f;

    if (sizeIdx == 0) {
        e.size_class = EnemySize::SMALL;
        e.texture_base = texEnemySmallBase;
        e.texture_destruction = texEnemySmallDest;
        e.hp = e.max_hp = (int)(4 * hpMul);
        e.shoot_delay = 2300;
        e.score_value = SCORE_ENEMY_SMALL;
        e.drop_chance = 20;
    } else if (sizeIdx == 1) {
        e.size_class = EnemySize::MEDIUM;
        e.texture_base = texEnemyMediumBase;
        e.texture_destruction = texEnemyMediumDest;
        e.hp = e.max_hp = (int)(14 * hpMul);
        e.shoot_delay = 1800;
        e.score_value = SCORE_ENEMY_MEDIUM;
        e.drop_chance = 30;
    } else {
        e.size_class = EnemySize::LARGE;
        e.texture_base = texEnemyLargeBase;
        e.texture_destruction = texEnemyLargeDest;
        e.hp = e.max_hp = (int)(600 * hpMul);
        e.shoot_delay = 600;
        e.score_value = SCORE_ENEMY_LARGE;
        e.drop_chance = 100;
    }

    e.w = 64;
    e.h = 64;
    if (e.texture_base) {
        SDL_QueryTexture(e.texture_base, NULL, NULL, &e.w, &e.h);
    }

    e.x = (float)(rand() % std::max(1, SCREEN_WIDTH - e.w));
    e.y = (float)(-e.h);
    e.start_x = e.x;
    e.start_y = e.y;
    e.spawn_time = SDL_GetTicks();
    e.target_y = 100.0f + (float)(rand() % 100);
    if (e.size_class == EnemySize::MEDIUM) {
        e.target_y = SCREEN_HEIGHT / 3.0f;
    }
    enemies.push_back(e);
}

void updateSpawner() {
    if (transitionStart > 0) {
        return;
    }

    int cooldown = std::max(30, 100 - currentWave * 5);
    if (++spawnTimer < cooldown) {
        return;
    }
    spawnTimer = 0;

    bool canSpawnSmall = (spawnedSmall < currentWave);
    bool canSpawnMedium = (spawnedMedium < currentWave);

    if (!canSpawnSmall && !canSpawnMedium) {
        return;
    }

    if (canSpawnSmall && canSpawnMedium) {
        if (rand() % 2 == 0) {
            spawnEnemy(1); // Medium
            spawnedMedium++;
        } else {
            spawnEnemy(0); // Small
            spawnedSmall++;
        }
    } else if (canSpawnSmall) {
        spawnEnemy(0);
        spawnedSmall++;
    } else {
        spawnEnemy(1);
        spawnedMedium++;
    }
}

static void moveStopAndShoot(Enemy& e, Uint32 now) {
    if (e.move_phase == 0) {
        e.y += ENEMY_SPEED;
        if (e.y >= e.target_y) {
            e.y = e.target_y;
            e.move_phase = 1; // Stay here forever
        }
    }
}

void updateEnemies() {
    Uint32 now = SDL_GetTicks();
    for (Enemy& e : enemies) {
        if (!e.active) {
            continue;
        }

        if (e.state == EnemyState::EXPLODING) {
            if (updateAnim(e.anim, now, 80)) {
                e.active = false;
            }
            continue;
        }

        moveStopAndShoot(e, now);

        if (!e.active || now - e.last_shoot_time <= e.shoot_delay) {
            continue;
        }

        float cx = e.x + e.w / 2.0f;
        float cy = e.y + e.h;

        if (e.move_phase == 1) {
            fireEnemyBullets(cx, cy, e.texture_bullet, AttackType::BURST_360, 2.5f);
        } else if (e.size_class == EnemySize::MEDIUM) {
            fireEnemyBullets(cx, cy, e.texture_bullet, AttackType::N_WAY, 3.5f);
        } else {
            fireEnemyBullets(cx, cy, e.texture_bullet, AttackType::AIMED, 4.0f);
        }
        e.last_shoot_time = now;
    }
}

void triggerExplosion(Enemy& e) {
    e.state = EnemyState::EXPLODING;
    e.anim  = {0, (e.texture_destruction ? countFrames(e.texture_destruction) : 1), SDL_GetTicks()};
}

void handleEnemyDeath(Enemy& e) {
    triggerExplosion(e);
    player.score += e.score_value;
    enemyKillCount++;

    if (enemyKillCount >= waveEnemyTarget) {
        if (currentWave == 15) {
            if (currentStage >= 3) {
                if (player.score > highScore) {
                    highScore = player.score;
                }
                currentState = GameState::GAME_WON;
            } else {
                transitionStart = SDL_GetTicks();
            }
        } else {
            currentWave++;
            enemyKillCount  = 0;
            waveEnemyTarget = currentWave * 2;
            spawnedSmall    = 0;
            spawnedMedium   = 0;
        }
    }

    floating_texts.push_back({e.x + e.w/2.0f - 20, e.y + e.h/2.0f - 10,
                               e.score_value, 255.0f, true});

    if (rand() % 100 >= e.drop_chance) {
        return;
    }

    Pickup p;
    p.x = e.x + e.w/2.0f - 16;
    p.y = e.y + e.h/2.0f;
    p.vy = PICKUP_SPEED;
    p.active = true;
    p.anim = {0, 1, SDL_GetTicks()};

    if (rand() % 100 < 20) {
        p.type = PickupType::SHIELD;
        p.texture = texIconShield;
    } else {
        p.type = PickupType::WEAPON;
        int roll = rand() % 2;
        p.weapon_type = static_cast<WeaponType>(roll);
        SDL_Texture* icons[] = {texIconAuto, texIconRocket};
        p.texture = icons[roll];
    }
    p.anim.total_frames = countFrames(p.texture);
    SDL_QueryTexture(p.texture, NULL, NULL, &p.w, &p.h);
    p.w = p.h;
    pickups.push_back(p);
}
