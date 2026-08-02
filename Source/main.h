#pragma once
#include <vector>
#include <string>
#include <windows.h>

void positionxy(short x, short y);
void EnableMouse();

extern float pi;
extern float x;
extern float y;
extern float angle;
extern int health;
extern int ammo;

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
extern Map map4_struct;
extern Map map5_struct;

extern POINT lastMouse;

enum class GameState {
    MENU,
    SETTINGS,
    RUNNING
};

extern GameState state;