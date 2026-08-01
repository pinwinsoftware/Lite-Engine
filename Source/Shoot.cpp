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
    float px = x;
    float py = y;

    float shootRad = angle * 3.14159f / 180.0f;
    float dx = cos(shootRad);
    float dy = sin(shootRad);

    float maxDistance = 999.0f;
    float distance = 0.0f;

    auto hitIt = entities.end();

    while (distance < maxDistance) {
        px += dx * 0.1f;
        py += dy * 0.1f;
        distance += 0.1f;

        int tileX = (int)px;
        int tileY = (int)py;

        // Stop when the bullet hits a wall.
        if (GetMapCell(tileX, tileY) == '1') {
            break;
        }

        // Find the first enemy hit by the ray.
        for (auto it = entities.begin(); it != entities.end(); ++it) {
            if (it->type == EntityType::ENEMY && fabs(it->x - px) < 0.5f && fabs(it->y - py) < 0.5f) {
                hitIt = it;
                break;
            }
        }

        if (hitIt != entities.end()) {
            break;
        }
    }

    if (hitIt != entities.end()) {
        kills++;

        // Remove only the exact enemy that was hit.
        entities.erase(hitIt);

        if (kills >= totalEnemies) {
            SpawnExit();
        }
    }
}