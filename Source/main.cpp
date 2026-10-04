/*
    === Lite Engine ===

    Lite Engine - Lightweight First-Person Game Engine
    Version 0.1.8.0
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

    === LED Archive ===
    
    Version 0.1.8.0 added support for the dynamic LED (Lite Engine Data) asset archive.

    LED files can store:

    * Maps
    * Entity definitions
    * Entity sprites
    * Weapon definitions
    * Weapon sprites


    LED files can be created and modified using the LED Editor: https://pinwinsoftware.com/Game/11/LADE

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
#include <cmath>
#include <vector>

#include "Menu.h"
#include "Settings.h"
#include "Game.h"
#include "Weapons.h"
#include "main.h"

#include "LevelDefinitions.h"
#include "EntityDefinitions.h"
#include "WeaponsDefinitions.h"

void positionxy(short x, short y) {
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

    mode &= ~ENABLE_QUICK_EDIT_MODE;
    mode |= ENABLE_EXTENDED_FLAGS;
    mode |= ENABLE_MOUSE_INPUT;

    SetConsoleMode(hInput, mode);
}

float pi = 3.14159f;

float x = 4.f;
float y = 11.f;
float angle = 180.f;

int health = 100;
int ammo = 100;

int screenHeight = 40;
int screenWidth = 120;

std::vector<std::vector<char>> screen;

POINT center;
POINT lastMouse;

GameState state = GameState::MENU;

Map currentMapData;
Map* currentMap = &currentMapData;

std::string currentLevelName = "MAP01";

int main() {
    hideCursor();

    GetCursorPos(&lastMouse);

    LoadLevelDefinitions("LED/GAME.LED");
    LoadEntityDefinitions("LED/GAME.LED");
    LoadWeaponDefinitions("LED/GAME.LED");

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