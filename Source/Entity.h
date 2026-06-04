#pragma once
#include <vector>

enum class EntityType { ENTITY, COLLECTIBLE, EXIT };

struct Entity {
    EntityType type;
    float x, y;
    float angle;

    const char** shape;
    int w, h;

    Entity(EntityType t, float px, float py)
        : type(t), x(px), y(py), angle(0) {
    }
};

extern std::vector<Entity> entities;
extern const char* monsterSprite[17];
extern const char* coinSprite[10];
extern const char* trollSprite[13];
extern const char* exitSprite[5];

void RenderSprite(
    const Entity& sprite,
    float playerX,
    float playerY,
    float playerRad,
    float fov,
    int screenWidth,
    int screenHeight,
    const std::vector<float>& depthBuffer,
    std::vector<std::vector<char>>& screen
);