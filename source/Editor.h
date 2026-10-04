#pragma once
#include <windows.h>

LRESULT CALLBACK WindowProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
);

extern HACCEL g_accelTable;

void ResizeEditor(HWND hwnd);
void UpdateMapSizeDisplay();