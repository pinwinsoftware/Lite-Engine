#include <iostream>
#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>

#include "main.h"
#include "Entity.h"
#include "Weapons.h"
#include "Map.h"
#include "PauseMenu.h"
#include "EntityDefinitions.h"
#include "LevelDefinitions.h"
#include "WeaponsDefinitions.h"
#include "MapLoader.h"

std::vector<float> depthBuffer;

int GunFrame = 1;
int shootTimer = 0;
int gunFrameTimer = 0;

int coins = 0;
int totalCoins = 0;

int kills = 0;
int totalEnemies = 0;

bool gameComplete = false;
bool levelComplete = false;
bool gameFailed = false;

const float playerRadius = 0.35f;
const float enemyRadius = 0.35f;

// Count all enemies on map.
int countEnemies(const std::vector<std::string>& mapLayout) {
    int enemyCount = 0;

    for (const std::string& row : mapLayout) {
        for (char cell : row) {
            if (cell == '2' || cell == '3' || cell == '4') {
                enemyCount++;
            }
        }
    }

    return enemyCount;
}

bool PlayerCollidingWithEnemy(float playerX, float playerY) {
    for (const Entity& e : entities) {

        if (e.type != EntityType::ENEMY)
            continue;

        float dx = playerX - e.x;
        float dy = playerY - e.y;

        float minimumDistance = playerRadius + enemyRadius;

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

    if (tile >= '2' && tile <= '9')
        return ' ';

    return tile;
}

bool DefineExit(float playerX, float playerY) {
    for (const Entity& e : entities) {
        if (e.type != EntityType::EXIT)
            continue;

        float dx = playerX - e.x;
        float dy = playerY - e.y;

        const float exitRadius = 0.5f;

        if (dx * dx + dy * dy < exitRadius * exitRadius) {
            return true;
        }
    }

    return false;
}

int RollDice(int amount, int sides) {
    int total = 0;

    for (int i = 0; i < amount; i++) {
        total += (rand() % sides) + 1;
    }

    return total;
}

void getMapEntities() {
    for (int y = 0; y < currentMap->height; y++) {
        int rowLen = static_cast<int>(currentMap->rows[y].size());

        for (int x = 0; x < rowLen; x++) {
            char tile = currentMap->rows[y][x];

            // detect enemies entities by id
            if (tile >= '2' && tile <= '9') {
                int enemyId = tile - '0';

                const EntityDefinition* definition =
                    FindEntityDefinition(enemyId);

                if (!definition) {
                    continue;
                }

                Entity e(definition->type, x + 0.5f, y + 0.5f);

                e.id = enemyId;
                e.sprite = definition->idle;

                e.w = definition->width;
                e.h = definition->height;

                e.speed = definition->speed;
                e.health = definition->health;

                if (definition->ranged)
                    e.enemyClass = EnemyClass::RANGED;
                else
                    e.enemyClass = EnemyClass::MELEE;

                entities.push_back(e);
            }
        }
    }
}

void LoadEntities() {
    entities.clear();
    getMapEntities();
}

bool LoadLevel(const std::string& mapName) {
    const LevelDefinition* definition =
        FindLevelDefinition(mapName);

    if (!definition) {
        return false;
    }

    // Player starting data

    x = definition->playerX;
    y = definition->playerY;
    angle = definition->playerAngle;

    ammo = definition->playerAmmo;
    health = definition->playerHealth;

    currentLevelName = definition->mapName;

    if (!LoadMapFromLED("LED/GAME.LED",definition->mapFile)) {
        return false;
    }

    LoadEntities();

    return true;
}

void Game() {
    if (!gamePaused)
        system("color 3f");
    else
        system("color 0F");

    totalEnemies = countEnemies(currentMap->rows);
    EnableMouse();

    const float fov = 60.0f;

    if (depthBuffer.size() != screenWidth) {
        depthBuffer.resize(screenWidth);
    }

    // mouse lock
    HWND gameWindow = GetForegroundWindow();

    if (gameWindow != nullptr) {
        // Get the actual drawable area of the active console window
        RECT clientRect;
        GetClientRect(gameWindow, &clientRect);

        int clientWidth = clientRect.right - clientRect.left;
        int clientHeight = clientRect.bottom - clientRect.top;

        // Convert game character coordinates to pixels
        float pixelsPerColumn = static_cast<float>(clientWidth) / screenWidth;
        float pixelsPerRow = static_cast<float>(clientHeight) / screenHeight;

        int centerX = (int)((screenWidth / 2.0f) * pixelsPerColumn);
        int centerY = (int)((screenHeight / 2.0f) * pixelsPerRow);

        // Convert client coordinates to screen coordinates
        POINT center = { centerX, centerY };

        ClientToScreen(gameWindow, &center);

        // Read mouse position
        if (!gamePaused) {
            POINT mousePos;
            GetCursorPos(&mousePos);

            // Horizontal mouse movement
            int deltaX = mousePos.x - center.x;

            const float mouseSensitivity = 0.10f;

            angle += deltaX * mouseSensitivity;

            // Lock the cursor to the center
            SetCursorPos(center.x, center.y);
        }
    }
    float playerRad = angle * pi / 180.0f;

    float speed = 0.20f;

    bool running = (GetAsyncKeyState(VK_SHIFT) & 0x8000);

    // player running fast
    if (running) {
        speed = speed * 2;
    }

    float moveX = cos(playerRad);
    float moveY = sin(playerRad);

    // clear buffer
    for (int y = 0; y < screenHeight; y++)
        for (int x = 0; x < screenWidth; x++)
            screen[y][x] = ' ';

    int prevSide = -1;

    // raycasting 
    for (int i = 0; i < screenWidth; i++) {
        float rayAngle = (angle - fov / 2.0f) + ((float)i / screenWidth) * fov;
        float rad = rayAngle * pi / 180.0f;

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
        float correctedDistance = distance * cos((rayAngle - angle) * pi / 180.0f);

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

        prevSide = side;

        // rendering hollow column
        for (int yCoord = wallTop; yCoord < wallBottom; yCoord++) {
            if (yCoord >= 0 && yCoord < screenHeight && i >= 0 && i < screenWidth) {
                // vertical corner, top outline and bottom outline
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

    // Sort entities from farthest to nearest
    std::stable_sort(entities.begin(), entities.end(), [&](const Entity& a, const Entity& b) {
            float da = (a.x - x) * (a.x - x) + (a.y - y) * (a.y - y);
            float db = (b.x - x) * (b.x - x) + (b.y - y) * (b.y - y);
            return da > db;
        });

    // Render entities after moving them
    for (const auto& e : entities) {
        RenderSprite(e, x, y, playerRad, fov, screenWidth, screenHeight, depthBuffer, screen);
    }

    UpdatePauseMenu();

    // Process enemy AI
    if (!gamePaused) {
        UpdateEnemies(0.04f);
        UpdateFireballs(0.04f);
    }

    if (gamePaused) {
        DrawPauseMenu();
    }

    // player info
    std::string info =
    "X:" + std::to_string(x) +
    " Y:" + std::to_string(y) +
    " A:" + std::to_string(angle);
    
    std::string enemiesInfo = 
    "E:" + std::to_string(kills) +
    "/" + std::to_string(totalEnemies) +
    " H:" + std::to_string(health) +
    " B:" + std::to_string(ammo);

    std::string allEnemiesKilled = 
    "Congrats! You killed all enemies";
    
    int offset = (int)info.size() + 1;

    for (int i = 0; i < (int)info.size() && i < screenWidth; i++)
        screen[0][i] = info[i];

    for (int i = 0; i < (int)enemiesInfo.size() && i + offset < screenWidth; i++)
        screen[0][i + offset] = enemiesInfo[i];

    if (kills >= totalEnemies) {
        for (int i = 0; i < (int)allEnemiesKilled.size() && i + offset < screenWidth; i++)
            screen[0][i + offset] = allEnemiesKilled[i];
    }

    if (!gamePaused) {
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

        // Player X movement

        float newX = x + dx;

        char xTile = GetMapCell((int)newX, (int)y);

        if (!PlayerCollidingWithEnemy(newX, y) && xTile == ' ') {
            x = newX;
        }

        // Player Y movement

        float newY = y + dy;

        char yTile = GetMapCell((int)x, (int)newY);

        if (!PlayerCollidingWithEnemy(x, newY) && yTile == ' ') {
            y = newY;
        }

        if (DefineExit(x, y)) {
            levelComplete = true;
        }

        if (GetAsyncKeyState(VK_TAB) & 0x8000) {
            DrawMap();
        }

        // Weapon selection
        for (int id = 1; id <= 9; ++id) {
            if (GetAsyncKeyState('0' + id) & 1) {
                SelectWeapon(id);
            }
        }

        // Current weapon
        const WeaponDefinition* weapon =
            GetCurrentWeapon();

        if (weapon && weapon->ranged && ammo <= 0) {
            // Find the first non-ranged weapon.
            for (int id = 1; id <= 9; ++id) {
                const WeaponDefinition* fallback = FindWeaponDefinitionById(id);

                if (fallback && !fallback->ranged) {
                    SelectWeapon(id);
                    weapon = GetCurrentWeapon();
                    break;
                }
            }
        }

        // Attack
        if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) || (GetAsyncKeyState(VK_SPACE) & 0x8000)) {
            if (shootTimer <= 0 && weapon != nullptr) {
                
                // Ranged weapons require ammo.
                if (!weapon->ranged || ammo > 0) {
                    Attack();

                    if (weapon->ranged) {
                        ammo--;
                    }

                    // Weapon animation.
                    GunFrame = 2;
                    gunFrameTimer = 7;

                    // Speed is the weapon cooldown in seconds.
                    // Convert seconds -> frames.
                    shootTimer = (int)(weapon->speed * 1000.0f / 16.0f);

                    if (shootTimer < 1)
                        shootTimer = 1;
                }
            }
        }

        // Weapon cooldown
        if (shootTimer > 0) {
            shootTimer--;
        }

        // Weapon animation
        if (gunFrameTimer > 0) {
            gunFrameTimer--;

            if (gunFrameTimer == 0) {
                GunFrame = 1;
            }
        }
    }

    for (auto it = entities.begin(); it != entities.end(); ) {
        if (it->type == EntityType::COLLECTIBLE || it->type == EntityType::AMMO || it->type == EntityType::MEDKIT) {
            float dx = x - it->x;
            float dy = y - it->y;

            const float pickupRange = 0.5f;

            if (dx * dx + dy * dy < pickupRange * pickupRange) {
                bool canPickUp = false;

                switch (it->type) {
                case EntityType::COLLECTIBLE:
                    // Coins can always be collected
                    coins++;
                    canPickUp = true;
                    break;

                case EntityType::AMMO:
                    // Only collect ammo when ammo is below 100
                    if (ammo < 100) {
                        ammo += 10;

                        if (ammo > 100)
                            ammo = 100;

                        canPickUp = true;
                    }
                    break;

                case EntityType::MEDKIT:
                    // Only collect a medkit when health is below 100
                    if (health < 100) {
                        health += 20;

                        if (health > 100)
                            health = 100;

                        canPickUp = true;
                    }
                    break;

                default:
                    break;
                }

                // Remove the item only if it was actually collected
                if (canPickUp) {
                    it = entities.erase(it);
                    continue;
                }
            }
        }
        ++it;
    }

    if (angle < 0.0f) angle += 360.0f;
    if (angle >= 360.0f) angle -= 360.0f;

    if (health <= 0) {
        gameFailed = true;
    }

    while (levelComplete) {
        const LevelDefinition* currentLevel =
            FindLevelDefinition(currentLevelName);

        if (!currentLevel) {
            gameComplete = true;
            levelComplete = false;
            break;
        }

        // No Next = this is the final level
        if (currentLevel->nextMap.empty()) {
            gameComplete = true;
            levelComplete = false;
            break;
        }

        // Load the next level from GAME.LEM
        if (!LoadLevel(currentLevel->nextMap)) {
            gameComplete = true;
            levelComplete = false;
            break;
        }

        kills = 0;
        levelComplete = false;

        break;
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

    DrawCurrentWeapon();

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