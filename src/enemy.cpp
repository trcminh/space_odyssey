#include "game.h"

static const float PI = 3.14159f;

// Kiểm tra va chạm hình chữ nhật
bool AABB(float x1, float y1, float w1, float h1,
          float x2, float y2, float w2, float h2) {
    return !(x1 + w1 <= x2 || x1 >= x2 + w2 ||
             y1 + h1 <= y2 || y1 >= y2 + h2);
}

// Tạo quái theo loại (0=Small, 1=Medium, 2=Large)
void spawnEnemy(int sizeIdx) {
    if (!texEnemyBullet) {
        return;
    }

    Enemy e;
    e.active = true;
    e.state = EnemyState::ALIVE;
    e.last_shoot_time = SDL_GetTicks();
    e.anim = {0, 1, 0};
    e.texture_bullet = texEnemyBullet;

    if (sizeIdx == 0) {
        e.size_class = EnemySize::SMALL;
        e.texture_base = texEnemySmallBase;
        e.texture_destruction = texEnemySmallDest;
        e.hp = 10;
        e.max_hp = 10;
        e.shoot_delay = 2300;
        e.score_value = SCORE_ENEMY_SMALL;
        e.drop_chance = 15;
    } else if (sizeIdx == 1) {
        e.size_class = EnemySize::MEDIUM;
        e.texture_base = texEnemyMediumBase;
        e.texture_destruction = texEnemyMediumDest;
        e.hp = 30;
        e.max_hp = 30;
        e.shoot_delay = 1800;
        e.score_value = SCORE_ENEMY_MEDIUM;
        e.drop_chance = 20;
    } else {
        e.size_class = EnemySize::LARGE;
        e.texture_base = texEnemyLargeBase;
        e.texture_destruction = texEnemyLargeDest;
        e.hp = 50;
        e.max_hp = 50;
        e.shoot_delay = 800;
        e.score_value = SCORE_ENEMY_LARGE;
        e.drop_chance = 25;
    }

    e.w = 64;
    e.h = 64;
    if (e.texture_base) {
        SDL_QueryTexture(e.texture_base, NULL, NULL, &e.w, &e.h);
    }
    e.x = (float)(rand() % std::max(1, SCREEN_WIDTH - e.w));
    e.y = (float)(-e.h);
    e.target_y = 100.0f + (float)(rand() % 100);
    enemies.push_back(e);
}

// Spawn quái theo stage
void updateSpawner() {
    if (transitionStart > 0) {
        return;
    }
    int cooldown = std::max(30, 100 - currentWave * 5);
    if (++spawnTimer < cooldown) {
        return;
    }
    spawnTimer = 0;

    if (spawnedSmall + spawnedMedium + spawnedLarge >= waveEnemyTarget) {
        return;
    }

    if (currentStage == 1) {
        spawnEnemy(0);
        spawnedSmall++;
    } else if (currentStage == 2) {
        if (rand() % 3 == 0) {
            spawnEnemy(1);
            spawnedMedium++;
        } else {
            spawnEnemy(0);
            spawnedSmall++;
        }
    } else {
        if (rand() % 4 == 0) {
            spawnEnemy(2);
            spawnedLarge++;
        } else if (rand() % 4 == 1) {
            spawnEnemy(1);
            spawnedMedium++;
        } else {
            spawnEnemy(0);
            spawnedSmall++;
        }
    }
}

// Cập nhật quái mỗi frame
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
        if (e.move_phase == 0) {
            e.y += ENEMY_SPEED;
            if (e.y >= e.target_y) {
                e.y = e.target_y;
                e.move_phase = 1;
            }
        }
        if (now - e.last_shoot_time <= e.shoot_delay) {
            continue;
        }
        float cx = e.x + e.w / 2.0f;
        float cy = e.y + e.h;
        if (e.size_class == EnemySize::LARGE) {
            fireEnemyBullets(cx, cy, e.texture_bullet, AttackType::BURST_360, 2.5f);
        } else if (e.size_class == EnemySize::MEDIUM) {
            fireEnemyBullets(cx, cy, e.texture_bullet, AttackType::N_WAY, 3.5f);
        } else {
            fireEnemyBullets(cx, cy, e.texture_bullet, AttackType::N_WAY, 4.0f);
        }
        e.last_shoot_time = now;
    }
}

// Xử lý khi quái chết (nổ + điểm + drop)
void handleEnemyDeath(Enemy& e) {
    e.state = EnemyState::EXPLODING;
    int deathFrames = 1;
    if (e.texture_destruction) {
        deathFrames = countFrames(e.texture_destruction);
    }
    e.anim = {0, deathFrames, SDL_GetTicks()};
    player.score += e.score_value;
    enemyKillCount++;
    floating_texts.push_back({
        e.x + e.w / 2.0f - 20,
        e.y + e.h / 2.0f - 10,
        e.score_value,
        255.0f,
        true
    });

    if (rand() % 100 >= e.drop_chance) {
        return;
    }

    Pickup p;
    p.x = e.x + e.w / 2.0f - 16;
    p.y = e.y + e.h / 2.0f;
    p.vy = PICKUP_SPEED;
    p.active = true;
    p.anim = {0, 1, SDL_GetTicks()};

    if (rand() % 100 < 60) {
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

// Bắn đạn quái theo góc
static void shootAngle(SDL_Texture* tex, float cx, float cy, float angleDeg, float speed) {
    float rad = angleDeg * PI / 180.0f;
    float vx = std::cos(rad) * speed;
    float vy = std::sin(rad) * speed;
    bullets.push_back(createBullet(tex, cx - 8, cy, vx, vy, 1, false));
}

// Bắn đạn quái theo pattern
void fireEnemyBullets(float cx, float cy, SDL_Texture* tex, AttackType type, float speed) {
    if (type == AttackType::N_WAY) {
        for (int off = -1; off <= 1; off++) {
            shootAngle(tex, cx, cy, 90.0f + off * 20.0f, speed);
        }
    } else {
        for (int deg = 0; deg < 360; deg += 30) {
            shootAngle(tex, cx, cy, (float)deg, speed);
        }
    }
}

// Đạn player trúng quái
void checkBulletEnemyCollisions() {
    for (Bullet& b : bullets) {
        if (!b.active || !b.is_player_bullet) {
            continue;
        }
        for (Enemy& e : enemies) {
            if (!e.active || e.state != EnemyState::ALIVE) {
                continue;
            }
            float ex = e.x + e.w / 2.0f - 2;
            float ey = e.y + e.h / 2.0f - 2;
            if (!AABB(b.x, b.y, b.w, b.h, ex, ey, 4, 4)) {
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
}
