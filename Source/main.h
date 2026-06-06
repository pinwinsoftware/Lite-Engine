#pragma once
#include <vector>
#include <string>
#include <windows.h>

void positionxy(short x, short y);

extern float x;
extern float y;
extern float angle;

extern int screenHeight;
extern int screenWidth;

extern std::vector<std::vector<char>> screen;

struct Map {
    std::vector<std::string> rows;
    int height;
};

extern Map* currentMap;

extern Map map1_struct;
extern Map map2_struct;
extern Map map3_struct;

extern POINT lastMouse;

enum class GameState {
    MENU,
    SETTINGS,
    RUNNING
};

extern GameState state;