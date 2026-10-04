#include <windows.h>
#include <string>

#include "resource.h"
#include "Editor.h"
#include "EditorState.h"
#include "EntityList.h"
#include "Filedialog.h"
#include "MapGrid.h"
#include "MapFile.h"

HWND g_mapSizeLeft = nullptr;
HWND g_mapSizeEdit = nullptr;
HWND g_mapSizeRight = nullptr;

WNDPROC g_oldLeftButtonProc = nullptr;
WNDPROC g_oldRightButtonProc = nullptr;

HACCEL g_accelTable = nullptr;

int g_mapSize = 64;
int g_resizeDirection = 0;

#define ID_MAP_SIZE_LEFT   2001
#define ID_MAP_SIZE_EDIT   2002
#define ID_MAP_SIZE_RIGHT  2003
#define ID_MAP_RESIZE_TIMER 3001

#define ID_HELP_CONTENTS 4000
#define ID_HELP_ABOUT 4001

bool g_mapSizeButtonHeld = false;
bool g_mapSizeAutoRepeat = false;

LRESULT CALLBACK MapSizeButtonProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
);

void CreateEmptyMap() {
    g_mapRows.assign(g_mapSize, std::string(g_mapSize, ' '));
}

void UpdateMapSizeDisplay() {
    if (!g_mapSizeEdit)
        return;

    char text[32];

    wsprintfA(text, "%dx%d", g_mapSize, g_mapSize);
    SetWindowTextA(g_mapSizeEdit, text);
}

void CreateMapSizeControls(HWND hwnd) {
    g_mapSizeLeft = CreateWindowExA(
            0,
            "BUTTON",
            "<",
            WS_CHILD |
            WS_VISIBLE |
            BS_PUSHBUTTON,

            0, 0,
            30, 24,

            hwnd,
            reinterpret_cast<HMENU>(ID_MAP_SIZE_LEFT),
            GetModuleHandleA(nullptr),
            nullptr
        );

    g_mapSizeEdit = CreateWindowExA(
            WS_EX_CLIENTEDGE,
            "EDIT",
            "64x64",
            WS_CHILD |
            WS_VISIBLE |
            ES_READONLY |
            ES_CENTER,

            0, 0,
            120, 24,

            hwnd,
            reinterpret_cast<HMENU>(ID_MAP_SIZE_EDIT),
            GetModuleHandleA(nullptr),
            nullptr
        );

    g_mapSizeRight = CreateWindowExA(
            0,
            "BUTTON",
            ">",
            WS_CHILD |
            WS_VISIBLE |
            BS_PUSHBUTTON,

            0, 0,
            30, 24,

            hwnd,
            reinterpret_cast<HMENU>(ID_MAP_SIZE_RIGHT),
            GetModuleHandleA(nullptr),
            nullptr
        );

    // Subclass the resize buttons so they can handle press-and-hold resizing
    g_oldLeftButtonProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(g_mapSizeLeft, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(MapSizeButtonProc)));
    g_oldRightButtonProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(g_mapSizeRight, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(MapSizeButtonProc)));
}

void ResizeMapData(int newSize) {
    if (newSize < MIN_MAP_SIZE || newSize > MAX_MAP_SIZE)
        return;

    g_mapRows.resize(newSize);

    for (std::string& row : g_mapRows)
        row.resize(newSize, ' ');

    g_mapSize = newSize;
}

