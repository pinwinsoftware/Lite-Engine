#include <iostream>
#include <windows.h>
#include <string>
#include <cstring>
#include <cmath>
#include <vector>
#include <algorithm>

#include "main.h"
#include "Entity.h"
#include "Weapons.h"
#include "Map.h"
#include "PauseMenu.h"

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

    if (tile == '2' || tile == '3' || tile == '4' || tile == '6' || tile == '7')
        return ' ';

    return tile;
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
        int rowLen = (int)currentMap->rows[y].size();
        for (int x = 0; x < rowLen; x++) {
            char tile = currentMap->rows[y][x];
            switch (tile) {
            case '2':
            {
                Entity e(EntityType::ENEMY, x + 0.5f, y + 0.5f);
                e.enemyClass = EnemyClass::MELEE;
                e.shape = bigMonsterSprite;
                e.w = 16;
                e.h = 16;
                e.speed = 3.f;
                e.health = 100;
                entities.push_back(e);
                break;
            }
            case '3':
            {
                Entity e(EntityType::ENEMY, x + 0.5f, y + 0.5f);
                e.enemyClass = EnemyClass::RANGED;
                e.shape = monsterSprite;
                e.w = 16;
                e.h = 16;
                e.speed = 1.5f;
                e.health = 50;
                entities.push_back(e);
                break;
            }
            case '4':
            {
                Entity e(EntityType::ENEMY, x + 0.5f, y + 0.5f);
                e.enemyClass = EnemyClass::MELEE;
                e.shape = trollSprite;
                e.w = 16;
                e.h = 13;
                e.speed = 3.f;
                e.health = 500;
                entities.push_back(e);
                break;
            }
            case '5':
            {
                Entity e(EntityType::EXIT, x + 0.5f, y + 0.5f);
                e.shape = exitSprite;
                e.w = 26;
                e.h = 5;
                entities.push_back(e);
                break;
            }
            case '6':
            {
                Entity e(EntityType::AMMO, x + 0.5f, y + 0.5f);
                e.shape = ammoSprite;
                e.w = 16;
                e.h = 16;
                entities.push_back(e);
                break;
            }

            case '7':
            {
                Entity e(EntityType::MEDKIT, x + 0.5f, y + 0.5f);
                e.shape = medKitSprite;
                e.w = 16;
                e.h = 16;
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

void CollectItem(Entity& e)
{
    bool itemWasPickedUp = false;

    switch (e.type)
    {
    case EntityType::COLLECTIBLE:
        coins++;
        itemWasPickedUp = true;
        break;

    case EntityType::AMMO:
        // Only pick up if player actually needs ammo
        if (ammo < 100) {
            ammo += 20;
            if (ammo > 100) ammo = 100;
            itemWasPickedUp = true;
        }
        break;

    case EntityType::MEDKIT:
        // Only pick up if player actually needs health
        if (health < 100) {
            health += 25;
            if (health > 100) health = 100;
            itemWasPickedUp = true;
        }
        break;

    default:
        return;
    }

    // If the item wasn't actually consumed, leave it on the map
    if (!itemWasPickedUp) {
        return;
    }

    for (auto it = entities.begin(); it != entities.end(); ++it) {
        if (it->x == e.x && it->y == e.y) {
            entities.erase(it);
            break;
        }
    }
}

void Game() {
    if (!gamePaused)
        system("color 3f");
    else
        system("color 0F");

    totalEnemies = countEnemies(currentMap->rows);
    EnableMouse();

    float fov = 60.0f;

    if (depthBuffer.size() != screenWidth)
    {
        depthBuffer.resize(screenWidth);
    }

    // mouse lock
    HWND gameWindow = GetForegroundWindow();

    if (gameWindow != NULL) {
        // Get the actual drawable area of the active console window
        RECT clientRect;
        GetClientRect(gameWindow, &clientRect);

        int clientWidth =
            clientRect.right - clientRect.left;

        int clientHeight =
            clientRect.bottom - clientRect.top;

        const int gameWidth = screenWidth;
        const int gameHeight = screenHeight;

        // Convert game character coordinates to pixels
        float pixelsPerColumn = (float)clientWidth / gameWidth;

        float pixelsPerRow = (float)clientHeight / gameHeight;

        int centerX = (int)((gameWidth / 2.0f) * pixelsPerColumn);
        int centerY = (int)((gameHeight / 2.0f) * pixelsPerRow);

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

        prevCellX = currentCellX;
        prevCellY = currentCellY;
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

    if (kills >= totalEnemies && currentMap == &map1_struct) {
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

        float newX = x + dx;

        char xTile = GetMapCell((int)newX, (int)y);

        if (!PlayerCollidingWithEnemy(newX, y) && (xTile == ' ' || (xTile == '5'))) {
            if (xTile == '5')
                levelComplete = true;

            x = newX;
        }

        float newY = y + dy;

        char yTile = GetMapCell((int)x, (int)newY);

        if (!PlayerCollidingWithEnemy(x, newY) && (yTile == ' ' || (yTile == '5'))) {
            if (yTile == '5')
                levelComplete = true;

            y = newY;
        }

        if (GetAsyncKeyState(VK_TAB) & 0x8000) {
            DrawMap();
        }

        // Weapon switching
        if (GetAsyncKeyState('1') & 1) {
            currentWeapon = Weapon::KNIFE;
        }

        if (GetAsyncKeyState('2') & 1) {
            if (ammo > 0)
                currentWeapon = Weapon::GUN;
        }

        if (ammo <= 0) {
            currentWeapon = Weapon::KNIFE;
        }


        // Attack
        if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) || (GetAsyncKeyState(VK_SPACE) & 0x8000)) {
            if (shootTimer == 0) {
                if (currentWeapon == Weapon::GUN) {
                    if (ammo > 0) {
                        Shoot();

                        ammo--;

                        GunFrame = 2;
                        gunFrameTimer = 7;
                    }
                    else {
                        currentWeapon = Weapon::KNIFE;
                    }
                }
                else {
                    KnifeAttack();

                    GunFrame = 2;
                    gunFrameTimer = 7;
                }

                shootTimer = 15;
            }
        }

        // cooldown
        if (shootTimer > 0) {
            shootTimer--;
        }


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

            if (dx * dx + dy * dy < pickupRange * pickupRange)
            {
                bool canPickUp = false;

                switch (it->type)
                {
                case EntityType::COLLECTIBLE:
                    // Coins can always be collected
                    coins++;
                    canPickUp = true;
                    break;

                case EntityType::AMMO:
                    // Only collect ammo when ammo is below 100
                    if (ammo < 100)
                    {
                        ammo += 10;

                        if (ammo > 100)
                            ammo = 100;

                        canPickUp = true;
                    }
                    break;

                case EntityType::MEDKIT:
                    // Only collect a medkit when health is below 100
                    if (health < 100)
                    {
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
                if (canPickUp)
                {
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
        if (currentMap == &map1_struct) {
            x = 15.f;
            y = 3.5f;
            angle = 180.f;
            currentMap = &map2_struct;
            kills = 0;
            LoadEntities();
            levelComplete = false;
        }
        else if (currentMap == &map2_struct) {
            x = 2.f;
            y = 2.5f;
            angle = 0.f;
            currentMap = &map3_struct;
            kills = 0;
            LoadEntities();
            levelComplete = false;
        }
        else if (currentMap == &map3_struct) {
            x = 9.5f;
            y = 1.f;
            angle = 90.f;
            currentMap = &map4_struct;
            kills = 0;
            LoadEntities();
            levelComplete = false;
        }
        else if (currentMap == &map4_struct) {
            x = 11.5f;
            y = 2.f;
            angle = 90.f;
            currentMap = &map5_struct;
            kills = 0;
            LoadEntities();
            levelComplete = false;
        }
        else if (currentMap == &map5_struct) {
            gameComplete = true;
            levelComplete = false;
        }

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

    if (currentWeapon == Weapon::GUN)
    {
        if (GunFrame == 1)
            DrawGunToBuffer(Sprite["GunSprite1"]);
        else
            DrawGunToBuffer(Sprite["GunSprite2"]);
    }
    else
    {
        if (GunFrame == 1)
            DrawGunToBuffer(Sprite["KnifeSprite1"]);
        else
            DrawGunToBuffer(Sprite["KnifeSprite2"]);
    }

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