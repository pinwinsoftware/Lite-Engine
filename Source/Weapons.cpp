#include "main.h"
#include "Game.h"
#include "Weapons.h"
#include "Entity.h"
#include <iostream>

std::map<std::string, SpriteGrid> Sprite;

Weapon currentWeapon = Weapon::GUN;

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

    SpriteGrid knifeSprite1;

    char KnifeSprite1[8][16] =
    {
        "            ",
        "  1         ",
        "  11        ",
        "  1111      ",
        "    1111    ",
        "      1111  ",
        "      111111",
        "    111111  ",
    };

    for (int i = 0; i < 8; i++) {
        knifeSprite1.push_back(KnifeSprite1[i]);
    }

    Sprite["KnifeSprite1"] = knifeSprite1;


    SpriteGrid knifeSprite2;

    char KnifeSprite2[8][16] =
    {
        "1           ",
        "11          ",
        "1111        ",
        "  1111      ",
        "    1111    ",
        "    111111  ",
        "  111111    ",
        "111111      ",
    };

    for (int i = 0; i < 8; i++) {
        knifeSprite2.push_back(KnifeSprite2[i]);
    }

    Sprite["KnifeSprite2"] = knifeSprite2;
}

void DrawGunToBuffer(const SpriteGrid& gun)
{
    if (gun.empty())
        return;

    int originalWidth = (int)gun[0].size();
    int originalHeight = (int)gun.size();

    int targetWidth = screenWidth / 9;

    float scale = (float)targetWidth / originalWidth;

    int targetHeight = (int)(originalHeight * scale);

    int startX = screenWidth / 2 - targetWidth / 2;

    int startY = screenHeight - targetHeight;

    for (int y = 0; y < targetHeight; y++) {
        for (int x = 0; x < targetWidth; x++) {
            int texX =
                x * originalWidth / targetWidth;

            int texY =
                y * originalHeight / targetHeight;

            if (gun[texY][texX] != '1')
                continue;

            int sx = startX + x;
            int sy = startY + y;

            if (sx >= 0 && sx < screenWidth && sy >= 0 && sy < screenHeight) {
                screen[sy][sx] = char(219);
            }
        }
    }
}

void Attack()
{
    if (currentWeapon == Weapon::GUN) {
        Shoot();
    }
    else {
        KnifeAttack();
    }
}

void Shoot() {
    float px = x;
    float py = y;

    float shootRad = angle * pi / 180.0f;
    float dx = cos(shootRad);
    float dy = sin(shootRad);

    float maxDistance = 999.0f;
    float distance = 0.0f;

    auto hitIt = entities.end();

    if (GetAsyncKeyState('1') & 1)
        currentWeapon = Weapon::KNIFE;

    if (GetAsyncKeyState('2') & 1 && ammo > 0)
        currentWeapon = Weapon::GUN;

    if (ammo <= 0)
        currentWeapon = Weapon::KNIFE;

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
            if (it->type == EntityType::ENEMY && fabs(it->x - px) < 0.25f && fabs(it->y - py) < 0.25f) {
                hitIt = it;
                break;
            }
        }

        if (hitIt != entities.end()) {
            break;
        }
    }

    if (hitIt != entities.end()) {
        int damage = RollDice(3, 7) + 7;

        hitIt->health -= damage;

        if (hitIt->health <= 0) {
            kills++;

            hitIt->type = EntityType::CORPSE;
            hitIt->shape = corpsesSprite;

            // Disable enemy behavior
            hitIt->state = EnemyState::IDLE;
            hitIt->velocityX = 0;
            hitIt->velocityY = 0;
        }
    }
}

void KnifeAttack() {
    const float knifeRange = 1.15f;

    float rad = angle * pi / 180.0f;
    float dirX = cos(rad);
    float dirY = sin(rad);

    for (auto& e : entities) {
        if (e.type != EntityType::ENEMY)
            continue;

        float dx = e.x - x;
        float dy = e.y - y;

        float dist = sqrtf(dx * dx + dy * dy);

        if (dist > knifeRange)
            continue;

        dx /= dist;
        dy /= dist;

        float dot = dx * dirX + dy * dirY;

        if (dot < 0.6f)
            continue;

        e.health -= RollDice(2, 6) + 3;

        if (e.health <= 0) {
            kills++;
            e.type = EntityType::CORPSE;
            e.shape = corpsesSprite;
            e.state = EnemyState::IDLE;
        }
        break;
    }
}