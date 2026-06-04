#include <iostream>
#include <windows.h>

#include "main.h"

bool setHeight = false;

void ResizeConsole(int width, int height)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    SMALL_RECT rect = { 0, 0, 1, 1 };
    SetConsoleWindowInfo(hConsole, TRUE, &rect);

    COORD size;
    size.X = width;
    size.Y = height;

    SetConsoleScreenBufferSize(hConsole, size);

    rect = { 0, 0, short(width - 1), short(height - 1) };
    SetConsoleWindowInfo(hConsole, TRUE, &rect);
}

void Settings() {
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
            system("cls");

            state = GameState::MENU;

            while (GetAsyncKeyState(VK_RETURN) & 0x8000) {
                Sleep(10);
            }
        }
        else {
            std::cout << "Invalid height\n";
        }
    }
}