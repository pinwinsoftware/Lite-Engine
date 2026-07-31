#pragma once

#include <vector>
#include <map>
#include <string>

using SpriteGrid = std::vector<std::string>;

extern std::map<std::string, SpriteGrid> Sprite;

void Shoot();
void LoadSprites();
void DrawGunToBuffer(const SpriteGrid& gun);