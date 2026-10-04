#include <sstream>
#include <vector>

#include "LevelDefinitions.h"
#include "LedReader.h"

static std::vector<LevelDefinition> levelDefinitions;

static std::vector<std::string> GetMapBlocks(const std::string& text) {
    std::vector<std::string> blocks;

    size_t searchPos = 0;

    while (true) {
        size_t mapPos = text.find("map ", searchPos);

        if (mapPos == std::string::npos)
            break;

        size_t openBrace = text.find('{', mapPos);

        if (openBrace == std::string::npos)
            break;

        size_t closeBrace = text.find('}', openBrace);

        if (closeBrace == std::string::npos)
            break;

        blocks.push_back(text.substr(mapPos, closeBrace - mapPos + 1));

        searchPos = closeBrace + 1;
    }

    return blocks;
}

// Remove whitespace from both ends
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

static std::string RemoveComments(const std::string& text) {
    std::istringstream stream(text);
    std::ostringstream result;

    std::string line;

    while (std::getline(stream, line)) {
        line = RemoveComment(line);
        result << line << '\n';
    }

    return result.str();
}

// Find a property in the map block
static std::string GetProperty(const std::string& block, const std::string& property) {
    std::istringstream stream(block);
    std::string line;

    while (std::getline(stream, line)) {
        // Remove comments before parsing the line
        line = RemoveComment(line);
        line = Trim(line);

        if (line.empty())
            continue;

        size_t equalsPos = line.find('=');

        if (equalsPos == std::string::npos)
            continue;

        std::string key = Trim(line.substr(0, equalsPos));

        if (key != property)
            continue;

        std::string value = Trim(line.substr(equalsPos + 1));

        // Remove trailing semicolon
        if (!value.empty() && value.back() == ';')
            value.pop_back();

        return Trim(value);
    }

    return "";
}

bool LoadLevelDefinitions(const std::string& ledPath) {
    LedReader reader;

    if (!reader.Open(ledPath)) {
        return false;
    }

    std::string text;

    if (!reader.ReadTextFile("GAME.LEM", text)) {
        return false;
    }

    text = RemoveComments(text);

    std::vector<std::string> blocks = GetMapBlocks(text);

    for (const std::string& block : blocks) {
        LevelDefinition definition;

        // Get map name
        size_t mapPos = block.find("map ");

        if (mapPos == std::string::npos)
            continue;

        size_t nameStart = mapPos + 4;

        size_t nameEnd = block.find_first_of(" \t\r\n{", nameStart);

        if (nameEnd == std::string::npos)
            continue;

        definition.mapName = block.substr(nameStart, nameEnd - nameStart);

        definition.mapFile = GetProperty(block, "Level");

        std::string playerX = GetProperty(block, "x");
        std::string playerY = GetProperty(block, "y");
        std::string playerAngle = GetProperty(block, "angle");
        std::string playerAmmo = GetProperty(block, "ammo");
        std::string playerHealth = GetProperty(block, "health");

        definition.nextMap = GetProperty(block, "Next");

        if (!playerX.empty()) 
            definition.playerX = std::stof(playerX);

        if (!playerY.empty()) 
            definition.playerY = std::stof(playerY);

        if (!playerAngle.empty())
            definition.playerAngle = std::stof(playerAngle);

        if (!playerAmmo.empty())
            definition.playerAmmo = std::stoi(playerAmmo);

        if (!playerHealth.empty())
            definition.playerHealth = std::stoi(playerHealth);

        if (definition.mapName.empty() || definition.mapFile.empty())
            continue;

        // Store
        levelDefinitions.push_back(definition);
    }
    return !levelDefinitions.empty();
}

const LevelDefinition* FindLevelDefinition(const std::string& mapName) {
    for (const LevelDefinition& definition : levelDefinitions) {
        if (definition.mapName == mapName)
            return &definition;
    }

    return nullptr;
}