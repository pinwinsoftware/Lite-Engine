#pragma once
#include <windows.h>
#include <string>
#include <vector>

#include "EntityData.h"

// Control ids

constexpr int ID_ENTITY_LIST = 1001;
constexpr int ID_OPEN_LED = 1002;
constexpr int ID_MAP_GRID = 1003;
constexpr int ID_UNDO = 1010;
constexpr int ID_REDO = 1011;  
constexpr int ID_CLEAR_MAP = 1012;
constexpr int ID_NEW_MAP = 1013;
constexpr int ID_OPEN_MAP = 1014;
constexpr int ID_SAVE_MAP = 1015;
constexpr int ID_SAVE_MAP_AS = 1016;
constexpr int ID_EXIT = 9999;

// Editor layout

constexpr int ENTITY_LIST_WIDTH = 180;
constexpr int EDITOR_MARGIN = 8;

// Map

constexpr int MAP_CELL_SIZE = 16;

constexpr int MAP_WIDTH = 32;
constexpr int MAP_HEIGHT = 32;

constexpr int MAP_PIXEL_WIDTH = MAP_WIDTH * MAP_CELL_SIZE;
constexpr int MAP_PIXEL_HEIGHT = MAP_HEIGHT * MAP_CELL_SIZE;

constexpr int MIN_MAP_SIZE = 16;
constexpr int MAX_MAP_SIZE = 128;

extern int g_mapSize;

// Colours

constexpr COLORREF MAP_BLUE = RGB(58, 150, 221);

constexpr COLORREF SPRITE_WHITE = RGB(255, 255, 255);

// Global state

extern HWND g_mainWindow;
extern HWND g_entityList;
extern HWND g_mapGrid;

extern std::vector<EntityDefinition> g_entities;
extern std::vector<std::string> g_mapRows;

extern int g_selectedEntity;

extern int g_mapScrollX;
extern int g_mapScrollY;

extern std::vector<int> g_entityImageIDs;

struct MapState {
    int size;
    std::vector<std::string> rows;
};

extern std::vector<MapState> g_undoStack;
extern std::vector<MapState> g_redoStack;

extern std::string g_currentMapFile;