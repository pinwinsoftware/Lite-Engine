#pragma once

extern int coins;
extern int totalCoins;

extern int kills;
extern int totalEnemies;

extern bool gameComplete;

void Game();
char getMapCell(int x, int y);

struct Map;
extern Map* currentMap;