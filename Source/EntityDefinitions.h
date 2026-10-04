#pragma once
#include <string>
#include <vector>
#include "EntityType.h"

struct EntitySprite {
    int width = 0;
    int height = 0;

    std::vector<std::string> pixels;
};

struct RangedAttackDefinition {
    int width = 16;
    int height = 16;
    float speed = 5.0f;

    float range = 12.0f;
    float cooldown = 2.0f;

    int damageDice = 0;
    int damageSides = 0;
    int damageBonus = 0;

    std::string sprite;
    EntitySprite spriteData;
};

struct EntityDefinition {
    int id = -1;
    int width = 16;
    int height = 16;

    float health = 100.0f;

    int damageDice = 0;
    int damageSides = 0;
    int damageBonus = 0;

    float speed = 3.0f;

    bool ranged = false;

    EntityType type = EntityType::ENEMY;

    std::string idleSprite;
    std::string chaseSprite;
    std::string attackSprite;
    std::string deathSprite;

    EntitySprite idle;
    EntitySprite chase;
    EntitySprite attack;
    EntitySprite death;

    RangedAttackDefinition rangedAttack;
};

const EntityDefinition* FindEntityDefinition(int id);
const EntityDefinition* FindEntityDefinitionByType(EntityType type);

void LoadEntityDefinitions(const std::string& ledPath);