#include <iostream>
#include <windows.h>
#include <string>
#include <cstring>
#include <cmath>
#include <vector>
#include <algorithm>

#include "main.h"
#include "Entity.h"
#include "Shoot.h"

std::vector<float> depthBuffer;

int GunFrame = 1;
int shootTimer = 0;

int coins = 0;
int totalCoins = 0;

int kills = 0;
int totalEnemies = 0;

bool gameComplete = false;
bool gameFailed = false;

const float PLAYER_RADIUS = 0.35f;
const float ENEMY_RADIUS = 0.35f;

bool PlayerCollidingWithEnemy(float playerX, float playerY) {
    for (const Entity& e : entities) {

        if (e.type != EntityType::ENEMY)
            continue;

        float dx = playerX - e.x;
        float dy = playerY - e.y;

        float minimumDistance = PLAYER_RADIUS + ENEMY_RADIUS;

        float distanceSquared = dx * dx + dy * dy;
        float minimumDistanceSquared = minimumDistance * minimumDistance;

        if (distanceSquared < minimumDistanceSquared) {
            return true;
        }
    }
    return false;
}

char GetMapCell(int x, int y) {
    if (y < 0 || y >= currentMap->height)
        return '1';

    if (x < 0 || x >= (int)currentMap->rows[y].size())
        return '1';

    char tile = currentMap->rows[y][x];

    if (tile == '2' || tile == '4')
        return ' ';

    return tile;
}

void getMapEntities() {
    for (int y = 0; y < currentMap->height; y++) {
        int rowLen = (int)currentMap->rows[y].size();
        for (int x = 0; x < rowLen; x++) {
            char tile = currentMap->rows[y][x];
            switch (tile) {
            case '2':
            {
                Entity e(EntityType::ENEMY, x + 0.5f, y + 0.5f);

                e.shape = monsterSprite;
                e.w = 16;
                e.h = 17;

                entities.push_back(e);

                break;
            }
            case '3':
            {
                Entity e(EntityType::COLLECTIBLE, x + 0.5f, y + 0.5f);
                e.shape = coinSprite;
                e.w = 12;
                e.h = 9;
                entities.push_back(e);
                break;
            }
            case '4':
            {
                Entity e(EntityType::ENEMY, x + 0.5f, y + 0.5f);
                e.shape = trollSprite;
                e.w = 16;
                e.h = 13;
                entities.push_back(e);
                break;
            }
            }
        }
    }
}

void LoadEntities() {
    entities.clear();
    getMapEntities();
}

void collectCoin(int tileX, int tileY) {
    coins++;

    currentMap->rows[tileY][tileX] = ' ';

    entities.erase(
        std::remove_if(
            entities.begin(),
            entities.end(),
            [tileX, tileY](const Entity& e) {
                return e.type == EntityType::COLLECTIBLE &&
                    (int)e.x == tileX &&
                    (int)e.y == tileY;
            }),
        entities.end());
}

void SpawnExit() {
    for (int y = 0; y < currentMap->height; y++) {
        for (int x = 0; x < (int)currentMap->rows[y].size(); x++) {
            if (currentMap->rows[y][x] == '5') {
                Entity e(EntityType::EXIT, x + 0.5f, y + 0.5f);
                e.shape = exitSprite;
                e.w = 26;
                e.h = 5;
                entities.push_back(e);
                return;
            }
        }
    }
}

