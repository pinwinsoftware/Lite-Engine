#pragma once
#include <string>
#include <vector>

struct EntitySprite {
    int width = 0;
    int height = 0;

    std::vector<std::string> pixels;
};

struct EntityDefinition {
    int id = -1;

    std::string name;

    int width = 16;
    int height = 16;

    float health = 100.0f;
    float speed = 3.0f;

    bool ranged = false;

    std::string idleSprite;
    std::string deathSprite;

    EntitySprite idle;
    EntitySprite death;
};

bool LoadLEDFile(const std::string& ledPath, std::vector<EntityDefinition>& entities);