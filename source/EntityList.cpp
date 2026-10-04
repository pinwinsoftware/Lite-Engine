#include <string>
#include <algorithm>
#include <vector>

#include "EntityList.h"
#include "EditorState.h"
#include "EntityRenderer.h"

void DrawEntityListItem(DRAWITEMSTRUCT* drawItem) {
    if (!drawItem)
        return;

    if (drawItem->itemID == LB_ERR)
        return;

    int entityIndex = static_cast<int>(drawItem->itemData);

    if (entityIndex < 0 || entityIndex >= static_cast<int>(g_entities.size())) {
        return;
    }

    const EntityDefinition& entity = g_entities[entityIndex];

    RECT rect = drawItem->rcItem;

    bool selected = (drawItem->itemState & ODS_SELECTED) != 0;

    COLORREF background = selected ? GetSysColor(COLOR_HIGHLIGHT) : RGB(255, 255, 255);
    COLORREF textColor = selected ? GetSysColor(COLOR_HIGHLIGHTTEXT) : RGB(0, 0, 0);

    HBRUSH backgroundBrush = CreateSolidBrush(background);

    FillRect(drawItem->hDC, &rect, backgroundBrush);
    DeleteObject(backgroundBrush);

    HIMAGELIST imageList = reinterpret_cast<HIMAGELIST>(GetPropA(drawItem->hwndItem, "EntityImageList"));

    if (imageList) {
        constexpr int spriteSize = 32;

        int spriteX = rect.left + 2;
        int spriteY = rect.top + (rect.bottom - rect.top - spriteSize) / 2;

        ImageList_Draw(
            imageList,
            drawItem->itemID,
            drawItem->hDC,
            spriteX,
            spriteY,
            ILD_NORMAL
        );
    }

    std::string text = std::to_string(entity.id) + "  -  " + entity.name;

    SetBkMode(drawItem->hDC, TRANSPARENT);
    SetTextColor(drawItem->hDC, textColor);

    RECT textRect = rect;

    textRect.left += 48;
    textRect.right -= 4;

    DrawTextA(
        drawItem->hDC,
        text.c_str(),
        -1,
        &textRect,
        DT_SINGLELINE |
        DT_VCENTER |
        DT_NOPREFIX
    );

    if (drawItem->itemState & ODS_FOCUS) {
        DrawFocusRect(drawItem->hDC, &rect);
    }
}

int GetEntityImageIndex(int entityID) {
    for (int i = 0; i < static_cast<int>(g_entityImageIDs.size()); ++i) {
        if (g_entityImageIDs[i] == entityID)
            return i;
    }

    return -1;
}

void RefreshEntityList() {
    if (!g_entityList)
        return;

    SendMessageA(
        g_entityList,
        WM_SETREDRAW,
        FALSE,
        0
    );

    SendMessageA(
        g_entityList,
        LB_RESETCONTENT,
        0,
        0
    );

    HIMAGELIST imageList = reinterpret_cast<HIMAGELIST>(GetPropA(g_entityList, "EntityImageList"));
    
    if (imageList) {
        ImageList_RemoveAll(imageList);
    }

    g_entityImageIDs.clear();

    std::vector<int> sortedIndices;

    sortedIndices.reserve(g_entities.size());

    for (int i = 0; i < static_cast<int>(g_entities.size()); ++i) {
        sortedIndices.push_back(i);
    }

    std::sort(sortedIndices.begin(), sortedIndices.end(), [](int a, int b){
            return g_entities[a].id < g_entities[b].id;
        }
    );

    for (int entityIndex : sortedIndices) {
        const EntityDefinition& entity = g_entities[entityIndex];

        if (imageList) {
            
            CreateEntityImage(imageList, entity);

            g_entityImageIDs.push_back(entity.id);
        }

        LRESULT index = SendMessageA(g_entityList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(""));

        if (index != LB_ERR && index != LB_ERRSPACE) {
            SendMessageA(
                g_entityList,
                LB_SETITEMDATA,
                index,
                static_cast<LPARAM>(entityIndex)
            );
        }
    }

    SendMessageA(
        g_entityList,
        WM_SETREDRAW,
        TRUE,
        0
    );

    InvalidateRect(
        g_entityList,
        nullptr,
        TRUE
    );

    UpdateWindow(g_entityList);

    // Select first entity

    if (!g_entities.empty()) {
        SendMessageA(
            g_entityList,
            LB_SETCURSEL,
            0,
            0
        );

        LRESULT data = SendMessageA(g_entityList, LB_GETITEMDATA, 0, 0);

        if (data != LB_ERR) {
            g_selectedEntity = static_cast<int>(data);
        }
        else {
            g_selectedEntity = -1;
        }
    }
    else {
        g_selectedEntity = -1;
    }
}

void CreateEntityList(HWND hwnd) {
    g_entityList =
        CreateWindowExA(
            WS_EX_CLIENTEDGE,
            "LISTBOX",
            "",
            WS_CHILD |
            WS_VISIBLE |
            WS_VSCROLL |
            LBS_OWNERDRAWFIXED |
            LBS_HASSTRINGS |
            LBS_NOINTEGRALHEIGHT |
            LBS_NOTIFY |
            WS_TABSTOP,

            0,
            0,
            0,
            0,

            hwnd,
            reinterpret_cast<HMENU>(ID_ENTITY_LIST),

            GetModuleHandleA(nullptr),
            nullptr
        );

    if (!g_entityList)
        return;

    // Item height

    SendMessageA(
        g_entityList,
        LB_SETITEMHEIGHT,
        0,
        36
    );

    HFONT font =
        CreateFontW(
            -8,
            0,
            0,
            0,
            FW_NORMAL,
            FALSE,
            FALSE,
            FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            NONANTIALIASED_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE,
            L"MS Sans Serif"
        );

    SendMessageA(
        g_entityList,
        WM_SETFONT,
        reinterpret_cast<WPARAM>(font),
        TRUE
    );

    // Image list
    HIMAGELIST imageList =
        ImageList_Create(
            32,
            32,
            ILC_COLOR32,
            16,
            16
        );

    if (imageList) { SetPropA(g_entityList, "EntityImageList", reinterpret_cast<HANDLE>(imageList)); }
}