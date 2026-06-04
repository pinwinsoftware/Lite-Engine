#pragma once

extern int coins;
extern int totalCoins;

extern bool gameComplete;

void Game();
char getMapCell(int x, int y);

struct Map;
extern Map* currentMap;