void Game() {
    float fov = 60.0f;

    if (depthBuffer.size() != screenWidth)
    {
        depthBuffer.resize(screenWidth);
    }

    POINT mousePos;
    GetCursorPos(&mousePos);

    int deltaX = mousePos.x - lastMouse.x;

    float mouseSensitivity = 0.10f;
    angle += deltaX * mouseSensitivity;

    float playerRad = angle * 3.14159f / 180.0f;

    lastMouse = mousePos;

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    int border = 10;

    if (mousePos.x <= border) {
        SetCursorPos(screenW - border - 1, mousePos.y);
        lastMouse.x = screenW - border - 1;
    }
    else if (mousePos.x >= screenW - border) {
        SetCursorPos(border + 1, mousePos.y);
        lastMouse.x = border + 1;
    }

    system("color 3f");

    float speed = 0.25f;

    bool running = (GetAsyncKeyState(VK_SHIFT) & 0x8000);

    // player running fast
    if (running) {
        speed = speed * 2;
    }

    int numRays = screenWidth;

    float moveX = cos(playerRad);
    float moveY = sin(playerRad);

    // clear buffer
    for (int y = 0; y < screenHeight; y++)
        for (int x = 0; x < screenWidth; x++)
            screen[y][x] = ' ';

    int prevCellX = -1;
    int prevCellY = -1;
    int prevSide = -1;

    // raycasting 
    for (int i = 0; i < numRays; i++) {
        float rayAngle = (angle - fov / 2.0f) + ((float)i / numRays) * fov;
        float rad = rayAngle * 3.14159f / 180.0f;

        float dirX = cos(rad);
        float dirY = sin(rad);

        // DDA setup
        int currentCellX = (int)x;
        int currentCellY = (int)y;

        float deltaDistX = (dirX == 0) ? 1e30f : fabs(1.0f / dirX);
        float deltaDistY = (dirY == 0) ? 1e30f : fabs(1.0f / dirY);

        float sideDistX, sideDistY;
        int stepX, stepY;

        if (dirX < 0) {
            stepX = -1;
            sideDistX = (x - currentCellX) * deltaDistX;
        }
        else {
            stepX = 1;
            sideDistX = (currentCellX + 1.0f - x) * deltaDistX;
        }
        if (dirY < 0) {
            stepY = -1;
            sideDistY = (y - currentCellY) * deltaDistY;
        }
        else {
            stepY = 1;
            sideDistY = (currentCellY + 1.0f - y) * deltaDistY;
        }

        // DDA execution
        int side = 0; // 0 = vertical boundary, 1 = horizontal boundary
        while (GetMapCell(currentCellX, currentCellY) != '1') {
            if (sideDistX < sideDistY) {
                sideDistX += deltaDistX;
                currentCellX += stepX;
                side = 0;
            }
            else {
                sideDistY += deltaDistY;
                currentCellY += stepY;
                side = 1;
            }
        }

        bool seam = false;

        if (i > 0) {
            if (side != prevSide) {
                seam = true;
            }
        }

        // distance calculation
        float distance;
        if (side == 0) distance = (sideDistX - deltaDistX);
        else           distance = (sideDistY - deltaDistY);

        if (distance < 0.01f) distance = 0.01f;

        // fixed fish-eye correction
        float correctedDistance = distance * cos((rayAngle - angle) * 3.14159f / 180.0f);

        depthBuffer[i] = correctedDistance;

        int wallHeight = (int)(screenHeight / correctedDistance);

        int wallTop = (screenHeight - wallHeight) / 2;
        int wallBottom = wallTop + wallHeight;

        // precision edge detection
        float wallHitFraction;
        if (side == 0) wallHitFraction = y + distance * dirY;
        else           wallHitFraction = x + distance * dirX;
        wallHitFraction -= floor(wallHitFraction);

        bool boundary = false;
        float edgeThreshold = 0.03f; // sensitivity of the vertical corners

        if (wallHitFraction < edgeThreshold || wallHitFraction >(1.0f - edgeThreshold)) {
            if (side == 0) { // ray hit a vertical line (X-boundary)
                if (wallHitFraction < edgeThreshold) {
                    if (GetMapCell(currentCellX, currentCellY - 1) == ' ') {
                        boundary = true;
                    }
                }
                else if (wallHitFraction > (1.0f - edgeThreshold)) {
                    if (GetMapCell(currentCellX, currentCellY + 1) == ' ') {
                        boundary = true;
                    }
                }
            }
            else { // side == 1: Ray hit a horizontal line (Y-boundary)
                if (wallHitFraction < edgeThreshold) {
                    if (GetMapCell(currentCellX - 1, currentCellY) == ' ') {
                        boundary = true;
                    }
                }
                else if (wallHitFraction > (1.0f - edgeThreshold)) {
                    if (GetMapCell(currentCellX + 1, currentCellY) == ' ') {
                        boundary = true;
                    }
                }
            }
        }

        prevCellX = currentCellX;
        prevCellY = currentCellY;
        prevSide = side;

        // rendering hollow column
        for (int yCoord = wallTop; yCoord < wallBottom; yCoord++) {
            if (yCoord >= 0 && yCoord < screenHeight && i >= 0 && i < screenWidth) {
                // true vertical corner, or top outline, or bottom outline
                if (boundary || yCoord == wallTop || yCoord == wallBottom - 1) {
                    screen[yCoord][i] = char(219);
                }
                else if (seam) {
                    screen[yCoord][i] = char(219);
                }
                else {
                    screen[yCoord][i] = ' ';
                }
            }
        }
    }

    // Process enemy AI
    UpdateEnemies(0.04f);

    // Sort entities from farthest to nearest
    std::stable_sort(
        entities.begin(),
        entities.end(),
        [&](const Entity& a, const Entity& b)
        {
            float da =
                (a.x - x) * (a.x - x) +
                (a.y - y) * (a.y - y);

            float db =
                (b.x - x) * (b.x - x) +
                (b.y - y) * (b.y - y);

            return da > db;
        });

    // Render entities after moving them
    for (const auto& e : entities)
    {
        RenderSprite( e, x, y, playerRad, fov, screenWidth, screenHeight, depthBuffer, screen );
    }

    // player info
    std::string info =
    "X:" + std::to_string(x) +
    " Y:" + std::to_string(y) +
    " A:" + std::to_string(angle);
    
    std::string enemiesInfo = 
    "E:" + std::to_string(kills) +
    "/" + std::to_string(totalEnemies) +
    " H:" + std::to_string(health);

    std::string allEnemiesKilled = 
    "Congrats! You killed all enemies";
    
    int offset = (int)info.size() + 1;

    for (int i = 0; i < (int)info.size() && i < screenWidth; i++)
        screen[0][i] = info[i];

    for (int i = 0; i < (int)enemiesInfo.size() && i + offset < screenWidth; i++)
        screen[0][i + offset] = enemiesInfo[i];

    if (kills >= totalEnemies && currentMap == &map1_struct) {
        for (int i = 0; i < (int)allEnemiesKilled.size() && i + offset < screenWidth; i++)
            screen[0][i + offset] = allEnemiesKilled[i];
    }

    // movement
    float dx = 0.0f;
    float dy = 0.0f;

    if (GetAsyncKeyState('W') & 0x8000) {
        dx += moveX * speed;
        dy += moveY * speed;
    }
    if (GetAsyncKeyState('S') & 0x8000) {
        dx -= moveX * speed;
        dy -= moveY * speed;
    }
    if (GetAsyncKeyState('D') & 0x8000) {
        dx -= moveY * speed;
        dy += moveX * speed;
    }
    if (GetAsyncKeyState('A') & 0x8000) {
        dx += moveY * speed;
        dy -= moveX * speed;
    }

    float newX = x + dx;

    char xTile = GetMapCell((int)newX, (int)y);

    if (!PlayerCollidingWithEnemy(newX, y) && (xTile == ' ' || (xTile == '5' && kills >= totalEnemies))) {
        if (xTile == '5')
            gameComplete = true;

        x = newX;
    }

    float newY = y + dy;

    char yTile = GetMapCell((int)x, (int)newY);

    if (!PlayerCollidingWithEnemy(x, newY) && (yTile == ' ' || (yTile == '5' && kills >= totalEnemies))) {
        if (yTile == '5')
            gameComplete = true;

        y = newY;
    }

    if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
        if (shootTimer == 0) {
            Shoot();
            GunFrame = 2;
            shootTimer = 5;
        }
    }

    if (shootTimer > 0) {
        shootTimer--;

        if (shootTimer == 0) {
            GunFrame = 1;
        }
    }

    if (angle < 0.0f) angle += 360.0f;
    if (angle >= 360.0f) angle -= 360.0f;

    if (health <= 0) {
        gameFailed = true;
    }

    while (gameComplete) {
        kills = 0;
        system("color 0f");
        system("cls");
        std::cout << "Demo Completed";
        Sleep(5000);
        state = GameState::MENU;
        break;
    }

    while (gameFailed) {
        kills = 0;
        system("color 0f");
        system("cls");
        std::cout << "Game Over";
        Sleep(5000);
        state = GameState::MENU;
        break;
    }

    // render buffer
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    if (GunFrame == 1)
        DrawGunToBuffer(Sprite["GunSprite1"]);
    else if (GunFrame == 2)
        DrawGunToBuffer(Sprite["GunSprite2"]);

    for (int y = 0; y < screenHeight; y++) {
        DWORD written;
        COORD pos = { 0, (SHORT)y };

        WriteConsoleOutputCharacterA(
            hConsole,
            screen[y].data(),
            screenWidth,
            pos,
            &written
        );
    }

    Sleep(16);
}