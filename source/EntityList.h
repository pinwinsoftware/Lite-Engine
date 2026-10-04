#pragma once
#include <windows.h>
#include <commctrl.h>

void CreateEntityList(HWND hwnd);
void RefreshEntityList();
void DrawEntityListItem(DRAWITEMSTRUCT* drawItem);