#pragma once
#include <vector>

enum class EntityType
{
    ENEMY,
    COLLECTIBLE,
    EXIT
};

enum class EnemyState
{
    IDLE,
    CHASE,
    ATTACK
};

struct Entity
{
    EntityType type;

    float x, y;
    float angle;

    const char** shape;
    int w, h;

    EnemyState state = EnemyState::IDLE;

    float lastSeenX = 0.0f;
    float lastSeenY = 0.0f;

    float patrolAngle = 0.0f;
    float stateTimer = 0.0f;
    bool flipSprite = false;
    float randomOffsetX = 0.0f;
    float randomOffsetY = 0.0f;
    float randomTimer = 0.0f;
    float attackTimer = 0.0f;
    bool hasAttacked = false;
    bool attackingFrame = false;
    float stuckTimer = 0.0f;

    Entity(EntityType t, float px, float py)
        : type(t), x(px), y(py), angle(0)
    {
    }
};

extern std::vector<Entity> entities;

extern const char* monsterSprite[17];
extern const char* coinSprite[10];
extern const char* trollSprite[13];
extern const char* exitSprite[5];

void UpdateEnemies(float dt);
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