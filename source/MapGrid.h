#pragma once
#include <windows.h>

void CreateMapGrid(HWND hwnd);
void SaveUndoState();
void UndoMapChange();
void RedoMapChange();
void DrawMapGrid(HWND hwnd, HDC hdc);

LRESULT CALLBACK MapGridProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
);