#pragma once
#include <vector>
#include <map>
#include <string>

enum class Weapon {
    KNIFE,
    GUN
};

extern Weapon currentWeapon;

using SpriteGrid = std::vector<std::string>;

extern std::map<std::string, SpriteGrid> Sprite;

void Attack();
void KnifeAttack();
void Shoot();
void LoadSprites();
void DrawGunToBuffer(const SpriteGrid& gun);