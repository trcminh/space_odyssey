#ifndef WEAPON_H
#define WEAPON_H

#include "types.h"
#include <SDL.h>

Bullet createBullet(SDL_Texture* tex, float x, float y, float vx, float vy, int damage, bool isPlayer);
void firePlayerWeapon(float cx, float y, WeaponType wpn, int lvl);
void fireEnemyBullets(float cx, float cy, SDL_Texture* tex, AttackType type, float speed);
void updateBullets();

#endif
