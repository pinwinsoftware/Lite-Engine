#include <cmath>
#include <algorithm>

#include "FindingPath.h"
#include "Entity.h"
#include "Game.h"
#include "main.h"

const char* monsterSprite[16] = {
    "       111      ",
    "      10101     ",
    "      11111     ",
    "      11011     ",
    "       111      ",
    "     1111111    ",
    "    111111111   ",
    "    1 11111 1   ",
    "    1 11111 1   ",
    "    1 11111 1   ",
    "      11 11     ",
    "      11 11     ",
    "       1 1      ",
    "       1        ",
    "         11     ",
    "       11       ",
};

const char* bigMonsterSprite[16] = {
    "     111111     ",
    "    10000001111 ",
    "   1101001011111",
    "   1100110011111",
    "  11110000111111",
    " 111  11111 1111",
    "111     11  111 ",
    " 1   111  11111 ",
    "    11111 11111 ",
    "    1111   111  ",
    "    111         ",
    "    111     11  ",
    "     1      11  ",
    "    11      111 ",
    "   1111         ",
    "  11111         ",
};

const char* fireBallSprite[16] = {
    "                ",
    "                ",
    "                ",
    "                ",
    "      1111      ",
    "     111101     ",
    "    11111001    ",
    "    11111101    ",
    "    10111111    ",
    "    10011111    ",
    "     101111     ",
    "      1111      ",
    "                ",
    "                ",
    "                ",
    "                ",
};

const char* corpsesSprite[16] = {
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "     1  1 1     ",
    "      11111     ",
    "  1  1111111  1 ",
    " 111111111111111",
    "  111       111 ",
    "   1         1  ",
};

const char* ammoSprite[16] = {
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "      1 1 1     ",
    "      1 1 1     ",
    "     111111     ",
    "     111111     ",
    "     111111     ",
    "     111111     ",
};

const char* medKitSprite[16] = {
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "                ",
    "    11111111    ",
    "    11100111    ",
    "    11100111    ",
    "    10000001    ",
    "    10000001    ",
    "    11100111    ",
    "    11100111    ",
    "    11111111    ",
};

const char* coinSprite[10] = {
    "           ",
    "    111    ",
    "   10001   ",
    "  1001001  ",
    "  1011001  ",
    "  1001001  ",
    "  1001001  ",
    "   10001   ",
    "    111    ",
    "           ",
};

const char* trollSprite[13] = {
    "  11111111111   ",
    " 1000000000001  ",
    " 1000000000001  ",
    "101110111100001 ",
    "1001100011000001",
    " 100001000000001",
    " 100110001110001",
    " 11111111010001 ",
    " 1010101010001  ",
    "1011111110001   ",
    "100000000011    ",
    "1000001111      ",
    " 11111          ",
};

const char* exitSprite[5] = {
    "11111001000100111110011111",
    "10000000101000001000000100",
    "11111000010000001000000100",
    "10000000101000001000000100",
    "11111001000100111110000100",
};

std::vector<Entity> entities;

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
)
{
    float dx = sprite.x - playerX;
    float dy = sprite.y - playerY;

    bool flipSprite = sprite.flipSprite;

    float spriteDistance = sqrt(dx * dx + dy * dy);

    float spriteAngle = atan2(dy, dx) - playerRad;

    while (spriteAngle > pi)
        spriteAngle -= pi * 2;

    while (spriteAngle < -pi)
        spriteAngle += pi * 2;

    float fovRad = fov * pi / 180.0f;

    int spriteScreenX = screenWidth / 2 + (int)(tan(spriteAngle) / tan(fovRad / 2.0f) * screenWidth / 2);

    float spriteHalfWidthAngle = atan2(sprite.w / 2.0f, spriteDistance);

    if (fabs(spriteAngle) < (fovRad / 2.0f) + spriteHalfWidthAngle) {
        int spriteScreenX = (int)((0.5f + spriteAngle / fovRad) * screenWidth);

        float aspectRatio = 2.0f;

        int spriteHeight = (int)(screenHeight / spriteDistance);
        int spriteWidth = (int)(spriteHeight * aspectRatio);

        int top = (screenHeight - spriteHeight) / 2;
        int left = spriteScreenX - spriteWidth / 2;

        for (int sx = 0; sx < spriteWidth; sx++) {
            int screenX = left + sx;
            if (screenX < 0 || screenX >= screenWidth) continue;
            if (spriteDistance > depthBuffer[screenX] + 0.1f) continue;

            for (int sy = 0; sy < spriteHeight; sy++) {
                int screenY = top + sy;
                if (screenY < 0 || screenY >= screenHeight) continue;

                int texX;

                if (flipSprite) {
                    texX = sprite.w - 1 -
                        (sx * sprite.w / spriteWidth);
                }
                else {
                    texX = sx * sprite.w / spriteWidth;
                }
                int texY = sy * sprite.h / spriteHeight;

                if (sprite.shape[texY][texX] == '1')
                    screen[screenY][screenX] = char(219);
                else if (sprite.shape[texY][texX] == '0')
                    screen[screenY][screenX] = ' ';
            }
        }
    }
}

