#include <windows.h>
#include <windowsx.h>

#include "MapGrid.h"
#include "EditorState.h"
#include "EntityList.h"

bool g_isPainting = false;
bool g_isErasing = false;

bool g_paintUndoSaved = false;

void CreateMapGrid(HWND hwnd) {
    g_mapGrid =
        CreateWindowExA(
            WS_EX_CLIENTEDGE,
            "Lite Engine MapGrid",
            "",
            WS_CHILD |
            WS_VISIBLE |
            WS_TABSTOP,

            0,
            0,
            0,
            0,

            hwnd,
            reinterpret_cast<HMENU>(
                ID_MAP_GRID
                ),

            GetModuleHandleA(nullptr),
            nullptr
        );
}

void UndoMapChange() {
    if (g_undoStack.empty())
        return;

    MapState current;

    current.size = g_mapSize;
    current.rows = g_mapRows;

    g_redoStack.push_back(current);

    MapState previous = g_undoStack.back();

    g_undoStack.pop_back();

    g_mapSize = previous.size;
    g_mapRows = previous.rows;
}

void RedoMapChange() {
    if (g_redoStack.empty())
        return;

    MapState current;

    current.size = g_mapSize;
    current.rows = g_mapRows;

    g_undoStack.push_back(current);

    MapState next = g_redoStack.back();

    g_redoStack.pop_back();

    g_mapSize = next.size;
    g_mapRows = next.rows;
}

void SaveUndoState() {
    MapState state;

    state.size = g_mapSize;
    state.rows = g_mapRows;

    g_undoStack.push_back(state);
    g_redoStack.clear();
}

void PaintMapTile(HWND hwnd, int mouseX, int mouseY) {
    RECT clientRect = {};

    GetClientRect(hwnd, &clientRect);

    int clientWidth = clientRect.right - clientRect.left;
    int clientHeight = clientRect.bottom - clientRect.top;

    if (clientWidth <= 0 || clientHeight <= 0) {
        return;
    }

    int mapX = static_cast<int>(static_cast<double>(mouseX) * g_mapSize / clientWidth);
    int mapY = static_cast<int>(static_cast<double>(mouseY) * g_mapSize / clientHeight);

    if (mapX < 0 || mapX >= g_mapSize || mapY < 0 || mapY >= g_mapSize)
        return;

    if (mapY >= static_cast<int>(g_mapRows.size()))
        return;

    if (mapX >= static_cast<int>(g_mapRows[mapY].size()))
        return;

    if (g_selectedEntity < 0 || g_selectedEntity >= static_cast<int>(g_entities.size()))
        return;

    if (g_mapRows[mapY][mapX] == '1')
        return;

    char entityID = static_cast<char>('0' + g_entities[g_selectedEntity].id);

    if (g_mapRows[mapY][mapX] == entityID) 
        return;

    if (!g_paintUndoSaved) {
        SaveUndoState();
        g_paintUndoSaved = true;
    }

    g_mapRows[mapY][mapX] = entityID;

    InvalidateRect(hwnd, nullptr, FALSE);
}

void EraseMapTile(HWND hwnd, int mouseX, int mouseY) {
    RECT clientRect = {};

    GetClientRect(hwnd, &clientRect );

    int clientWidth = clientRect.right - clientRect.left;
    int clientHeight = clientRect.bottom - clientRect.top;

    if (clientWidth <= 0 || clientHeight <= 0) {
        return;
    }

    int mapX = static_cast<int>(static_cast<double>(mouseX) * g_mapSize / clientWidth);
    int mapY = static_cast<int>(static_cast<double>(mouseY) * g_mapSize / clientHeight);

    if (mapX < 0 || mapX >= g_mapSize || mapY < 0 || mapY >= g_mapSize) {
        return;
    }

    if (mapY >= static_cast<int>(g_mapRows.size())) {
        return;
    }

    if (mapX >= static_cast<int>(g_mapRows[mapY].size())) {
        return;
    }

    if (g_mapRows[mapY][mapX] == ' ') {
        return;
    }

    if (!g_paintUndoSaved) {
        SaveUndoState();
        g_paintUndoSaved = true;
    }

    g_mapRows[mapY][mapX] = ' ';

    InvalidateRect(hwnd, nullptr, FALSE);
}

