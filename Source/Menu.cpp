#include <iostream>
#include <windows.h>

#include "main.h"
#include "Settings.h"
#include "Game.h"

int menuIndex = 0;

void Menu() {
    gameComplete = false;

    positionxy(0, 0);
    std::cout << "Run DEMO 1 | Coin Hunt\n\n";
    std::cout << "Run DEMO 2 | Hiding Monsters\n\n";
    std::cout << "Run DEMO 3 | Troll Rooms\n\n";
    std::cout << "Settings\n";

    if (GetAsyncKeyState(VK_UP) & 0x8000) {
        if (menuIndex > 0) {
            menuIndex -= 2;
            Sleep(120);
            system("cls");
        }
    }

    if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
        if (menuIndex < 6) {
            menuIndex += 2;
            Sleep(120);
            system("cls");
        }
    }

    positionxy(30, menuIndex);
    std::cout << "<--";

    if (GetAsyncKeyState(VK_RETURN) & 1) {
        switch (menuIndex) {
        case 0:
            currentMap = &map1_struct;
            system("cls");
            x = 5.f;
            y = 2.5f;
            angle = 0.f;
            totalCoins = 250;
            state = GameState::RUNNING;
            break;
        case 2:
            currentMap = &map2_struct;
            system("cls");
            x = 15.f;
            y = 3.5f;
            angle = 180.f;
            totalCoins = 0;
            state = GameState::RUNNING;
            break;
        case 4:
            currentMap = &map3_struct;
            system("cls");
            x = 3.f;
            y = 3.f;
            angle = 0.f;
            totalCoins = 0;
            state = GameState::RUNNING;
            break;
        case 6:
            system("cls");
            state = GameState::SETTINGS;
            break;
        }
    }
}