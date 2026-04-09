// ============================================================
//  globals.h — Khai báo tất cả biến toàn cục của game
//
//  Dùng từ khoá "extern" để báo cho compiler biết:
//  "biến này tồn tại, nhưng được định nghĩa ở file globals.cpp"
//  Mọi file .cpp cần dùng biến toàn cục thì #include "globals.h"
// ============================================================
#ifndef GLOBALS_H
#define GLOBALS_H

#include "types.h"

// ============================================================
//  CỬA SỔ VÀ RENDERER CHÍNH
// ============================================================
extern SDL_Window*   window;
extern SDL_Renderer* renderer;
extern TTF_Font*     font;

// ============================================================
//  TEXTURES: TÀU NGƯỜI CHƠI
//  texPlayer[0] = Very Damaged, [1] = Damaged, [2] = Slight, [3] = Full
// ============================================================
extern SDL_Texture* texPlayer[4];
extern SDL_Texture* texEngineFlame;

// ============================================================
//  TEXTURES: ĐẠN NGƯỜI CHƠI
// ============================================================
extern SDL_Texture* texWeaponAuto;
extern SDL_Texture* texWeaponRocket;

// ============================================================
//  TEXTURES: ICON VẬT PHẨM RƠI
// ============================================================
extern SDL_Texture* texIconAuto;
extern SDL_Texture* texIconRocket;
extern SDL_Texture* texIconShield;

// ============================================================
//  TEXTURES: KHÁC
// ============================================================
extern SDL_Texture* texPlayerShield;  // Vòng sáng khiên

extern SDL_Texture* texScore50;       // Điểm nổi "+50"  (pre-render 1 lần)
extern SDL_Texture* texScore70;       // Điểm nổi "+70"
extern SDL_Texture* texScore100;      // Điểm nổi "+100"

extern SDL_Texture* texEnemyBullet;   // Đạn kẻ địch (nạp lại mỗi stage)

// ============================================================
//  NHẠC NỀN
// ============================================================
extern Mix_Music* bgmMusic;

extern SDL_Texture* texEnemySmallBase;
extern SDL_Texture* texEnemySmallDest;
extern SDL_Texture* texEnemyMediumBase;
extern SDL_Texture* texEnemyMediumDest;
extern SDL_Texture* texEnemyLargeBase;
extern SDL_Texture* texEnemyLargeDest;

// ============================================================
//  DANH SÁCH THỰC THỂ ĐANG HOẠT ĐỘNG
// ============================================================
extern std::vector<Bullet>       bullets;
extern std::vector<Enemy>        enemies;
extern std::vector<Pickup>       pickups;
extern std::vector<FloatingText> floating_texts;
extern Player                    player;

// ============================================================
//  BIẾN TRẠNG THÁI GAME
// ============================================================
extern GameState    currentState;
extern int          currentStage;     // Stage hiện tại (1–3)
extern int          currentWave;      // Wave hiện tại (1–15)
extern int          enemyKillCount;   // Số quái đã giết trong wave này
extern int          waveEnemyTarget;  // Số quái cần giết để lên wave tiếp
extern int          spawnTimer;       // Bộ đếm frame để spawn quái
extern int          spawnedSmall;
extern int          spawnedMedium;
extern Uint32       transitionStart;  // Thời điểm bắt đầu chuyển stage (ms)
extern int          highScore;
extern const Uint8* keyboardState;    // Trạng thái bàn phím (SDL cấp phát)

#endif
