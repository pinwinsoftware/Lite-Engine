/*
    === Lite Engine ===

    Lite Engine - Lightweight First-Person Game Engine
    Version 0.1.7.0
    Copyright (C) 2026 Pinwin Software

    === License ===

    SPDX-License-Identifier: GPL-3.0-or-later

    This project is licensed under the GNU General Public License v3.0 or later.
    See the LICENSE file for details.

    === About Lite Engine ===

    Lite Engine is a lightweight engine intended for 3D first-person games.
    The current version of Lite Engine (Lite Engine 0) runs entirely in Windows Console, and all the graphics are being rendered in raycasting.
    The Lite Engine is going to receive many updates in the future that will significantly change it.
    You can read the official devlog about Lite Engine on https://liteengine.pinwinsoftware.com/devlog

    === Current Features ===

    The engine currently supports basic FPS features such as wall rendering, player movement, different entity types, collectibles, and enemies.

    The engine is designed for developing FPS shooter games and provides the main systems needed for creating a playable game.

    === How to Run ===

    Before running the demo or compiling code, make sure that your CMD resolution is big enough.
    By default, Lite Engine is running in 120x40 mode (characters, not pixels!), but you can change it in the settings menu or in code.

    === Controls ===

    W = Walk Forward
    S = Walk Backward
    A = Strafe Left
    D = Strafe Right

    Mouse = Camera Movement
    Shift = Sprint
    M1 = Shoot
    Space = Shoot
    Tab = Map

    1 = Knife
    2 = Gun

    === Credits ===

    Developed by Larion Naumenko

    Official Website: https://pinwinsoftware.com

    YouTube Channel: https://www.youtube.com/@PinwinSoftware
*/

#include <iostream>
#include <windows.h>
#include <string>
#include <cstring>
#include <cmath>
#include <vector>

#include "Menu.h"
#include "Settings.h"
#include "Game.h"
#include "Weapons.h"
#include "main.h"

using namespace std;

std::vector<std::string> map1 = {
    "1111111111111",
    "16    1    71",
    "1 1 1 1 121 1",
    "1           1",
    "1 1 1 1 1 1 1",
    "12    1     1",
    "111 11111 111",
    "17  2 1    61",
    "1 1 1 1 1 1 1",
    "1     1  2  1",
    "1 1 1 1 1 1 1",
    "1     12    1",
    "1111111115111"
};

std::vector<std::string> map2 = {
    "1111111111111111111111",
    "12666 13    1777166771",
    "1 111 1     1   1  2 1",
    "1        1      1    1",
    "1  3  1     1   1 11 1",
    "1     1     16661 11 1",
    "1     11111111111 11 1",
    "1     1     3        1",
    "1  2  1     11    3  1",
    "1        11211       1",
    "1 111 1  11    1111  1",
    "1  2  1   7    112   1",
    "11111111111111111 1  1",
    "1121    21     1171 21",
    "15       3           1",
    "11   21     1        1",
    "1111111111111111111111"
};

std::vector<std::string> map3 = {
    "1111111111111111111111",
    "16   1     61676767671",
    "16      11  1333333331",
    "16   1  11  1333333331",
    "111111  112 1333333331",
    "1 5 11  11  1        1",
    "1   11  3            1",
    "1   117          2   1",
    "1323111111111  1111  1",
    "1     2712621  1111  1",
    "1      31   1  1111  1",
    "1     271            1",
    "1     111   1111111111",
    "1   2         16666661",
    "1  2 2            2  1",
    "1   2         17777771",
    "1111111111111111111111"
};

std::vector<std::string> map4 = {
    "1111111111111111111111",
    "1777771     1222277771",
    "177777      1222266661",
    "166666      12222    1",
    "1666661     12222    1",
    "1111111  1  11111    1",
    "1 3      1     3  6  1",
    "1        1   3       1",
    "1   111111111111111111",
    "1      3             1",
    "1 2       2          1",
    "111   11111   11111  1",
    "1517371636173716361221",
    "1 11111   11111   1  1",
    "1 3 3 3            221",
    "1  3 3               1",
    "1111111111111111111111"
};

std::vector<std::string> map5 = {
    "11111111111111111111111",
    "1                     1",
    "1                     1",
    "1                     1",
    "1     21111 11113     1",
    "1    2 1777 6661 3    1",
    "1   2  1777 6661  3   1",
    "1  2   1777 6661   3  1",
    "1 2    1777 6661    3 1",
    "12     111111111     31",
    "12     2       3     31",
    "172     2     3     361",
    "1772     2   3     3661",
    "17772     243     36661",
    "177772     4     366661",
    "1777772    4    3666661",
    "11111111111511111111111"
};

void positionxy(short x, short y)
{
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD position = { x, y };
    SetConsoleCursorPosition(hStdout, position);
}

void hideCursor() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cInfo;
    GetConsoleCursorInfo(hOut, &cInfo);
    cInfo.bVisible = false;
    SetConsoleCursorInfo(hOut, &cInfo);
}

void EnableMouse() {
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);

    DWORD mode;
    GetConsoleMode(hInput, &mode);

    // Remove quick edit (text selection)
    mode &= ~ENABLE_QUICK_EDIT_MODE;

    
    mode |= ENABLE_EXTENDED_FLAGS;

    // Enable mouse
    mode |= ENABLE_MOUSE_INPUT;

    SetConsoleMode(hInput, mode);
}

int screenHeight = 40;
int screenWidth = 120;

float pi = 3.14159;
float x = 4.f;
float y = 11.f;
float angle = 180;
int health = 100;
int ammo = 100;
std::vector<std::vector<char>> screen;

POINT center;
POINT lastMouse;

GameState state = GameState::MENU;

Map map1_struct = { map1, static_cast<int>(map1.size()) };
Map map2_struct = { map2, static_cast<int>(map2.size()) };
Map map3_struct = { map3, static_cast<int>(map3.size()) };
Map map4_struct = { map4, static_cast<int>(map4.size()) };
Map map5_struct = { map5, static_cast<int>(map5.size()) };

Map* currentMap = &map1_struct;

int main() {
    LoadSprites();
    hideCursor();
    GetCursorPos(&lastMouse);

    screen.resize(screenHeight);
    for (auto& row : screen)
        row.resize(screenWidth, ' ');

    while (true) {

        switch (state) {
        case GameState::MENU:
            Menu();
            break;
        case GameState::SETTINGS:
            Settings();
            break;
        case GameState::RUNNING:
            Game();
            break;
        }
    }
}