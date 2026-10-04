#include <windows.h>
#include <commctrl.h>
#include <string>
#include <algorithm>

#include "Editor.h"
#include "EditorState.h"
#include "MapGrid.h"
#include "resource.h"

#pragma comment(lib, "Comctl32.lib")

#define IDB_SPLASH 101

LRESULT CALLBACK SplashProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK SplashProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HBITMAP hBitmap = nullptr;

    switch (msg) {
    case WM_CREATE:
    {
        hBitmap = LoadBitmapA(reinterpret_cast<HINSTANCE>(GetWindowLongPtrA(hwnd, GWLP_HINSTANCE)), MAKEINTRESOURCEA(IDB_SPLASH));

        if (!hBitmap) {
            OutputDebugStringA("FAILED TO LOAD IDB_SPLASH\n");
        }

        return 0;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        if (hBitmap) {

            BITMAP bm = {};

            GetObjectA(hBitmap, sizeof(BITMAP), &bm);

            HDC memDC = CreateCompatibleDC(hdc);

            if (memDC) {
                HGDIOBJ oldBitmap = SelectObject(memDC, hBitmap);

                BitBlt(
                    hdc,
                    0,
                    0,
                    bm.bmWidth,
                    bm.bmHeight,
                    memDC,
                    0,
                    0,
                    SRCCOPY
                );

                SelectObject(memDC, oldBitmap);

                DeleteDC(memDC);
            }
        }

        EndPaint(hwnd, &ps);

        return 0;
    }

    case WM_DESTROY: {
        if (hBitmap) {
            DeleteObject(hBitmap);
            hBitmap = nullptr;
        }

        return 0;
    }
    }

    return DefWindowProcA(
        hwnd,
        msg,
        wParam,
        lParam
    );
}

void ShowSplashScreen(HINSTANCE hInstance) {
    WNDCLASSA wc = {};

    wc.lpfnWndProc = SplashProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "MY_TEST_SPLASH";

    RegisterClassA(&wc);

    // Load only to determine dimensions
    HBITMAP bitmap = LoadBitmapA(hInstance, MAKEINTRESOURCEA(IDB_SPLASH));

    if (!bitmap) {

        MessageBoxA(
            nullptr,
            "IDB_SPLASH could not be loaded.",
            "Error",
            MB_OK | MB_ICONERROR
        );

        return;
    }

    BITMAP bm = {};

    GetObjectA(bitmap, sizeof(BITMAP), &bm);

    DeleteObject(bitmap);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    int x = (screenWidth - bm.bmWidth) / 2;
    int y = (screenHeight - bm.bmHeight) / 2;

    HWND hwnd = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,

        "MY_TEST_SPLASH",

        "",

        WS_POPUP,

        x,
        y,

        bm.bmWidth,
        bm.bmHeight,

        nullptr,
        nullptr,

        hInstance,
        nullptr
    );

    if (!hwnd)
        return;

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    Sleep(3000);

    DestroyWindow(hwnd);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    INITCOMMONCONTROLSEX controls = {};

    controls.dwSize = sizeof(controls);
    controls.dwICC = ICC_LISTVIEW_CLASSES;

    InitCommonControlsEx(&controls);

    WNDCLASSEXA wc = {};

    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.hIcon = LoadIconA(hInstance, MAKEINTRESOURCEA(IDI_ICON1));
    wc.hIconSm = LoadIconA(hInstance, MAKEINTRESOURCEA(IDI_ICON1));
    wc.lpszClassName = "Lite Engine Map Editor";

    if (!RegisterClassExA(&wc))
        return 0;

    WNDCLASSA mapGridClass = {};

    mapGridClass.lpfnWndProc = MapGridProc;
    mapGridClass.hInstance = hInstance;
    mapGridClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    mapGridClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    mapGridClass.lpszClassName = "Lite Engine MapGrid";

    if (!RegisterClassA(&mapGridClass))
        return 0;

    ShowSplashScreen(hInstance);

    int windowWidth = 960;
    int windowHeight = 720;

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    int x = (screenWidth - windowWidth) / 2;
    int y = (screenHeight - windowHeight) / 2;

    g_mainWindow =
        CreateWindowExA(
            0,
            "Lite Engine Map Editor",
            "LWD Editor",
            WS_OVERLAPPEDWINDOW,
            x,
            y,
            windowWidth,
            windowHeight,
            nullptr,
            nullptr,
            hInstance,
            nullptr
        );


    if (!g_mainWindow)
        return 0;

    ShowWindow(g_mainWindow, nCmdShow);
    UpdateWindow(g_mainWindow);

    MSG msg = {};

    while (GetMessageA(&msg, nullptr, 0, 0)) {
        if (!TranslateAcceleratorA(g_mainWindow, g_accelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }

    return static_cast<int>(msg.wParam);
}