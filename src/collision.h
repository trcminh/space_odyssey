#ifndef COLLISION_H
#define COLLISION_H

#include "types.h"
#include <SDL.h>

bool AABB(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2);
void getEnemyHitbox(const Enemy& e, float& ex, float& ey, float& ew, float& eh);
void checkCollisions();

#endif
