#include "main.h"
#include "Game.h"
#include "Shoot.h"
#include "Entity.h"
#include <iostream>

std::map<std::string, SpriteGrid> Sprite;

void LoadSprites() {
    SpriteGrid gunSprite1;

    char GunSprite1[8][16] = {
        "    11      ",
        "    11      ",
        "  111111    ",
        "  111111    ",
        "    11      ",
        "1111111111  ",
        "  11111111  ",
        "    11111111",
    };

    for (int i = 0; i < 8; i++) {
        gunSprite1.push_back(GunSprite1[i]);
    }

    Sprite["GunSprite1"] = gunSprite1;

    SpriteGrid gunSprite2;

    char GunSprite2[8][16] = {
        "    11      ",
        "  111111    ",
        "    11      ",
        "  111111    ",
        "  111111    ",
        "    11      ",
        "1111111111  ",
        "  11111111  ",
    };

    for (int i = 0; i < 8; i++) {
        gunSprite2.push_back(GunSprite2[i]);
    }

    Sprite["GunSprite2"] = gunSprite2;
}

void DrawGunToBuffer(const SpriteGrid& gun)
{
    if (gun.empty())
        return;

    int originalWidth = (int)gun[0].size();
    int originalHeight = (int)gun.size();

    int targetWidth = screenWidth / 9;

    float scale =
        (float)targetWidth / originalWidth;

    int targetHeight =
        (int)(originalHeight * scale);

    int startX =
        screenWidth / 2 - targetWidth / 2;

    int startY =
        screenHeight - targetHeight;

    for (int y = 0; y < targetHeight; y++)
    {
        for (int x = 0; x < targetWidth; x++)
        {
            int texX =
                x * originalWidth / targetWidth;

            int texY =
                y * originalHeight / targetHeight;

            if (gun[texY][texX] != '1')
                continue;

            int sx = startX + x;
            int sy = startY + y;

            if (sx >= 0 &&
                sx < screenWidth &&
                sy >= 0 &&
                sy < screenHeight)
            {
                screen[sy][sx] = char(219);
            }
        }
    }
}

void Shoot() {
    // player position
    float px = x;
    float py = y;

    // shoot direction (angle in degrees)
    float shootRad = angle * 3.14159f / 180.0f;
    float dx = cos(shootRad);
    float dy = sin(shootRad);

    // max shoot distance
    float maxDistance = 999.0f;

    float distance = 0.0f;
    bool hit = false;
    Entity* hitEntity = nullptr;

    while (distance < maxDistance && !hit) {
        px += dx * 0.1f;
        py += dy * 0.1f;
        distance += 0.1f;

        int tileX = (int)px;
        int tileY = (int)py;

        // check wall collision
        if (getMapCell(tileX, tileY) == '1') {
            // hit a wall stop the ray
            break;
        }

        // check entity collision
        for (auto& e : entities) {
            if (e.type == EntityType::ENEMY &&
                fabs(e.x - px) < 0.5f && fabs(e.y - py) < 0.5f) {
                hitEntity = &e;
                hit = true;
                break;
            }
        }
    }

    if (hitEntity) {
        kills++;

        // remove the enemy
        int ex = (int)hitEntity->x;
        int ey = (int)hitEntity->y;
        currentMap->rows[ey][ex] = ' ';
        entities.erase(std::remove_if(entities.begin(), entities.end(),
            [ex, ey](const Entity& e) {
                return e.type == EntityType::ENEMY &&
                    (int)e.x == ex && (int)e.y == ey;
            }),
            entities.end());
    }
}