/*
    === Lite Engine ===

    Lite Engine - Very Lightweight First-Person Game Engine
    Version 0.1.0.0
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

    A     = Walk Forward
    S     = Walk Backward
    Mouse = Camera Movement
    Shift = Sprint

    === Credits ===

   Developed by Larion Naumenko

   Official Website: https://pinwinsoftware.com

   YouTube Channel: https://www.youtube.com/@PinwinSoftware
*/

#include <iostream>
#include <windows.h>
#include <string>
#include <cmath>
#include <vector>

using namespace std;

constexpr int MAP_W = 32;
constexpr int MAP_H = 32;

char map1[MAP_H][MAP_W] = {
    "111111111111            ",
    "1          1            ",
    "1          1            ",
    "1  11  11  1            ",
    "1  11  11  1            ",
    "1          1            ",
    "1          1            ",
    "1  11  11  1            ",
    "1  11  11  1            ",
    "1          1            ",
    "1          1            ",
    "111111111111            ",
};

char map2[MAP_H][MAP_W] = {
    "1111111111111            ",
    "1     1     1            ",
    "1 1 1 1 1 1 1            ",
    "1           1            ",
    "1 1 1 1 1 1 1            ",
    "1     1     1            ",
    "111 11111 111            ",
    "1     1     1            ",
    "1 1 1 1 1 1 1            ",
    "1           1            ",
    "1 1 1 1 1 1 1            ",
    "1     1     1            ",
    "1111111111111            "
};

