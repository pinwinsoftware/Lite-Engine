#pragma once

extern int coins;
extern int totalCoins;

extern int kills;
extern int totalEnemies;

extern bool gameComplete;
extern bool gameFailed;

void SpawnExit();
void Game();
void LoadEntities();
char GetMapCell(int x, int y);

struct Map;
extern Map* currentMap;