LRESULT CALLBACK MapGridProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam) {
    switch (message) {
    case WM_PAINT: {
        PAINTSTRUCT ps = {};

        HDC hdc = BeginPaint(hwnd, &ps);

        DrawMapGrid(hwnd, hdc);
        EndPaint(hwnd, &ps);

        return 0;
    }
    case WM_ERASEBKGND: {
        return 1;
    }
    case WM_LBUTTONDOWN: {
        SetFocus(hwnd);

        g_isPainting = true;
        g_isErasing = false;
        g_paintUndoSaved = false;

        SetCapture(hwnd);

        PaintMapTile(hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

        return 0;
    }
    case WM_MOUSEMOVE: {
        if (g_isPainting) {
            PaintMapTile(hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }
        else if (g_isErasing) {
            EraseMapTile(hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }

        return 0;
    }
    case WM_LBUTTONUP: {
        g_isPainting = false;
        g_paintUndoSaved = false;

        if (GetCapture() == hwnd) {
            ReleaseCapture();
        }

        return 0;
    }
    case WM_RBUTTONDOWN: {
        SetFocus(hwnd);

        g_isPainting = false;
        g_isErasing = true;
        g_paintUndoSaved = false;

        SetCapture(hwnd);

        EraseMapTile(hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

        return 0;
    }
    case WM_RBUTTONUP: {
        g_isErasing = false;
        g_paintUndoSaved = false;

        if (GetCapture() == hwnd) {
            ReleaseCapture();
        }

        return 0;
    }
    case WM_CAPTURECHANGED: {
        g_isPainting = false;
        g_isErasing = false;
        g_paintUndoSaved = false;
        return 0;
    }
    case WM_KEYDOWN: {
        if (GetKeyState(VK_CONTROL) & 0x8000) {
            if (wParam == 'Z') {
                UndoMapChange();
                InvalidateRect(hwnd, nullptr, FALSE);
                return 0;
            }

            if (wParam == 'Y') {
                RedoMapChange();
                InvalidateRect(hwnd, nullptr, FALSE);
                return 0;
            }
        }

        if (wParam == VK_ESCAPE) {
            g_selectedEntity = -1;

            if (g_entityList) {
                SendMessageA(g_entityList, LB_SETCURSEL, static_cast<WPARAM>(-1), 0);
            }

            InvalidateRect(hwnd, nullptr, FALSE);
        }

        return 0;
    }
    }

    return DefWindowProcA(hwnd, message, wParam, lParam);
}

void DrawMapGrid(HWND hwnd,HDC hdc) {

    RECT clientRect = {};

    GetClientRect(hwnd, &clientRect);

    const int clientWidth = clientRect.right - clientRect.left;
    const int clientHeight = clientRect.bottom - clientRect.top;

    if (clientWidth <= 0 || clientHeight <= 0) {
        return;
    }

    HDC memoryDC = CreateCompatibleDC(hdc);

    if (!memoryDC)
        return;

    HBITMAP memoryBitmap = CreateCompatibleBitmap(hdc, clientWidth, clientHeight);

    if (!memoryBitmap) {
        DeleteDC(memoryDC);
        return;
    }

    HBITMAP oldBitmap = static_cast<HBITMAP>(SelectObject(memoryDC, memoryBitmap));

    HBRUSH background = CreateSolidBrush(MAP_BLUE);

    FillRect(memoryDC, &clientRect, background);
    DeleteObject(background);

    HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    HPEN oldPen = static_cast<HPEN>(SelectObject(memoryDC, gridPen));

    for (int y = 0; y < g_mapSize; ++y) {
        if (y >= static_cast<int>(g_mapRows.size())) {
            break;
        }

        const std::string& row = g_mapRows[y];

        for (int x = 0; x < g_mapSize; ++x) {
            if (x >= static_cast<int>(row.size())) {
                break;
            }

            char tile = row[x];

            int left = static_cast<int>(static_cast<double>(x) * clientWidth / g_mapSize);
            int top = static_cast<int>(static_cast<double>(y) * clientHeight / g_mapSize);
            int right = static_cast<int>(static_cast<double>(x + 1) * clientWidth / g_mapSize);
            int bottom = static_cast<int>(static_cast<double>(y + 1) * clientHeight / g_mapSize);

            RECT cellRect = {
                left,
                top,
                right,
                bottom
            };

            COLORREF color;

            if (tile == '1') {
                color = RGB(255, 255, 255); // Wall
            }
            else {
                color = MAP_BLUE; // Floor/entity
            }

            HBRUSH brush = CreateSolidBrush(color);

            FillRect(memoryDC, &cellRect, brush);
            DeleteObject(brush);

            // Grid lines
            MoveToEx(
                memoryDC,
                left,
                top,
                nullptr
            );

            LineTo(
                memoryDC,
                right,
                top
            );

            MoveToEx(
                memoryDC,
                left,
                top,
                nullptr
            );

            LineTo(
                memoryDC,
                left,
                bottom
            );

            if (tile >= '2' && tile <= '9') {
                int entityID = tile - '0';

                int imageIndex = -1;

                for (int i = 0; i < static_cast<int>(g_entityImageIDs.size()); ++i) {
                    if (g_entityImageIDs[i] == entityID) {
                        imageIndex = i;
                        break;
                    }
                }

                if (imageIndex >= 0 && g_entityList) {
                    HIMAGELIST imageList = reinterpret_cast<HIMAGELIST>(GetPropA(g_entityList, "EntityImageList"));

                    if (imageList) {
                        int cellWidth = right - left;
                        int cellHeight = bottom - top;

                        HICON icon = ImageList_GetIcon(imageList, imageIndex, ILD_NORMAL);

                        if (icon) {
                            DrawIconEx(
                                memoryDC,
                                left,
                                top,
                                icon,
                                cellWidth,
                                cellHeight,
                                0,
                                nullptr,
                                DI_NORMAL
                            );

                            DestroyIcon(icon);
                        }
                    }
                }
            }
        }
    }

    SelectObject(memoryDC, oldPen);
    DeleteObject(gridPen);

    BitBlt(
        hdc,
        0,
        0,
        clientWidth,
        clientHeight,
        memoryDC,
        0,
        0,
        SRCCOPY
    );

    SelectObject(memoryDC, oldBitmap);
    DeleteObject(memoryBitmap);
    DeleteDC(memoryDC);
}