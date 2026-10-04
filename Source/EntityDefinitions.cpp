#include <sstream>
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>
#include <stdexcept>

#include "EntityDefinitions.h"
#include "LedReader.h"
#include "Entity.h"

static std::vector<EntityDefinition> entityDefinitions;

static std::string Trim(const std::string& value) {
    size_t start = value.find_first_not_of(" \t\r\n");

    if (start == std::string::npos)
        return "";

    size_t end = value.find_last_not_of(" \t\r\n");

    return value.substr(start, end - start + 1);
}

static std::string RemoveComment(const std::string& value) {
    size_t commentPosition = value.find("//");

    if (commentPosition == std::string::npos)
        return value;

    return value.substr(0, commentPosition);
}

static std::string RemoveSemicolon(const std::string& value) {
    std::string result = Trim(value);

    if (!result.empty() && result.back() == ';')
        result.pop_back();

    return Trim(result);
}

static std::string ToLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        }
    );

    return value;
}

// Parse dice expressions such as 1d2 or 1d2+3.
static bool ParseDiceExpression(const std::string& expression, int& dice, int& sides, int& bonus) {
    size_t dPosition = expression.find('d');

    if (dPosition == std::string::npos)
        return false;

    try {
        dice = std::stoi(expression.substr(0, dPosition));

        size_t plusPosition = expression.find('+', dPosition);

        if (plusPosition != std::string::npos) {
            sides = std::stoi(expression.substr(dPosition + 1, plusPosition - dPosition - 1));

            bonus = std::stoi(expression.substr(plusPosition + 1));
        }
        else {
            sides = std::stoi(expression.substr(dPosition + 1));

            bonus = 0;
        }
    }
    catch (const std::exception&) {
        return false;
    }

    return dice > 0 && sides > 0;
}

// Find a property in an LES script.
static bool GetProperty(const std::string& text, const std::string& property, std::string& value) {
    std::istringstream stream(text);
    std::string line;

    while (std::getline(stream, line)) {
        line = RemoveComment(line);
        line = Trim(line);

        if (line.empty())
            continue;

        // Find '='
        size_t equalPosition = line.find('=');

        if (equalPosition == std::string::npos)
            continue;

        std::string left = Trim(line.substr(0, equalPosition));

        // Exact property name.
        if (left != property)
            continue;

        value = RemoveSemicolon(line.substr(equalPosition + 1));

        return true;
    }
    return false;
}

// Load sprite data from the LED archive.
static bool LoadEntitySprite(LedReader& reader, const std::string& fileName, EntitySprite& sprite, int entityWidth, int entityHeight) {
    std::string spriteText;

    if (!reader.ReadTextFile(fileName, spriteText)) {
        return false;
    }

    sprite.pixels.clear();

    std::istringstream stream(spriteText);

    std::string line;

    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        sprite.pixels.push_back(line);
    }

    sprite.width = entityWidth;
    sprite.height = entityHeight;

    return true;
}

// Find sprite filename belonging to a state

static bool FindStateSprite(const std::string& text, const std::string& stateName, std::string& spriteName) {
    std::istringstream stream(text);

    std::string line;

    bool insideRequestedState = false;
    int braceDepth = 0;

    while (std::getline(stream, line)) {
        line = RemoveComment(line);
        line = Trim(line);

        if (line.empty())
            continue;

        // Find state
        if (!insideRequestedState && line.rfind("state ", 0) == 0) {

            std::string state = Trim(line.substr(6));

            if (state == stateName) {
                insideRequestedState = true;
                braceDepth = 0;
            }

            continue;
        }

        if (!insideRequestedState)
            continue;

        // Braces
        if (line == "{") {
            braceDepth++;
            continue;
        }

        if (line == "}") {
            if (braceDepth > 0)
                braceDepth--;

            if (braceDepth == 0)
                insideRequestedState = false;

            continue;
        }

        // Sprite filename
        if (line.back() == ';') {
            spriteName = RemoveSemicolon(line);

            return !spriteName.empty();
        }
    }

    return false;
}

static bool GetRangedAttackBlock(const std::string& text, std::string& block) {
    size_t start = text.find("ranged_attack");

    if (start == std::string::npos)
        return false;

    size_t open = text.find('{', start);

    if (open == std::string::npos)
        return false;

    size_t close = text.find('}', open);

    if (close == std::string::npos)
        return false;

    block = text.substr(open + 1, close - open - 1);

    return true;
}

