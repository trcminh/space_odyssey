#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <string>

// Kích thước chuẩn của cửa sổ (chiều ngang nhỏ, chiều dọc dài theo chuẩn Shoot em up)
const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 800;

// Các trạng thái của Game
enum class GameState {
    MENU,
    PLAYING,
    GAMEOVER
};