void ResizeEditor(HWND hwnd) {
    RECT rect = {};

    GetClientRect(hwnd, &rect);

    const int width = rect.right - rect.left;
    const int height = rect.bottom - rect.top;

    const int mapSizeButtonWidth = 30;
    const int mapSizeEditWidth = 120;
    const int mapSizeControlHeight = 24;
    const int mapSizeControlGap = 6;

    const int mapSizeTotalWidth = mapSizeButtonWidth + mapSizeEditWidth + mapSizeButtonWidth;

    int listX = width - ENTITY_LIST_WIDTH - EDITOR_MARGIN;
    int listHeight = height - EDITOR_MARGIN * 2 - mapSizeControlGap - mapSizeControlHeight;

    if (listX < 0)
        listX = 0;

    if (listHeight < 0)
        listHeight = 0;

    if (g_entityList) {
        MoveWindow(
            g_entityList,
            listX,
            EDITOR_MARGIN,
            ENTITY_LIST_WIDTH,
            listHeight,
            TRUE
        );
    }

    int mapSizeX = listX + (ENTITY_LIST_WIDTH - mapSizeTotalWidth) / 2; // Center controls underneath entity list
    int mapSizeY = EDITOR_MARGIN + listHeight + mapSizeControlGap;

    MoveWindow(
        g_mapSizeLeft,
        mapSizeX,
        mapSizeY,
        mapSizeButtonWidth,
        mapSizeControlHeight,
        TRUE
    );

    MoveWindow(
        g_mapSizeEdit,
        mapSizeX + mapSizeButtonWidth,
        mapSizeY,
        mapSizeEditWidth,
        mapSizeControlHeight,
        TRUE
    );

    MoveWindow(
        g_mapSizeRight,
        mapSizeX +
        mapSizeButtonWidth +
        mapSizeEditWidth,
        mapSizeY,
        mapSizeButtonWidth,
        mapSizeControlHeight,
        TRUE
    );

    int availableWidth = width - ENTITY_LIST_WIDTH - EDITOR_MARGIN * 3;

    int availableHeight = height - EDITOR_MARGIN * 2;

    if (availableWidth < 0)
        availableWidth = 0;

    if (availableHeight < 0)
        availableHeight = 0;

    // Keep the map viewport square so the grid retains equal width and height
    
    int mapViewportSize = min(availableWidth, availableHeight);

    if (g_mapGrid) {
        MoveWindow(
            g_mapGrid,
            EDITOR_MARGIN,
            EDITOR_MARGIN,
            mapViewportSize,
            mapViewportSize,
            TRUE
        );

        InvalidateRect(
            g_mapGrid,
            nullptr,
            TRUE
        );
    }
}

void ResizeMapByStep(HWND hwnd) {
    if (g_resizeDirection < 0) {
        ResizeMapData(g_mapSize - 1);
    }
    else if (g_resizeDirection > 0) {
        ResizeMapData(g_mapSize + 1);
    }

    UpdateMapSizeDisplay();
    ResizeEditor(hwnd);
    InvalidateRect(g_mapGrid, nullptr, TRUE);
}

LRESULT CALLBACK MapSizeButtonProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam) {

    int id = GetDlgCtrlID(hwnd);

    WNDPROC oldProc = nullptr;

    if (id == ID_MAP_SIZE_LEFT) {
        oldProc = g_oldLeftButtonProc;
    }
    else if (id == ID_MAP_SIZE_RIGHT) {
        oldProc = g_oldRightButtonProc;
    }

    if (message == WM_LBUTTONDOWN) {
        HWND parent = GetParent(hwnd);

        SaveUndoState();

        if (id == ID_MAP_SIZE_LEFT) {
            g_resizeDirection = -1;
        }
        else if (id == ID_MAP_SIZE_RIGHT) {
            g_resizeDirection = 1;
        }

        g_mapSizeButtonHeld = true;

        ResizeMapByStep(parent);

        /*
            Don't start repeating immediately
            Give the user ~400 ms before auto-repeat starts
        */
        
        SetTimer(
            parent,
            ID_MAP_RESIZE_TIMER,
            400,
            nullptr
        );

        SetCapture(hwnd);

        if (oldProc) {
            return CallWindowProcA(
                oldProc,
                hwnd,
                message,
                wParam,
                lParam
            );
        }

        return 0;
    }

    if (message == WM_LBUTTONUP) {
        HWND parent = GetParent(hwnd);

        g_mapSizeButtonHeld = false;
        g_mapSizeAutoRepeat = false;
        g_resizeDirection = 0;

        KillTimer(parent, ID_MAP_RESIZE_TIMER);
        ReleaseCapture();

        if (oldProc) {
            return CallWindowProcA(
                oldProc,
                hwnd,
                message,
                wParam,
                lParam
            );
        }

        return 0;
    }

    if (oldProc) {
        return CallWindowProcA(
            oldProc,
            hwnd,
            message,
            wParam,
            lParam
        );
    }

    return DefWindowProcA(
        hwnd,
        message,
        wParam,
        lParam
    );
}

