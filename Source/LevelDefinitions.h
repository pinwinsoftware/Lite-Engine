#pragma once
#include <string>
#include <vector>

struct LevelDefinition
{
    std::string mapName;
    std::string mapFile;

    float playerX = 0.0f;
    float playerY = 0.0f;
    float playerAngle = 0.0f;

    int playerAmmo;
    int playerHealth;

    std::string nextMap;
};

bool LoadLevelDefinitions(const std::string& ledPath);

const LevelDefinition* FindLevelDefinition(const std::string& mapName);