char map3[MAP_H][MAP_W] = {
    "1111111111111111111111   ",
    "1     1     1   1    1   ",
    "1 111 1     1   1    1   ",
    "1        1      1    1   ",
    "1     1     1   1 11 1   ",
    "1     1     1   1 11 1   ",
    "1     11111111111 11 1   ",
    "1     1              1   ",
    "1     1     11       1   ",
    "1        11 11       1   ",
    "1 111 1  11    1111  1   ",
    "1     1        11    1   ",
    "11111111111111111 1  1   ",
    "11 1     1     11 1  1   ",
    "1                    1   ",
    "11    1     1        1   ",
    "1111111111111111111111   "
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
bool settings = false;
bool setHeight = false;

const char (*map)[MAP_W] = map3;
int mapWidth = MAP_W;
int mapHeight = MAP_H;

POINT center;

int main() {
    hideCursor();
    
    POINT lastMouse;
    GetCursorPos(&lastMouse);

    int select = 0;

    enum GameState {
        MENU,
        SETTINGS,
        RUNNING
    };

    GameState state = MENU;

    screen.resize(screenHeight);
    for (auto& row : screen)
        row.resize(screenWidth, ' ');

    while (true) {

        switch (state) {

            // === MAIN MENU ===
        case MENU:
            positionxy(0, 0);
            std::cout << "Run DEMO 1\n\n";
            std::cout << "Run DEMO 2\n\n";
            std::cout << "Run DEMO 3\n\n";
            std::cout << "Settings\n";

            if (GetAsyncKeyState(VK_UP) & 0x8000) {
                if (select > 0) {
                    select -= 2;
                    Sleep(120);
                    system("cls");
                }
            }

            if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
                if (select < 6) {
                    select += 2;
                    Sleep(120);
                    system("cls");
                }
            }

            positionxy(12, select);
            std::cout << "<--";

            if (GetAsyncKeyState(VK_RETURN) & 1) {
                switch (select) {
                case 0:
                    state = RUNNING;
                    map = map1;
                    mapWidth = 12;
                    mapHeight = 12;
                    system("cls");
                    x = 6.f;
                    y = 6.f;
                    break;

                case 2:
                    state = RUNNING;
                    map = map2;
                    mapWidth = 13;
                    mapHeight = 13;
                    system("cls");
                    x = 3.5f;
                    y = 10.f;
                    angle = 270;
                    break;

                case 4:
                    state = RUNNING;
                    map = map3;
                    mapWidth = 23;
                    mapHeight = 17;
                    system("cls");
                    x = 15.f;
                    y = 3.5f;
                    break;

                case 6:
                    state = SETTINGS;
                    system("cls");
                    break;

                }
            }
            break;

            // === SETTINGS ===

        case SETTINGS:
            positionxy(0, 0);

            std::cout << "Set custom screen resolution\n\n";
            std::cout << "Width? ";

            int width;
            std::cin >> width;

            if (width > 0 && width <= 512) {
                screenWidth = width;
                std::cout << "Set width: " << screenWidth << "\n";
                setHeight = true;
            }
            else {
                std::cout << "Invalid width\n";
            }

            while (setHeight) {
                std::cout << "Height? ";

                int height;
                std::cin >> height;

                if (height > 0 && height <= 512) {
                    screenHeight = height;

                    screen.resize(screenHeight);

                    for (auto& row : screen) {
                        row.resize(screenWidth, ' ');
                    }

                    std::cout << "Set resolution: " << screenWidth << "x" << screenHeight;
                    Sleep(5000);
                    setHeight = false;
                    settings = false;
                    system("cls");
                    state = MENU;

                    while (GetAsyncKeyState(VK_RETURN) & 0x8000) {
                        Sleep(10);
                    }
                }
                else {
                    std::cout << "Invalid height\n";
                }
            }
            break;

        // === GAME ===
        case RUNNING:
            POINT mousePos;
            GetCursorPos(&mousePos);

            int deltaX = mousePos.x - lastMouse.x;

            float mouseSensitivity = 0.10f;
            angle += deltaX * mouseSensitivity;

            lastMouse = mousePos;

            int screenW = GetSystemMetrics(SM_CXSCREEN);
            int screenH = GetSystemMetrics(SM_CYSCREEN);

            int border = 10;

            if (mousePos.x <= border) {
                SetCursorPos(screenW - border - 1, mousePos.y);
                lastMouse.x = screenW - border - 1;
            }
            else if (mousePos.x >= screenW - border) {
                SetCursorPos(border + 1, mousePos.y);
                lastMouse.x = border + 1;
            }

            system("color 3f");

            float speed = 0.25f;

            bool running = (GetAsyncKeyState(VK_SHIFT) & 0x8000);

            // player running fast
            if (running) {
                speed = speed * 2;
            }

            float fov = 60.0f;
            int numRays = screenWidth;

            float playerRad = angle * 3.14159f / 180.0f;
            float moveX = cos(playerRad);
            float moveY = sin(playerRad);

            // clear buffer
            for (int y = 0; y < screenHeight; y++)
                for (int x = 0; x < screenWidth; x++)
                    screen[y][x] = ' ';

            int prevCellX = -1;
            int prevCellY = -1;
            int prevSide = -1;

            // raycasting 
            for (int i = 0; i < numRays; i++) {
                float rayAngle = (angle - fov / 2.0f) + ((float)i / numRays) * fov;
                float rad = rayAngle * 3.14159f / 180.0f;

                float dirX = cos(rad);
                float dirY = sin(rad);

                // DDA setup
                int currentCellX = (int)x;
                int currentCellY = (int)y;

                float deltaDistX = (dirX == 0) ? 1e30f : fabs(1.0f / dirX);
                float deltaDistY = (dirY == 0) ? 1e30f : fabs(1.0f / dirY);

                float sideDistX, sideDistY;
                int stepX, stepY;

                if (dirX < 0) {
                    stepX = -1;
                    sideDistX = (x - currentCellX) * deltaDistX;
                }
                else {
                    stepX = 1;
                    sideDistX = (currentCellX + 1.0f - x) * deltaDistX;
                }
                if (dirY < 0) {
                    stepY = -1;
                    sideDistY = (y - currentCellY) * deltaDistY;
                }
                else {
                    stepY = 1;
                    sideDistY = (currentCellY + 1.0f - y) * deltaDistY;
                }

                // DDA execution
                int side = 0; // 0 = vertical boundary, 1 = horizontal boundary
                while (map[currentCellY][currentCellX] != '1') {
                    if (sideDistX < sideDistY) {
                        sideDistX += deltaDistX;
                        currentCellX += stepX;
                        side = 0;
                    }
                    else {
                        sideDistY += deltaDistY;
                        currentCellY += stepY;
                        side = 1;
                    }
                }

                bool seam = false;

                if (i > 0) {
                    if (side != prevSide) {
                        seam = true;
                    }
                }

                // distance calculation
                float distance;
                if (side == 0) distance = (sideDistX - deltaDistX);
                else           distance = (sideDistY - deltaDistY);

                if (distance < 0.01f) distance = 0.01f;

                // fixed fish-eye correction
                float correctedDistance = distance * cos((rayAngle - angle) * 3.14159f / 180.0f);

                int wallHeight = (int)(screenHeight / correctedDistance);

                int wallTop = (screenHeight - wallHeight) / 2;
                int wallBottom = wallTop + wallHeight;

                // precision edge detection
                float wallHitFraction;
                if (side == 0) wallHitFraction = y + distance * dirY;
                else           wallHitFraction = x + distance * dirX;
                wallHitFraction -= floor(wallHitFraction);

                bool boundary = false;
                float edgeThreshold = 0.03f; // sensitivity of the vertical corners

                if (wallHitFraction < edgeThreshold || wallHitFraction >(1.0f - edgeThreshold)) {
                    if (side == 0) { // ray hit a vertical line (X-boundary)
                        if (wallHitFraction < edgeThreshold) {
                            if (currentCellY > 0 && map[currentCellY - 1][currentCellX] == ' ') {
                                boundary = true;
                            }
                        }
                        else if (wallHitFraction > (1.0f - edgeThreshold)) {
                            if (currentCellY < 14 && map[currentCellY + 1][currentCellX] == ' ') {
                                boundary = true;
                            }
                        }
                    }
                    else { // side == 1: Ray hit a horizontal line (Y-boundary)
                        if (wallHitFraction < edgeThreshold) {
                            if (currentCellX > 0 && map[currentCellY][currentCellX - 1] == ' ') {
                                boundary = true;
                            }
                        }
                        else if (wallHitFraction > (1.0f - edgeThreshold)) {
                            if (currentCellX < 22 && map[currentCellY][currentCellX + 1] == ' ') {
                                boundary = true;
                            }
                        }
                    }
                }

                prevCellX = currentCellX;
                prevCellY = currentCellY;
                prevSide = side;

                // rendering hollow column
                for (int yCoord = wallTop; yCoord < wallBottom; yCoord++) {
                    if (yCoord >= 0 && yCoord < screenHeight && i >= 0 && i < screenWidth) {
                        // true vertical corner, or top outline, or bottom outline
                        if (boundary || yCoord == wallTop || yCoord == wallBottom - 1) {
                            screen[yCoord][i] = char(219);
                        }
                        else if (seam) {
                            screen[yCoord][i] = char(219);
                        }
                        else {
                            screen[yCoord][i] = ' ';
                        }
                    }
                }
            }

            // player info
            std::string info = "X:" + std::to_string(x) + " Y:" + std::to_string(y) + " A:" + std::to_string(angle);
            for (int i = 0; i < (int)info.size() && i < screenWidth; i++) {
                screen[0][i] = info[i];
            }

            // movement
            if (GetAsyncKeyState('W') & 0x8000) {
                float newX = x + moveX * speed;
                float newY = y + moveY * speed;

                // x collision
                if (map[(int)y][(int)newX] == ' ') {
                    x = newX;
                }

                // y collision
                if (map[(int)newY][(int)x] == ' ') {
                    y = newY;
                }
            }
            if (GetAsyncKeyState('S') & 0x8000) {
                float newX = x - moveX * speed;
                float newY = y - moveY * speed;

                // x collision
                if (map[(int)y][(int)newX] == ' ') {
                    x = newX;
                }

                // y collision
                if (map[(int)newY][(int)x] == ' ') {
                    y = newY;
                }
            }
            if (GetAsyncKeyState('D') & 0x8000) {
                float newX = x - moveY * speed;
                float newY = y + moveX * speed;

                // x collision
                if (map[(int)y][(int)newX] == ' ') {
                    x = newX;
                }

                // y collision
                if (map[(int)newY][(int)x] == ' ') {
                    y = newY;
                }
            }

            if (GetAsyncKeyState('A') & 0x8000) {
                float newX = x + moveY * speed;
                float newY = y - moveX * speed;

                // x collision
                if (map[(int)y][(int)newX] == ' ') {
                    x = newX;
                }

                // y collision
                if (map[(int)newY][(int)x] == ' ') {
                    y = newY;
                }
            }

            if (angle < 0.0f) angle += 360.0f;
            if (angle >= 360.0f) angle -= 360.0f;

            // render buffer
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

            for (int y = 0; y < screenHeight; y++) {
                DWORD written;
                COORD pos = { 0, (SHORT)y };

                WriteConsoleOutputCharacterA(
                    hConsole,
                    screen[y].data(),
                    screenWidth,
                    pos,
                    &written
                );
            }
            Sleep(40);
        }
    }
}