// ============================================================
//  render.h — Khai báo các hàm vẽ lên màn hình
// ============================================================
#ifndef RENDER_H
#define RENDER_H

#include "globals.h"

// Render chuỗi text thành SDL_Texture
// Lưu ý: Người gọi phải SDL_DestroyTexture() sau khi dùng xong
SDL_Texture* loadText(const std::string& text, SDL_Color color);

// Vẽ text tại tọa độ (x, y), với scale tuỳ chọn (mặc định 1.0 = kích thước gốc)
void renderText(const std::string& text, SDL_Color color, int x, int y, float scale = 1.0f);

// Vẽ text căn giữa màn hình theo chiều ngang tại dòng y
void renderTextCentered(const std::string& text, SDL_Color color, int y, float scale = 1.0f);

// Vẽ toàn bộ 1 frame: nền sao + entities + HUD + menu
void render();

#endif
