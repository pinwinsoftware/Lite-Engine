#pragma once
#include <vector>

enum class EntityType {
    ENEMY,
    CORPSE,
    FIREBALL,
    COLLECTIBLE,
    AMMO,
    MEDKIT,
    EXIT
};

enum class EnemyClass {
    MELEE,
    RANGED
};

enum class EnemyState {
    IDLE,
    CHASE,
    ATTACK
};

struct Entity {
    EntityType type;

    float x;
    float y;
    float angle;

    const char** shape = nullptr;
    int w = 0;
    int h = 0;

    float speed = 3.0f;
    int health = 0.f;

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

// Sprites
extern const char* monsterSprite[16];
extern const char* bigMonsterSprite[16];
extern const char* fireBallSprite[16];
extern const char* corpsesSprite[16];
extern const char* ammoSprite[16];
extern const char* medKitSprite[16];
extern const char* coinSprite[10];
extern const char* trollSprite[13];
extern const char* exitSprite[5];

// Entity updates
void UpdateEnemies(float dt);
void UpdateFireballs(float dt);
void SpawnFireball(float startX, float startY, float targetX, float targetY);

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