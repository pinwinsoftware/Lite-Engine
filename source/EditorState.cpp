#include "EditorState.h"

// Globals

HWND g_mainWindow = nullptr;
HWND g_entityList = nullptr;
HWND g_mapGrid = nullptr;

std::vector<EntityDefinition> g_entities;
std::vector<std::string> g_mapRows;

std::vector<MapState> g_undoStack;
std::vector<MapState> g_redoStack;

int g_selectedEntity = -1;

int g_mapScrollX = 0;
int g_mapScrollY = 0;

std::vector<int> g_entityImageIDs;

std::string g_currentMapFile;