#pragma once
#include <string>
#include <vector>

struct WeaponAnimationState {
    std::string state;
    std::string file;
    std::vector<std::string> pixels;
};

struct WeaponDefinition {
    std::string name;

    int id = -1;
    int width = 0;
    int height = 0;

    std::string damage;

    float speed = 0.0f;
    bool ranged = false;

    std::vector<WeaponAnimationState> animations;
};

bool LoadWeaponDefinitions(const std::string& ledPath);

const WeaponDefinition* FindWeaponDefinition(const std::string& weaponName);
const WeaponDefinition* FindWeaponDefinitionById(int id);