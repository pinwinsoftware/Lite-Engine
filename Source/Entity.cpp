#include <cmath>

#include "FindingPath.h"
#include "Entity.h"
#include "Game.h"
#include "main.h"

const char* monsterSprite[17] = {
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
    "    111     11  ",
    "     1      111 ",
    "    11          ",
    "   1111         ",
    "  11111         ",
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

    while (spriteAngle > 3.14159f)
        spriteAngle -= 6.28318f;

    while (spriteAngle < -3.14159f)
        spriteAngle += 6.28318f;

    float fovRad = fov * 3.14159f / 180.0f;

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
const float collisionRadius = 0.40f;

bool EnemyCollidingWithEnemy(const Entity& current, float newX, float newY) {
    const float enemyRadius = 0.35f;

    for (const Entity& other : entities) {

        // ignore itself
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
            return false; // wall blocks vision
        }
    }
    return true;
}

void UpdateEnemies(float dt) {
    const float speed = 3.0f;

    const float playerRadius = 0.25f;
    const float enemyRadius = 0.35f;

    struct Direction {
        int dx;
        int dy;
    };

    Direction dirs[4] = {
        { 1, 0 },   // right
        {-1, 0 },   // left
        { 0, 1 },   // down
        { 0,-1 }    // up
    };

    for (Entity& e : entities) {
        if (e.type != EntityType::ENEMY)
            continue;

        if (e.state == EnemyState::CHASE) {
            e.stateTimer += dt;

            if (e.stateTimer >= 0.25f) {
                e.stateTimer = 0.0f;
                e.flipSprite = !e.flipSprite;
            }
        }

        switch (e.state) {
        case EnemyState::IDLE: {
            float dx = x - e.x;
            float dy = y - e.y;

            float distance = sqrt(dx * dx + dy * dy);

            if (distance < 32.f && EnemyVision(e.x, e.y, x, y)) {
                e.state = EnemyState::CHASE;
            }

            break;
        }
        case EnemyState::CHASE: {
            float dxPlayer = x - e.x;
            float dyPlayer = y - e.y;

            float playerDistance = sqrt(dxPlayer * dxPlayer + dyPlayer * dyPlayer);

            if (playerDistance < 1.0f) {
                e.state = EnemyState::ATTACK;
                e.attackTimer = 0.0f;
                e.hasAttacked = false;
                break;
            }

            int enemyTileX = (int)e.x;
            int enemyTileY = (int)e.y;

            int playerTileX = (int)x;
            int playerTileY = (int)y;

            int bestDir = -1;

            std::vector<Node> path = FindPath(
                enemyTileX,
                enemyTileY,
                playerTileX,
                playerTileY
            );


            if (path.size() >= 2) {
                int nextX = path[1].x;
                int nextY = path[1].y;


                float dirX = nextX - enemyTileX;
                float dirY = nextY - enemyTileY;

                if (dirX != 0)
                {
                    bestDir = dirX > 0 ? 0 : 1;
                }

                else if (dirY != 0)
                {
                    bestDir = dirY > 0 ? 2 : 3;
                }
            }

            if (bestDir == -1) {
                e.stateTimer += dt; // wait instead of freezing forever

                if (e.stateTimer > 1.0f) {
                    e.stateTimer = 0.0f;

                    std::vector<int> possibleDirs;

                    for (int i = 0; i < 4; i++) {
                        int tx = enemyTileX + dirs[i].dx;
                        int ty = enemyTileY + dirs[i].dy;

                        if (GetMapCell(tx, ty) == '1')
                            continue;

                        if (EnemyOnTile(tx, ty, e))
                            continue;

                        possibleDirs.push_back(i);
                    }

                    if (!possibleDirs.empty()) {
                        bestDir = possibleDirs[rand() % possibleDirs.size()];
                    }
                    else {
                        break;
                    }
                }
                else {
                    break;
                }
            }

            e.randomTimer += dt;

            if (e.randomTimer > 1.0f) {
                e.randomTimer = 0.0f;

                e.randomOffsetX = ((rand() % 200) - 100) / 100.0f;
                e.randomOffsetY = ((rand() % 200) - 100) / 100.0f;
            }

            // target tile center
            float targetX = enemyTileX + dirs[bestDir].dx + 0.5f + e.randomOffsetX * 0.3f;
            float targetY = enemyTileY + dirs[bestDir].dy + 0.5f + e.randomOffsetY * 0.3f;

            // direction toward target
            float dx = targetX - e.x;
            float dy = targetY - e.y;

            float length = sqrt(dx * dx + dy * dy);

            if (length > 0.001f) {
                dx /= length;
                dy /= length;
            }

            float newX = e.x + dx * speed * dt;
            float newY = e.y + dy * speed * dt;

            // wall collision
            bool hitsWall = GetMapCell((int)newX, (int)newY) == '1';

            if (hitsWall) {
                e.stuckTimer += dt;
            }
            else {
                e.stuckTimer = 0.0f;
            }

            // player collision
            float pdx = newX - x;
            float pdy = newY - y;

            bool hitsPlayer = (pdx * pdx + pdy * pdy) < (playerRadius + enemyRadius) * (playerRadius + enemyRadius);
            bool hitsEnemy = EnemyCollidingWithEnemy(e, newX, newY);

            if (!hitsWall && !hitsPlayer && !hitsEnemy) {
                e.x = newX;
                e.y = newY;
            }
            if (e.stuckTimer > 0.5f) {
                e.stuckTimer = 0.0f;
                e.stateTimer = 1.0f;
            }
            else if (hitsEnemy) {
                float pushX = 0;
                float pushY = 0;

                for (const Entity& other : entities) {
                    if (&other == &e)
                        continue;

                    if (other.type != EntityType::ENEMY)
                        continue;

                    float dx = e.x - other.x;
                    float dy = e.y - other.y;

                    float dist = sqrt(dx * dx + dy * dy);

                    if (dist < 0.8f && dist > 0.001f) {
                        float force = (0.8f - dist) / 0.8f;

                        pushX += dx / dist * force;
                        pushY += dy / dist * force;
                    }
                }

                float pushLength = sqrt(
                    pushX * pushX +
                    pushY * pushY
                );

                if (pushLength > 0.001f) {
                    pushX /= pushLength;
                    pushY /= pushLength;

                    float newPushX =
                        e.x + pushX * 2.0f * dt;

                    float newPushY =
                        e.y + pushY * 2.0f * dt;

                    if (!EnemyWallCollision(newPushX, newPushY)) {
                        e.x = newPushX;
                        e.y = newPushY;
                    }
                }
            }
            else {
                e.stuckTimer = 0.0f;
            }
            break;
        }
        case EnemyState::ATTACK:
        {
            e.attackTimer += dt;

            float dxPlayer = x - e.x;
            float dyPlayer = y - e.y;

            float playerDistance = sqrt(dxPlayer * dxPlayer + dyPlayer * dyPlayer);

            // Disable walking animation while attacking
            e.flipSprite = false;

            // wait 400 miliseconds before attacking
            if (e.attackTimer >= 0.4f && !e.hasAttacked && playerDistance < 1.0f) {
                e.hasAttacked = true;

                health -= 10;
            }

            // quick attack "animation"
            if (e.attackTimer >= 0.4f && e.attackTimer < 0.6f) {
                e.flipSprite = true;
            }

            // return to chase
            if (e.attackTimer >= 0.7f) {
                e.attackTimer = 0.0f;
                e.hasAttacked = false;

                e.flipSprite = false;

                e.state = EnemyState::CHASE;
            }

            break;
        }
        }
    }
}