const float playerRadius = 0.25f;
const float enemyRadius = 0.35f;
const float collisionRadius = 0.40f;

constexpr float enemyViewRange = 32.f;
constexpr float meleeRange = 1.15f;

bool EnemyCollidingWithEnemy(const Entity& current, float newX, float newY) {
    for (const Entity& other : entities) {

        if (&current == &other)
            continue;

        if (other.type != EntityType::ENEMY)
            continue;

        float dx = newX - other.x;
        float dy = newY - other.y;

        float distanceSquared = dx * dx + dy * dy;

        float minimumDistance = enemyRadius * 2.0f;

        if (distanceSquared < minimumDistance * minimumDistance) {
            return true;
        }
    }

    return false;
}

bool EnemyWallCollision(float px, float py) {
    const float radius = 0.5f;

    // check four corners of the hitbox
    if (GetMapCell((int)(px + radius), (int)py) == '1')
        return true;

    if (GetMapCell((int)(px - radius), (int)py) == '1')
        return true;

    if (GetMapCell((int)px, (int)(py + radius)) == '1')
        return true;

    if (GetMapCell((int)px, (int)(py - radius)) == '1')
        return true;

    return false;
}

bool EnemyOnTile(int tileX, int tileY, const Entity& current) {
    for (const Entity& other : entities) {
        if (&other == &current)
            continue;

        if (other.type != EntityType::ENEMY)
            continue;

        int otherX = (int)other.x;
        int otherY = (int)other.y;

        if (otherX == tileX && otherY == tileY)
            return true;
    }
    return false;
}

bool EnemyVision(float enemyX, float enemyY, float playerX, float playerY) {
    float dx = playerX - enemyX;
    float dy = playerY - enemyY;

    float distance = sqrt(dx * dx + dy * dy);

    int steps = (int)(distance * 20);

    float stepX = dx / steps;
    float stepY = dy / steps;

    float checkX = enemyX;
    float checkY = enemyY;

    for (int i = 0; i < steps; i++) {
        checkX += stepX;
        checkY += stepY;

        if (GetMapCell((int)checkX, (int)checkY) == '1') {
            return false;
        }
    }
    return true;
}

void UpdateFireballs(float dt) {
    for (Entity& e : entities) {
        if (e.type != EntityType::FIREBALL)
            continue;

        float newX =
            e.x + e.velocityX * dt;

        float newY =
            e.y + e.velocityY * dt;

        // Destroy the fireball when it hits a wall
        if (GetMapCell((int)newX, (int)newY) == '1') {
            e.lifeTimer = 0.0f;
            continue;
        }

        e.x = newX;
        e.y = newY;

        e.lifeTimer -= dt;

        float dx = x - e.x;
        float dy = y - e.y;

        float hitDistance = collisionRadius;

        if (dx * dx + dy * dy < hitDistance * hitDistance) {
            int damage = RollDice(3, 7) + 7;
            health -= damage;

            e.lifeTimer = 0.0f;
        }
    }

    entities.erase(std::remove_if(entities.begin(), entities.end(), [](const Entity& e) {
        return
            e.type == EntityType::FIREBALL &&
            e.lifeTimer <= 0.0f;
        }
        ),
        entities.end()
    );
}

void SpawnFireball(float startX, float startY, float targetX, float targetY) {
    Entity fireball(EntityType::FIREBALL, startX, startY);

    fireball.shape = fireBallSprite;
    fireball.w = 16;
    fireball.h = 16;

    float dx = targetX - startX;
    float dy = targetY - startY;

    float length = sqrtf(dx * dx + dy * dy);

    if (length > 0.001f) {
        dx /= length;
        dy /= length;
    }

    const float fireballSpeed = 5.0f;

    fireball.velocityX = dx * fireballSpeed;

    fireball.velocityY = dy * fireballSpeed;

    fireball.lifeTimer = 5.0f;

    entities.push_back(fireball);
}

