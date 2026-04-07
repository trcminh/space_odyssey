// ============================================================
//  assets.h — Khai báo hàm tải file ảnh và âm thanh
// ============================================================
#ifndef ASSETS_H
#define ASSETS_H

#include "globals.h"

// Tải ảnh PNG từ đường dẫn, trả về SDL_Texture (nullptr nếu lỗi)
SDL_Texture* loadTexture(const std::string& path);
inline void safeDestroy(SDL_Texture*& tex) {
    if (tex) {
        SDL_DestroyTexture(tex);
        tex = nullptr;
    }
}



// Tải toàn bộ file ảnh, font và nhạc — gọi 1 lần lúc khởi động
// Trả về false nếu có lỗi nghiêm trọng (ví dụ không tìm thấy font)
bool loadFiles();

// Nạp texture quái theo stage (giải phóng texture cũ trước)
// Gọi lại mỗi khi chuyển sang stage mới
void loadStageCatalog(int stage);

#endif
