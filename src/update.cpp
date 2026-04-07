#include "update.h"
#include <algorithm>

bool updateAnim(AnimState& anim, Uint32 now, Uint32 interval) {
    if (now - anim.last_anim_time <= interval) {
        return false;
    }
    anim.last_anim_time = now;
    if (++anim.current_frame >= anim.total_frames) {
        anim.current_frame = 0;
        return true;
    }
    return false;
}

int countFrames(SDL_Texture* tex) {
    if (!tex) {
        return 1;
    }
    int w = 0, h = 0;
    SDL_QueryTexture(tex, NULL, NULL, &w, &h);
    int frames = (h > 0) ? w / h : 1;
    return (frames > 0) ? frames : 1;
}

void updateFloatingTexts() {
    for (FloatingText& ft : floating_texts) {
        if (!ft.active) {
            continue;
        }
        ft.y -= 1.0f;
        ft.alpha -= 5.0f;
        if (ft.alpha <= 0) {
            ft.active = false;
        }
    }
}

void updatePickups() {
    Uint32 now = SDL_GetTicks();
    for (Pickup& p : pickups) {
        if (!p.active) {
            continue;
        }
        p.y += p.vy;
        updateAnim(p.anim, now, 80);
        if (p.y > SCREEN_HEIGHT) {
            p.active = false;
        }
    }
}

void cleanupVectors() {
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const Bullet& x) { return !x.active; }), bullets.end());
    enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](const Enemy& x) { return !x.active; }), enemies.end());
    pickups.erase(std::remove_if(pickups.begin(), pickups.end(), [](const Pickup& x) { return !x.active; }), pickups.end());
    floating_texts.erase(std::remove_if(floating_texts.begin(), floating_texts.end(), [](const FloatingText& x) { return !x.active; }), floating_texts.end());
}