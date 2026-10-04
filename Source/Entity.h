#pragma once
#include <vector>
#include "EntityType.h"
#include "EntityDefinitions.h"

enum class EnemyClass {
    MELEE,
    RANGED
};

enum class EnemyState {
    IDLE,
    CHASE,
    ATTACK,
    DEATH
};

struct Entity {
    EntityType type;

    float x;
    float y;
    float angle;

    int id = 0;

    EntitySprite sprite;
    int w = 0;
    int h = 0;

    float speed = 3.0f;
    int health = 0;

    EnemyClass enemyClass = EnemyClass::MELEE;
    EnemyState state = EnemyState::IDLE;

    // Fireball movement
    float velocityX = 0.0f;
    float velocityY = 0.0f;

    // Fireball lifetime
    float lifeTimer = 0.0f;

    // AI information
    float lastSeenX = 0.0f;
    float lastSeenY = 0.0f;
    bool hasSeenPlayer = false;

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
    float scale = 1.0f;

    Entity(EntityType t, float px, float py)
        : type(t),
        x(px),
        y(py),
        angle(0.0f)
    {
    }
};

extern std::vector<Entity> entities;

// Entity updates
void UpdateEnemies(float dt);
void UpdateFireballs(float dt);
void SpawnFireball(const Entity& enemy, float targetX, float targetY);
void SetEnemyState(Entity& e, EnemyState newState);

// Sprite rendering
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