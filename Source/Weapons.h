#pragma once
#include <string>
#include <vector>

#include "WeaponsDefinitions.h"

using SpriteGrid = std::vector<std::string>;

extern int currentWeaponId; // The currently selected weapon is identified by its LES Id

extern bool SelectWeapon(int id);

const WeaponDefinition* GetCurrentWeapon();

void Attack();
void Shoot(const WeaponDefinition& weapon);
void KnifeAttack(const WeaponDefinition& weapon);
void DrawCurrentWeapon();