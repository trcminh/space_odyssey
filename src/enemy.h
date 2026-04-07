#ifndef ENEMY_H
#define ENEMY_H

#include "types.h"
#include <SDL.h>

void spawnEnemy(int sizeIdx);
void updateSpawner();
void updateEnemies();
void triggerExplosion(Enemy& e);
void handleEnemyDeath(Enemy& e);

#endif