struct Direction {
    int dx;
    int dy;
};

const Direction directions[4] = {
    { 1, 0 },   // right
    {-1, 0 },   // left
    { 0, 1 },   // down
    { 0,-1 }    // up
};

void UpdateIdle(Entity& e) {
    float dx = x - e.x;
    float dy = y - e.y;

    float distance = sqrtf(dx * dx + dy * dy);

    if (distance < enemyViewRange && EnemyVision(e.x, e.y, x, y)) {
        e.state = EnemyState::CHASE;
        e.attackTimer = 0.f;
    }
}

void UpdateEnemyAnimations(float dt) {
    for (Entity& e : entities) {
        if (e.type == EntityType::ENEMY) {
            if (e.state == EnemyState::CHASE) {
                e.stateTimer += dt;

                if (e.stateTimer >= 0.25f) {
                    e.stateTimer = 0.0f;
                    e.flipSprite = !e.flipSprite;
                }
            }
        }
        else if (e.type == EntityType::FIREBALL) {
            e.stateTimer += dt;

            if (e.stateTimer >= 0.15f) {
                e.stateTimer = 0.0f;
                e.flipSprite = !e.flipSprite;
            }
        }
    }
}

void MoveEnemy(Entity& e, float dt) {
    float dxPlayer = x - e.x;
    float dyPlayer = y - e.y;

    float playerDistance = sqrtf(dxPlayer * dxPlayer + dyPlayer * dyPlayer);
    
    if (playerDistance <= meleeRange) {
        e.state = EnemyState::ATTACK;
        e.attackTimer = 0.0f;
        e.hasAttacked = false;
        e.flipSprite = false;
        return;
    }

    int enemyTileX = (int)e.x;
    int enemyTileY = (int)e.y;

    int playerTileX = (int)x;
    int playerTileY = (int)y;

    int bestDir = -1;

    std::vector<Node> path = FindPath(enemyTileX, enemyTileY, playerTileX, playerTileY);

    if (path.size() >= 2) {
        int nextX = path[1].x;
        int nextY = path[1].y;

        float dirX = nextX - enemyTileX;

        float dirY = nextY - enemyTileY;

        if (dirX != 0) {
            bestDir = dirX > 0 ? 0 : 1;
        }
        else if (dirY != 0) {
            bestDir = dirY > 0 ? 2 : 3;
        }
    }

    // Fallback
    if (bestDir == -1) {
        e.stateTimer += dt;

        if (e.stateTimer > 1.0f) {
            e.stateTimer = 0.0f;

            std::vector<int> possibleDirs;

            for (int i = 0; i < 4; i++) {
                int tx = enemyTileX + directions[i].dx;

                int ty = enemyTileY + directions[i].dy;

                if (GetMapCell(tx, ty) == '1')
                    continue;

                if (EnemyOnTile(tx, ty, e)) {
                    continue;
                }

                possibleDirs.push_back(i);
            }
            if (!possibleDirs.empty()) {
                bestDir = possibleDirs[rand() % possibleDirs.size()];
            }
            else {
                return;
            }
        }
        else {
            return;
        }
    }

    // Random movement offset
    e.randomTimer += dt;

    if (e.randomTimer > 1.0f) {
        e.randomTimer = 0.0f;
        e.randomOffsetX = ((rand() % 200) - 100) / 100.0f;
        e.randomOffsetY = ((rand() % 200) - 100) / 100.0f;
    }

    float targetX = enemyTileX + directions[bestDir].dx + 0.5f + e.randomOffsetX * 0.3f;
    float targetY = enemyTileY + directions[bestDir].dy + 0.5f + e.randomOffsetY * 0.3f;

    // Movement
    float moveX = targetX - e.x;
    float moveY = targetY - e.y;
    float moveLength = sqrtf(moveX * moveX + moveY * moveY);

    if (moveLength > 0.001f) {
        moveX /= moveLength;
        moveY /= moveLength;
    }

    float newX = e.x + moveX * e.speed * dt;
    float newY = e.y + moveY * e.speed * dt;

    // Collisions
    bool hitsWall = GetMapCell((int)newX, (int)newY) == '1';

    if (hitsWall) {
        e.stuckTimer += dt;
    }
    else {
        e.stuckTimer = 0.0f;
    }

    float pdx = newX - x;
    float pdy = newY - y;

    float collisionDistance = playerRadius + enemyRadius;

    bool hitsPlayer = (pdx * pdx + pdy * pdy) < (collisionDistance * collisionDistance);
    bool hitsEnemy = EnemyCollidingWithEnemy(e, newX, newY);

    if (!hitsWall && !hitsPlayer && !hitsEnemy) {
        e.x = newX;
        e.y = newY;
    }

    // Stuck recovery
    if (e.stuckTimer > 0.5f) {
        e.stuckTimer = 0.0f;
        e.stateTimer = 1.0f; // Force the random fallback on the next update.
    }
    else if (hitsEnemy) {
        float pushX = 0.0f;
        float pushY = 0.0f;

        for (const Entity& other : entities) {
            if (&other == &e)
                continue;

            if (other.type != EntityType::ENEMY) {
                continue;
            }

            float enemyDX = e.x - other.x;
            float enemyDY = e.y - other.y;

            float distance = sqrtf(enemyDX * enemyDX + enemyDY * enemyDY);

            if (distance < 0.8f && distance > 0.001f) {
                float force = (0.8f - distance) / 0.8f;

                pushX += enemyDX / distance * force;
                pushY += enemyDY / distance * force;
            }
        }

        float pushLength = sqrtf(pushX * pushX + pushY * pushY);

        if (pushLength > 0.001f) {
            pushX /= pushLength;
            pushY /= pushLength;

            float newPushX = e.x + pushX * 2.0f * dt;
            float newPushY = e.y + pushY * 2.0f * dt;

            if (!EnemyWallCollision(newPushX, newPushY)) {
                e.x = newPushX;
                e.y = newPushY;
            }
        }
    }
    else {
        e.stuckTimer = 0.0f;
    }
}

