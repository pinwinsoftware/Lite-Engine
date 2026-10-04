#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <cctype>

#include "main.h"
#include "Game.h"
#include "Weapons.h"
#include "Entity.h"
#include "WeaponsDefinitions.h"

static int currentWeaponId = 1;

const WeaponDefinition* GetCurrentWeapon() {
    return FindWeaponDefinitionById(currentWeaponId);
}

bool SelectWeapon(int id) {
    const WeaponDefinition* weapon = FindWeaponDefinitionById(id);

    if (!weapon)
        return false;

    currentWeaponId = id;

    return true;
}

static const WeaponAnimationState*
FindWeaponAnimation(const WeaponDefinition& weapon, const std::string& state) {
    std::string wanted = state;

    std::transform(wanted.begin(), wanted.end(), wanted.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        }
    );

    for (const WeaponAnimationState& animation : weapon.animations) {
        std::string animationState = animation.state;

        std::transform(animationState.begin(), animationState.end(), animationState.begin(),
            [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            }
        );

        if (animationState == wanted)
            return &animation;
    }
    return nullptr;
}

// Draw loaded weapon sprite
static void DrawWeaponToBuffer(const WeaponAnimationState& animation) {

    if (animation.pixels.empty())
        return;

    int originalHeight = static_cast<int>(animation.pixels.size());

    int originalWidth = 0;

    for (const std::string& line : animation.pixels) {
        if (static_cast<int>(line.size()) > originalWidth)
            originalWidth = static_cast<int>(line.size());
    }

    if (originalWidth <= 0 || originalHeight <= 0) {
        return;
    }

    int targetWidth = screenWidth / 9;

    float scale = static_cast<float>(targetWidth) / static_cast<float>(originalWidth);

    int targetHeight = static_cast<int>(originalHeight * scale);

    if (targetHeight <= 0)
        return;

    int startX = screenWidth / 2 - targetWidth / 2;
    int startY = screenHeight - targetHeight;

    for (int y = 0; y < targetHeight; ++y) {
        for (int x = 0; x < targetWidth; ++x) {

            int texX = x * originalWidth / targetWidth;
            int texY = y * originalHeight / targetHeight;

            if (texY < 0 || texY >= originalHeight) {
                continue;
            }

            if (texX < 0 || texX >= static_cast<int>(animation.pixels[texY].size())) {
                continue;
            }

            char pixel = animation.pixels[texY][texX];

            int sx = startX + x;
            int sy = startY + y;

            if (sx >= 0 && sx < screenWidth && sy >= 0 && sy < screenHeight) {
                if (pixel == '1') {
                    screen[sy][sx] = char(219);
                }
                else if (pixel == '0') {
                    screen[sy][sx] = ' ';
                }
            }
        }
    }
}

void DrawCurrentWeapon() {

    const WeaponDefinition* weapon = GetCurrentWeapon();

    if (!weapon)
        return;

    const WeaponAnimationState* animation =
        FindWeaponAnimation(
            *weapon,
            GunFrame == 2
            ? "ATTACK"
            : "IDLE");

    if (!animation) {
        animation = FindWeaponAnimation(*weapon, "IDLE");
    }

    if (!animation) {
        return;
    }

    DrawWeaponToBuffer(*animation);
}

// Damage
static int RollWeaponDamage(const WeaponDefinition& weapon) {
    std::string expression = weapon.damage;

    // Remove whitespace.
    expression.erase(std::remove_if(expression.begin(), expression.end(), [](unsigned char c) {
                return std::isspace(c);
            }
        ),
        expression.end()
    );

    size_t dPos = expression.find('d');

    // Plain number, e.g. "10"
    if (dPos == std::string::npos) {
        try {
            return std::stoi(expression);
        }
        catch (...) {
            return 0;
        }
    }

    int dice = 0;
    int sides = 0;
    int bonus = 0;

    try {
        
        dice = std::stoi(expression.substr(0, dPos)); // Everything before 'd'. Example: 2d6+3 -> 2
        
        size_t plus = expression.find('+', dPos + 1);
        size_t minus = expression.find('-', dPos + 1);

        size_t modifier = std::string::npos;

        if (plus != std::string::npos) {
            modifier = plus;
        }

        if (minus != std::string::npos && (modifier == std::string::npos || minus < modifier)) {
            modifier = minus;
        }

        // Get the dice sides.

        if (modifier == std::string::npos) {
            sides = std::stoi(expression.substr(dPos + 1));
        }
        else {
            sides = std::stoi(expression.substr(dPos + 1, modifier - dPos - 1));
            bonus = std::stoi(expression.substr(modifier)); // Get the bonus.
        }
    }
    catch (...) {
        return 0;
    }

    if (dice <= 0 || sides <= 0)
        return 0;

    int damage = bonus;

    for (int i = 0; i < dice; ++i) {
        damage += 1 + (std::rand() % sides);
    }

    return damage;
}

void Attack() {

    const WeaponDefinition* weapon = GetCurrentWeapon();

    if (!weapon)
        return;

    if (weapon->ranged)
        Shoot(*weapon);
    else
        KnifeAttack(*weapon);
}


// Ranged attack

void Shoot(const WeaponDefinition& weapon) {
    float px = x;
    float py = y;

    float shootRad = angle * pi / 180.0f;

    float dx = cos(shootRad);
    float dy = sin(shootRad);

    float maxDistance = 999.0f;
    float distance = 0.0f;

    auto hitIt = entities.end();

    while (distance < maxDistance) {
        px += dx * 0.1f;
        py += dy * 0.1f;

        distance += 0.1f;

        int tileX = static_cast<int>(px);

        int tileY = static_cast<int>(py);

        if (GetMapCell(tileX, tileY) == '1')
            break;

        for (auto it = entities.begin(); it != entities.end(); ++it) {
            if (it->type != EntityType::ENEMY) {
                continue;
            }

            if (fabs(it->x - px) < 0.25f && fabs(it->y - py) < 0.25f) {
                hitIt = it;
                break;
            }
        }

        if (hitIt != entities.end())
            break;
    }

    if (hitIt == entities.end())
        return;

    int damage = RollWeaponDamage(weapon);

    hitIt->health -= damage;

    if (hitIt->health <= 0) {

        kills++;

        SetEnemyState(*hitIt, EnemyState::DEATH);

        hitIt->type = EntityType::CORPSE;

        hitIt->velocityX = 0;
        hitIt->velocityY = 0;
    }
}

// Melee attack
void KnifeAttack(const WeaponDefinition& weapon) {
    
    const float knifeRange = 1.15f;

    float rad = angle * pi / 180.0f;

    float dirX = cos(rad);
    float dirY = sin(rad);

    for (auto& e : entities) {
        if (e.type != EntityType::ENEMY) {
            continue;
        }

        float dx = e.x - x;
        float dy = e.y - y;

        float dist = sqrtf(dx * dx + dy * dy);

        if (dist <= 0.0001f)
            continue;

        if (dist > knifeRange)
            continue;

        dx /= dist;
        dy /= dist;

        float dot = dx * dirX + dy * dirY;

        if (dot < 0.6f)
            continue;

        e.health -= RollWeaponDamage(weapon);

        if (e.health <= 0) {
            kills++;

            SetEnemyState(e, EnemyState::DEATH);

            e.type = EntityType::CORPSE;

            e.velocityX = 0;
            e.velocityY = 0;
        }
        break;
    }
}