LRESULT CALLBACK WindowProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        ACCEL accelerators[] =
        {
            { FVIRTKEY | FCONTROL, 'N', ID_NEW_MAP  },
            { FVIRTKEY | FCONTROL, 'O', ID_OPEN_MAP },
            { FVIRTKEY | FCONTROL, 'S', ID_SAVE_MAP }
        };

        g_accelTable = CreateAcceleratorTableA(accelerators, ARRAYSIZE(accelerators));

        HMENU menuBar = CreateMenu();
        HMENU fileMenu = CreatePopupMenu();
        HMENU helpMenu = CreatePopupMenu();

        HBITMAP hNew = LoadBitmapA(GetModuleHandleA(nullptr), MAKEINTRESOURCEA(IDB_FILE));
        HBITMAP hOpen = LoadBitmapA(GetModuleHandleA(nullptr), MAKEINTRESOURCEA(IDB_FILE));
        HBITMAP hSave = LoadBitmapA(GetModuleHandleA(nullptr), MAKEINTRESOURCEA(IDB_SAVE));
        HBITMAP hSaveAs = LoadBitmapA(GetModuleHandleA(nullptr), MAKEINTRESOURCEA(IDB_SAVE));
        HBITMAP hOpenLED = LoadBitmapA(GetModuleHandleA(nullptr), MAKEINTRESOURCEA(IDB_FILE));

        // New Map
        MENUITEMINFOA mii = {};
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_ID | MIIM_STRING | MIIM_BITMAP;
        mii.fType = MFT_STRING;
        mii.wID = ID_NEW_MAP;
        mii.dwTypeData = const_cast<LPSTR>("New\tCtrl+N");
        mii.hbmpItem = hNew;

        InsertMenuItemA(fileMenu, GetMenuItemCount(fileMenu), TRUE, &mii);

        // Open Map
        mii.wID = ID_OPEN_MAP;
        mii.dwTypeData = const_cast<LPSTR>("Open...\tCtrl+O");
        mii.hbmpItem = hOpen;

        InsertMenuItemA(fileMenu, GetMenuItemCount(fileMenu), TRUE, &mii);

        // Save
        mii.wID = ID_SAVE_MAP;
        mii.dwTypeData = const_cast<LPSTR>("Save\tCtrl+S");
        mii.hbmpItem = hSave;

        InsertMenuItemA(
            fileMenu,
            GetMenuItemCount(fileMenu),
            TRUE,
            &mii
        );

        // Save As...
        mii.wID = ID_SAVE_MAP_AS;
        mii.dwTypeData = const_cast<LPSTR>("Save As...");
        mii.hbmpItem = hSaveAs;

        InsertMenuItemA(
            fileMenu,
            GetMenuItemCount(fileMenu),
            TRUE,
            &mii
        );


        // Separator
        AppendMenuA(
            fileMenu,
            MF_SEPARATOR,
            0,
            nullptr
        );

        // Open LED
        mii.wID = ID_OPEN_LED;
        mii.dwTypeData = const_cast<LPSTR>("Open LED...");
        mii.hbmpItem = hOpenLED;

        InsertMenuItemA(
            fileMenu,
            GetMenuItemCount(fileMenu),
            TRUE,
            &mii
        );

        // Separator
        AppendMenuA(
            fileMenu,
            MF_SEPARATOR,
            0,
            nullptr
        );

        // Exit
        AppendMenuA(
            fileMenu,
            MF_STRING,
            ID_EXIT,
            "Exit"
        );

        HMENU editMenu =
            CreatePopupMenu();

        AppendMenuA(
            menuBar,
            MF_POPUP,
            reinterpret_cast<UINT_PTR>(fileMenu),
            "File"
        );

        AppendMenuA(
            editMenu,
            MF_STRING,
            ID_UNDO,
            "Undo\tCtrl+Z"
        );

        AppendMenuA(
            editMenu,
            MF_STRING,
            ID_REDO,
            "Redo\tCtrl+Y"
        );

        AppendMenuA(
            editMenu,
            MF_SEPARATOR,
            0,
            nullptr
        );

        AppendMenuA(
            editMenu,
            MF_STRING,
            ID_CLEAR_MAP,
            "Clear Map"
        );

        AppendMenuA(
            menuBar,
            MF_POPUP,
            reinterpret_cast<UINT_PTR>(editMenu),
            "Edit"
        );

        AppendMenuA(
            helpMenu,
            MF_STRING,
            ID_HELP_CONTENTS,
            "Creating Maps"
        );

        AppendMenuA(
            helpMenu,
            MF_SEPARATOR,
            0,
            nullptr
        );

        AppendMenuA(
            helpMenu,
            MF_STRING,
            ID_HELP_ABOUT,
            "About"
        );

        AppendMenuA(
            menuBar,
            MF_POPUP,
            (UINT_PTR)
            helpMenu,
            "Help"
        );

        SetMenu(hwnd, menuBar);
        CreateEmptyMap();
        CreateMapGrid(hwnd);
        CreateEntityList(hwnd);
        CreateMapSizeControls(hwnd);
        break;
    }
    case WM_SIZE: {
        ResizeEditor(hwnd);
        break;
    }

    case WM_TIMER: {
        if (wParam == ID_MAP_RESIZE_TIMER) {
            if (!g_mapSizeButtonHeld) {
                KillTimer(hwnd, ID_MAP_RESIZE_TIMER);

                return 0;
            }

            if (!g_mapSizeAutoRepeat) {
                g_mapSizeAutoRepeat = true;

                SetTimer(
                    hwnd,
                    ID_MAP_RESIZE_TIMER,
                    50,
                    nullptr
                );

                return 0;
            }
            ResizeMapByStep(hwnd);
            return 0;
        }
        break;
    }
    case WM_DRAWITEM: {
        DRAWITEMSTRUCT* drawItem = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);

        if (drawItem && drawItem->CtlID == ID_ENTITY_LIST) {
            DrawEntityListItem(drawItem);
            return TRUE;
        }

        break;
    }
    case WM_MEASUREITEM: {
        MEASUREITEMSTRUCT* measureItem = reinterpret_cast<MEASUREITEMSTRUCT*>(lParam);

        if (measureItem && measureItem->CtlID == ID_ENTITY_LIST) {
            measureItem->itemHeight = 36;
            return TRUE;
        }
        break;
    }
    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case ID_NEW_MAP: {
            SaveUndoState();
            NewMap();
            g_currentMapFile.clear();

            InvalidateRect(
                g_mapGrid,
                nullptr,
                TRUE
            );

            break;
        }
        case ID_OPEN_MAP: {
            
            std::string filename = OpenMapFile();

            if (filename.empty())
                break;

            // Preserve the current map before replacing it
            SaveUndoState();

            if (!OpenMap(filename)) {
                /* 
                    The file failed to open
                    The undo state we just created should not remain
                */
                if (!g_undoStack.empty()) {
                    g_undoStack.pop_back();
                }

                MessageBoxA(
                    hwnd,
                    "Failed to open map file.",
                    "Open Map Error",
                    MB_OK | MB_ICONERROR
                );

                break;
            }


            g_currentMapFile = filename;

            InvalidateRect(
                g_mapGrid,
                nullptr,
                TRUE
            );

            std::string fileNameOnly = filename.substr(filename.find_last_of("\\/") + 1);
            std::string title = "LWD Editor - " + fileNameOnly;

            SetWindowTextA(hwnd, title.c_str());

            break;
        }
        case ID_SAVE_MAP: {
            if (g_currentMapFile.empty()) {
                
                std::string filename = SaveMapFile();

                if (filename.empty())
                    break;

                if (!SaveMap(filename)) {
                    MessageBoxA(
                        hwnd,
                        "Failed to save map file.",
                        "Error",
                        MB_OK | MB_ICONERROR
                    );

                    break;
                }
                g_currentMapFile = filename;
            }
            else {
                if (!SaveMap(g_currentMapFile)) {
                    MessageBoxA(
                        hwnd,
                        "Failed to save map file.",
                        "Error",
                        MB_OK | MB_ICONERROR
                    );

                    break;
                }
            }

            break;
        }
        case ID_SAVE_MAP_AS: {
            std::string filename = SaveMapFile();

            if (filename.empty())
                break;

            if (!SaveMap(filename)) {
                MessageBoxA(
                    hwnd,
                    "Failed to save map file.",
                    "Error",
                    MB_OK | MB_ICONERROR
                );

                break;
            }

            g_currentMapFile = filename;

            std::string fileNameOnly = filename.substr(filename.find_last_of("\\/") + 1);
            std::string title = "LWD Editor - " + fileNameOnly;

            SetWindowTextA(hwnd, title.c_str());

            break;
        }
        case ID_OPEN_LED: {
            std::string filename = OpenLEDFile();

            if (filename.empty())
                break;

            if (!LoadLEDFile(filename, g_entities)) {
                MessageBoxA(
                    hwnd,
                    "Failed to load LED file.",
                    "Error",
                    MB_OK | MB_ICONERROR
                );

                break;
            }

            RefreshEntityList();

            std::string title = "LWD Editor - " + filename;

            SetWindowTextA(hwnd, title.c_str());

            InvalidateRect(
                g_mapGrid,
                nullptr,
                TRUE
            );

            break;
        }
        case ID_UNDO: {
            UndoMapChange();

            InvalidateRect(
                g_mapGrid,
                nullptr,
                TRUE
            );

            break;
        }
        case ID_REDO: {
            RedoMapChange();

            InvalidateRect(
                g_mapGrid,
                nullptr,
                TRUE
            );

            break;
        }
        case ID_CLEAR_MAP: {
            bool hasContent = false;

            for (const std::string& row : g_mapRows) {
                for (char tile : row) {
                    if (tile != ' ') {
                        hasContent = true;
                        break;
                    }
                }

                if (hasContent)
                    break;
            }

            if (!hasContent)
                break;

            // Save the map before clearing it
            SaveUndoState();
            CreateEmptyMap();
            InvalidateRect(g_mapGrid, nullptr, TRUE);
            break;
        }
        case ID_ENTITY_LIST: {
            if (HIWORD(wParam) == LBN_SELCHANGE) {
                
                int selected = static_cast<int>(SendMessageA(g_entityList, LB_GETCURSEL, 0, 0));

                if (selected != LB_ERR) {

                    LRESULT data = SendMessageA(g_entityList, LB_GETITEMDATA, selected, 0);

                    if (data != LB_ERR) {
                        g_selectedEntity = static_cast<int>(data);
                    }
                }
            }

            break;
        }
        case ID_HELP_CONTENTS: {
            STARTUPINFOA si = {};
            si.cb = sizeof(si);

            PROCESS_INFORMATION pi = {};

            CreateProcessA(
                nullptr,
                (LPSTR)"hh.exe \"LWD Editor.chm::/CreatingMaps.htm\"",
                nullptr,
                nullptr,
                FALSE,
                0,
                nullptr,
                nullptr,
                &si,
                &pi
            );

            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            break;
        }
        case ID_HELP_ABOUT: {
            MessageBoxA(
                hwnd,
                "LWD Editor\nVersion 1.0.0.0\nDeveloped by Pinwin",
                "About",
                MB_OK | MB_ICONINFORMATION);

            break;
        }
        case ID_EXIT: {
            DestroyWindow(hwnd);
            break;
        }
        }

        break;
    }
    case WM_DESTROY: {
        if (g_accelTable) {
            DestroyAcceleratorTable(g_accelTable);
            g_accelTable = nullptr;
        }

        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProcA(
            hwnd,
            message,
            wParam,
            lParam
        );
    }

    return 0;
}