void UpdateMeleeAttack(Entity& e, float dt) {
    float dxPlayer = x - e.x;
    float dyPlayer = y - e.y;

    float playerDistance = sqrtf(dxPlayer * dxPlayer + dyPlayer * dyPlayer);

    // The player moved away.
    // Return to chasing and using fireballs.
    if (playerDistance > meleeRange) {
        e.state = EnemyState::CHASE;
        e.attackTimer = 0.0f;
        e.hasAttacked = false;
        e.flipSprite = false;
        return;
    }

    // Stop the walking animation
    e.flipSprite = false;
    e.attackTimer += dt;

    if (e.attackTimer >= 0.5f && !e.hasAttacked) {
        e.hasAttacked = true;

        // Ranged enemies have weaker melee attacks than melee enemies.
        if (e.enemyClass == EnemyClass::RANGED) {
            int damage = RollDice(2, 5) + 2;
            health -= damage;
        }
        else {
            int damage = RollDice(3, 5) + 7;
            health -= damage;
        }
    }

    // attack animation delay
    if (e.attackTimer >= 0.5f && e.attackTimer < 1.0f) {
        e.flipSprite = true;
    }

    // Start another melee attack if the player is still close.
    if (e.attackTimer >= 1.1f) {
        e.attackTimer = 0.0f;
        e.hasAttacked = false;
        e.flipSprite = false;
    }
}

void UpdateRangedAttack(Entity& e, float dt) {
    float dxPlayer = x - e.x;
    float dyPlayer = y - e.y;
    float playerDistance = sqrtf(dxPlayer * dxPlayer + dyPlayer * dyPlayer);
    bool canSeePlayer = EnemyVision(e.x, e.y, x, y);

    // Fireball attack
    const float shootRange = 12.f;
    const float fireCooldown = 2.f;

    if (canSeePlayer && playerDistance <= shootRange && playerDistance > meleeRange) {
        e.attackTimer += dt;

        if (e.attackTimer >= fireCooldown) {
            SpawnFireball(e.x, e.y, x, y);

            e.attackTimer = 0.0f;
        }
    }
    else {
        e.attackTimer = 0.0f; // fireball cooldown after losing sight.
    }
}

void UpdateEnemies(float dt) {
    UpdateEnemyAnimations(dt);

    for (Entity& e : entities) {
        if (e.type != EntityType::ENEMY)
            continue;

        switch (e.state) {
        case EnemyState::IDLE:
            UpdateIdle(e);
            break;

        case EnemyState::CHASE:
        {
            if (e.enemyClass == EnemyClass::RANGED)
                UpdateRangedAttack(e, dt);

            MoveEnemy(e, dt);
            break;
        }
        case EnemyState::ATTACK:
            UpdateMeleeAttack(e, dt);
            break;
        }
    }
}