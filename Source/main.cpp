/*
    === Lite Engine ===

    Lite Engine - Very Lightweight First-Person Game Engine
    Version 0.1.3.0
    Copyright (C) 2026 Pinwin Software

    === License ===

    SPDX-License-Identifier: GPL-3.0-or-later

    This project is licensed under the GNU General Public License v3.0 or later.
    See the LICENSE file for details.

    === About Lite Engine ===

    Lite Engine is a lightweight engine intended for 3D first-person games.
    The current version of Lite Engine (Lite Engine 0) runs entirely in Windows Console, and all the graphics are being rendered in raycasting.
    The Lite Engine is going to get tons of updates later that will change it completly.
    You can read the official devlog about Lite Engine on https://liteengine.pinwinsoftware.com/devlog

    === How to Run ===

    Before running the demo or compiling code, make sure that your CMD resolution is big enough.
    By default, Lite Engine is running in 120x40 mode (characters, not pixels!), but you can change it in the settings menu or in code.

    === Controls ===

    W = Walk Forward
    S = Walk Backward
    A = Strafe Left
    D = Strafe Rigfht

    Mouse = Camera Movement
    Shift = Sprint
    Space = Shoot

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
#include "Shoot.h"
#include "main.h"

using namespace std;

std::vector<std::string> map1 = {
    "1111111111111111111111111111111111111111111111111111111111111111   1111111111111111111",
    "1       1    13333333                      1                   1   1                 1",
    "1            13  3  3                      1  3 3 3 3 3 3 3 3  1   1       NOT       1",
    "1       1    13333333                      1  3             3  1   1                 1",
    "111111111    1      1       11111111       1  3 3 3 3 3 3 3 3  1   1111111111111111111",
    "1                   133333331333333133333331  3             3  1                      ",
    "1                   13     3 33333313     31  3 3 3 3 3 3 3 3  1   1111111111111111111",
    "1                   133333331333333133333331  3             3  1   1                 1",
    "111111111111111111111       11111111       1  3 3 3 3 3 3 3 3  1   1        A        1",
    "1                           1                                  1   1                 1",
    "1                           1                                  1   1111111111111111111",
    "1         1111111111111111111     111111111111111111111        1                      ",
    "1333333333                  1333331    333 333 333 333 333333331   1111111111111111111",
    "1333333333                  13   31    3 3 3 3 3 3 3 3 3      31   1                 1",
    "1333333333                  1333331    333 333 333 333 333333331   1     SECRET      1",
    "1111111111111111111111      111111111111111111111111111111111111   1                 1",
    "13 3 3 3                                   1 1 333     1 1333331   1111111111111111111",
    "1 3 3 3                               111  111     111 111     1                      ",
    "13 3 3 3                              1 1      333 1 1    333331   1111111111111111111",
    "111111111111111111111111111111111111111 111111111111111111     1   1                 1",
    "1                     1 1   1111111   1 1  333 333 333 1 1333331   1      AREA       1",
    "15                    111             111  3 3 3 3 3 3 111     1   1                 1",
    "1                           1111111        333 333 333         1   1111111111111111111",
    "1111111111111111111111111111111111111111111111111111111111111111                      ",
    "1                    ÑÑ   ÑÑ   ÑÑÑ  Ñ  Ñ   ÑÑÑ                 1   1111111111111111111",
    "1                   Ñ    Ñ  Ñ   Ñ   ÑÑ Ñ  ÑÑ                   1   1                 1",
    "1                   Ñ    Ñ  Ñ   Ñ   Ñ ÑÑ    ÑÑ                 1   1     AT ALL      1",
    "1                    ÑÑ   ÑÑ   ÑÑÑ  Ñ  Ñ  ÑÑÑ                  1   1                 1",
    "1111111111111111111111111111111111111111111111111111111111111111   1111111111111111111",
};

std::vector<std::string> map2 = {
    "1111111111111111111111",
    "1     1     1   1    1",
    "1 111 1     1   1  2 1",
    "1        1      1    1",
    "1  2  1     1   1 11 1",
    "1     1     1   1 11 1",
    "1     11111111111 11 1",
    "1     1              1",
    "1  2  1     11       1",
    "1        11211       1",
    "1 111 1  11    1111  1",
    "1  2  1        112   1",
    "11111111111111111 1  1",
    "1121    21     11 1 21",
    "15                   1",
    "11   21     1        1",
    "1111111111111111111111"
};

std::vector<std::string> map3 = {
    "1111111111111111111111111111111111111111111111111111111111111    111111111      ",
    "1444444444444444444444444444444444111444444444444444444444441    144444441      ",
    "14                                4114                     41    14     41      ",
    "14                         4444    414                     41    14     41      ",
    "14444444     444444444            4114    44444    444     41    14     41      ",
    "11111114     411111114     44    41114    41114    414     41111114     41111111",
    "      14     41     14     414    4444    44444    414     44444444     44444441",
    "      14     41     14     4114                    414                        41",
    "      14     41     14     4114                    414                        51",
    "      144444441     144444441144444444444444444444441444444444444444444444444441",
    "      111111111     111111111111111111111111111111111111111111111111111111111111"
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

int screenHeight = 40;
int screenWidth = 120;

float x = 4.f;
float y = 11.f;
float angle = 180;
std::vector<std::vector<char>> screen;

POINT center;
POINT lastMouse;

GameState state = GameState::MENU;

Map map1_struct = { map1, static_cast<int>(map1.size()) };
Map map2_struct = { map2, static_cast<int>(map2.size()) };
Map map3_struct = { map3, static_cast<int>(map3.size()) };

Map* currentMap = &map3_struct;

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