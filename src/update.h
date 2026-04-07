#ifndef UPDATE_H
#define UPDATE_H

#include "globals.h"
#include "weapon.h"
#include "enemy.h"
#include "player.h"
#include "collision.h"

// ============================================================
//  HELPER FUNCTIONS
// ============================================================

bool updateAnim(AnimState& anim, Uint32 now, Uint32 interval);
int countFrames(SDL_Texture* tex);

// ============================================================
//  CÁC HÀM UPDATE MỖI FRAME CÒN LẠI
// ============================================================

void updatePickups();
void updateFloatingTexts();
void cleanupVectors();

#endif
