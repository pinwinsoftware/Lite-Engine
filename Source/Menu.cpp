#include <iostream>
#include <windows.h>

#include "main.h"
#include "Settings.h"
#include "Game.h"

int menuIndex = 0;

void Menu() {
    gameComplete = false;

    positionxy(0, 0);
    std::cout << "=== Monster Shooter ===\n\n";
    std::cout << "Run DEMO\n\n";
    std::cout << "Settings\n";

    if (GetAsyncKeyState(VK_UP) & 0x8000) {
        if (menuIndex > 0) {
            menuIndex -= 2;
            Sleep(120);
            system("cls");
        }
    }

    if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
        if (menuIndex < 2) {
            menuIndex += 2;
            Sleep(120);
            system("cls");
        }
    }

    positionxy(10, menuIndex + 2);
    std::cout << "<--";

    if (GetAsyncKeyState(VK_RETURN) & 1) {
        switch (menuIndex) {
        case 0:
            currentMap = &map1_struct;
            system("cls");
            x = 3.5f;
            y = 11.5f;
            angle = 270.f;
            ammo = 100;
            health = 100;
            gameFailed = false;
            LoadEntities();
            state = GameState::RUNNING;
            break;
        case 2:
            system("cls");
            state = GameState::SETTINGS;
            break;
        }
    }
}