void LoadEntityDefinitions(const std::string& ledPath)
{
    entityDefinitions.clear();

    LedReader reader;

    if (!reader.Open(ledPath)) {
        return;
    }

    // Process every LES file in LED
    for (const LedFileEntry& entry : reader.GetEntries()) {
        std::string fileName = entry.name;

        // Check extension
        if (fileName.size() < 4)
            continue;

        std::string extension = fileName.substr(fileName.size() - 4);

        extension = ToLower(extension);

        if (extension != ".les")
            continue;

        // Read complete LES file
        std::string text;

        if (!reader.ReadTextFile(fileName, text)) {
            continue;
        }

        EntityDefinition definition{};

        definition.id = -1;
        definition.width = 0;
        definition.height = 0;
        definition.health = 0.0f;
        definition.speed = 0.0f;
        definition.ranged = false;

        // Id
        std::string value;

        if (GetProperty(text, "Id", value)) {
            definition.id = std::stoi(value);
        }

        // Sprite width
        if (GetProperty(text, "Width", value)) {
            definition.width = std::stoi(value);
        }

        // Sprite height
        if (GetProperty(text, "Height", value)) {
            definition.height = std::stoi(value);
        }

        // Health
        if (GetProperty(text, "Health", value)) {
             definition.health = std::stof(value);
        }

        // Damage
        if (GetProperty(text, "Damage", value)) {
            ParseDiceExpression(value, definition.damageDice, definition.damageSides, definition.damageBonus);
        }

        // Speed
        if (GetProperty(text, "Speed", value)) {
            definition.speed = std::stof(value);
        }

        // Enemy type
        if (GetProperty(text, "Ranged", value)) {
            std::string lower = ToLower(value);

            definition.ranged = (lower == "true");
        }

        if (GetProperty(text, "Type", value)) {
            std::string type = ToLower(value);

            if (type == "enemy")
                definition.type = EntityType::ENEMY;
            else if (type == "corpse")
                definition.type = EntityType::CORPSE;
            else if (type == "fireball")
                definition.type = EntityType::FIREBALL;
            else if (type == "collectible")
                definition.type = EntityType::COLLECTIBLE;
            else if (type == "ammo")
                definition.type = EntityType::AMMO;
            else if (type == "medkit")
                definition.type = EntityType::MEDKIT;
            else if (type == "exit")
                definition.type = EntityType::EXIT;
        }

        // IDLE sprite
        if (FindStateSprite(text, "IDLE", definition.idleSprite)) {
            LoadEntitySprite(reader, definition.idleSprite, definition.idle, definition.width, definition.height);
        }

        if (FindStateSprite(text, "CHASE", definition.chaseSprite)) {
            LoadEntitySprite(reader, definition.chaseSprite, definition.chase, definition.width, definition.height);
        }

        if (FindStateSprite(text, "ATTACK", definition.attackSprite)) {
            LoadEntitySprite(reader, definition.attackSprite, definition.attack, definition.width, definition.height);
        }

        if (FindStateSprite(text, "DEATH", definition.deathSprite)) {
            LoadEntitySprite(reader, definition.deathSprite, definition.death, definition.width, definition.height);
        }

        std::string rangedBlock;

        if (GetRangedAttackBlock(text, rangedBlock)) {
            if (GetProperty(rangedBlock, "Width", value)) {
                definition.rangedAttack.width = std::stoi(value);
            }

            if (GetProperty(rangedBlock, "Height", value)) {
                definition.rangedAttack.height = std::stoi(value);
            }

            if (GetProperty(rangedBlock, "Speed", value)) {
                definition.rangedAttack.speed = std::stof(value);
            }

            // Ranged attack damage
            if (GetProperty(rangedBlock, "Damage", value)) {
                ParseDiceExpression(value, definition.rangedAttack.damageDice, definition.rangedAttack.damageSides, definition.rangedAttack.damageBonus);
            }

            // Find ranged attack sprite filename
            std::istringstream stream(rangedBlock);
            std::string line;

            while (std::getline(stream, line)) {
                line = RemoveComment(line);
                line = Trim(line);

                if (line.empty())
                    continue;

                if (line.back() == ';' && line.find('=') == std::string::npos) {
                    definition.rangedAttack.sprite = RemoveSemicolon(line);
                    break;
                }
            }

            // Load ranged attack sprite pixels
            if (!definition.rangedAttack.sprite.empty()) {
                LoadEntitySprite(reader, definition.rangedAttack.sprite, definition.rangedAttack.spriteData, 
                    definition.rangedAttack.width, definition.rangedAttack.height);
            }
        }

        entityDefinitions.push_back(std::move(definition));
    }
}

const EntityDefinition* FindEntityDefinition(int id) {
    for (const EntityDefinition& definition : entityDefinitions) {
        if (definition.id == id)
            return &definition;
    }

    return nullptr;
}

const EntityDefinition* FindEntityDefinitionByType(EntityType type) {
    for (const EntityDefinition& definition : entityDefinitions) {
        if (definition.type == type)
            return &definition;
    }

    return nullptr;
}