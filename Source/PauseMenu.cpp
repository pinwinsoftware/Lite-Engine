#include <iomanip>
#include <iostream>
#include "main.h"
#include "Game.h"
#include "PauseMenu.h"

int MenuOption = 1;
int PrevOption = 1;

char PausedMenu[3][28] = {
    "733333333333333333333333336",
    "5         Paused          5",
    "233333333333333333333333334",
};

void DrawPauseMenu()
{
    const int menuWidth = 28;
    const int menuHeight = 3;

    int startX = (screenWidth - menuWidth) / 2;
    int startY = (screenHeight - menuHeight) / 2;

    for (int menuY = 0; menuY < menuHeight; menuY++)
    {
        for (int menuX = 0; menuX < menuWidth; menuX++)
        {
            int drawX = startX + menuX;
            int drawY = startY + menuY;

            // Prevent drawing outside the game buffer
            if (drawX < 0 || drawX >= screenWidth || drawY < 0 || drawY >= screenHeight) {
                continue;
            }

            char tile = PausedMenu[menuY][menuX];

            switch (tile)
            {
            case '2':
                screen[drawY][drawX] = char(192);
                break;

            case '3':
                screen[drawY][drawX] = char(196);
                break;

            case '4':
                screen[drawY][drawX] = char(217);
                break;

            case '5':
                screen[drawY][drawX] = char(179);
                break;

            case '6':
                screen[drawY][drawX] = char(191);
                break;

            case '7':
                screen[drawY][drawX] = char(218);
                break;

            case '8':
                screen[drawY][drawX] = char(195);
                break;

            case '9':
                screen[drawY][drawX] = char(180);
                break;

            default:
                screen[drawY][drawX] = tile;
                break;
            }
        }
    }
}

bool gamePaused = false;

void UpdatePauseMenu() {
    // Toggle only once when ESC is pressed
    static bool escapeWasDown = false;

    bool escapeIsDown = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;

    if (escapeIsDown && !escapeWasDown) {
        gamePaused = !gamePaused;
    }

    escapeWasDown = escapeIsDown;
}
