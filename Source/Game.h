#pragma once
#include <string>

extern int coins;
extern int totalCoins;

extern int kills;
extern int totalEnemies;

extern bool gameComplete;
extern bool gameFailed;
extern int GunFrame;

void Game();
void LoadEntities();
char GetMapCell(int x, int y);
int RollDice(int amount, int sides);

struct Map;
extern Map* currentMap;

void getMapEntities();

bool LoadLevel(const std::string& levelName);