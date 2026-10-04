#include "WeaponsDefinitions.h"
#include "LedReader.h"

#include <sstream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <string>

static std::vector<WeaponDefinition> weaponDefinitions;

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

    if (!result.empty() && result.back() == ';') {
        result.pop_back();
    }

    return Trim(result);
}

// Lowercase
static std::string ToLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        }
    );

    return value;
}

/*
    Get properties

    Id;
    Width;
    Height;
    Damage;
    Speed;
    Ranged;
*/

static std::string GetProperty(const std::string& block, const std::string& property) {
    std::istringstream stream(block);

    std::string line;

    while (std::getline(stream, line)) {
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

        if (!value.empty() && value.back() == ';') {
            value.pop_back();
        }

        return Trim(value);
    }

    return "";
}

// Get matching brace

static std::string GetBracedBlock(const std::string& text, size_t openBrace) {
    
    if (openBrace == std::string::npos)
        return "";

    int depth = 0;

    for (size_t i = openBrace; i < text.length(); ++i) {
        if (text[i] == '{') {
            ++depth;
        }
        else if (text[i] == '}') {
            --depth;
            if (depth == 0) {
                return text.substr(openBrace, i - openBrace + 1);
            }
        }
    }
    return "";
}

/*
    Find weapon blocks

    weapon Knife
    {
        ...
    }
*/

static std::vector<std::string> GetWeaponBlocks(const std::string& text) {
    std::vector<std::string> blocks;

    size_t searchPos = 0;

    while (true) {
        size_t weaponPos = text.find("weapon ", searchPos);

        if (weaponPos == std::string::npos)
            break;

        size_t openBrace = text.find('{', weaponPos);

        if (openBrace == std::string::npos)
            break;

        std::string block = GetBracedBlock(text, openBrace);

        if (block.empty())
            break;

        blocks.push_back(text.substr(weaponPos, openBrace - weaponPos) + block);

        searchPos = openBrace + block.length();
    }
    return blocks;
}

// Load LSP sprite from LED
static void ParseAnimations(LedReader& reader, const std::string& block, WeaponDefinition& definition) {
    
    size_t animationPos = block.find("animation");

    if (animationPos == std::string::npos)
        return;

    size_t animationOpen = block.find('{', animationPos);

    if (animationOpen == std::string::npos)
        return;

    std::string animationBlock = GetBracedBlock(block, animationOpen);

    if (animationBlock.empty())
        return;

    size_t searchPos = 0;

    while (true) {
        
        size_t statePos = animationBlock.find("state ", searchPos);

        if (statePos == std::string::npos)
            break;

        size_t nameStart = statePos + 6;
        size_t nameEnd = animationBlock.find_first_of(" \t\r\n{", nameStart);

        if (nameEnd == std::string::npos)
            break;

        WeaponAnimationState animation;

        animation.state = animationBlock.substr(nameStart, nameEnd - nameStart);

        size_t stateOpen = animationBlock.find('{', nameEnd);

        if (stateOpen == std::string::npos)
            break;

        std::string stateBlock = GetBracedBlock(animationBlock, stateOpen);

        if (stateBlock.empty())
            break;

        std::istringstream stateStream(stateBlock);
        std::string line;

        while (std::getline(stateStream, line)) {
            line = RemoveComment(line);
            line = Trim(line);

            if (line.empty())
                continue;


            if (line == "{")
                continue;

            if (line == "}")
                continue;

            if (line.back() == ';') {
                line.pop_back();
            }

            line = Trim(line);

            if (line.empty())
                continue;

            animation.file = line;

            break;
        }

        if (!animation.file.empty()) {
            std::string spriteText;

            if (!reader.ReadTextFile(animation.file, spriteText)) {

            }
            else {
                std::istringstream spriteStream(spriteText);
                std::string spriteLine;

                while (std::getline(spriteStream, spriteLine)) {
                    if (!spriteLine.empty() && spriteLine.back() == '\r') {
                        spriteLine.pop_back();
                    }
                    animation.pixels.push_back(spriteLine);
                }
            }
        }

        if (!animation.state.empty() && !animation.file.empty()) {
            definition.animations.push_back(std::move(animation));
        }

        searchPos = stateOpen + stateBlock.length();
    }
}

bool LoadWeaponDefinitions(const std::string& ledPath) {
    
    weaponDefinitions.clear();

    LedReader reader;

    if (!reader.Open(ledPath)) {
        return false;
    }

    // Process every .LES in the LED
    for (const LedFileEntry& entry : reader.GetEntries()) {
        
        std::string fileName = entry.name;

        if (fileName.size() < 4)
            continue;

        std::string extension = fileName.substr(fileName.size() - 4);

        extension = ToLower(extension);

        if (extension != ".les")
            continue;

        std::string text;

        if (!reader.ReadTextFile(fileName, text)) {
            continue;
        }

        std::vector<std::string>
            blocks = GetWeaponBlocks(text);

        for (const std::string& block : blocks) {
            WeaponDefinition definition{};

            definition.id = -1;
            definition.width = 0;
            definition.height = 0;
            definition.speed = 0.0f;
            definition.ranged = false;

            size_t weaponPos = block.find("weapon "); // Weapon name

            if (weaponPos == std::string::npos)
                continue;

            size_t nameStart = weaponPos + 7;

            size_t nameEnd = block.find_first_of(" \t\r\n{", nameStart);

            if (nameEnd == std::string::npos)
                continue;

            definition.name = block.substr(nameStart, nameEnd - nameStart);

            // Properties
            std::string id = GetProperty(block, "Id");
            std::string width = GetProperty(block, "Width");
            std::string height = GetProperty(block, "Height");
            definition.damage = GetProperty(block, "Damage");
            std::string speed = GetProperty(block, "Speed");
            std::string ranged = GetProperty(block, "Ranged");

            // Convert
            try {
                if (!id.empty()) definition.id = std::stoi(id);

                if (!width.empty()) definition.width = std::stoi(width);

                if (!height.empty()) definition.height = std::stoi(height);

                if (!speed.empty()) definition.speed = std::stof(speed);
            }
            catch (...) {
                continue;
            }

            if (!ranged.empty()) {
                std::string value = ToLower(Trim(ranged));

                definition.ranged = value == "true" || value == "1" || value == "yes";
            }

            // Animations + LSP loading

            ParseAnimations(reader, block, definition);
            weaponDefinitions.push_back(std::move(definition));
        }
    }

    return !weaponDefinitions.empty();
}

// Find by name
const WeaponDefinition*
FindWeaponDefinition(const std::string& weaponName) {
    for (const WeaponDefinition& definition : weaponDefinitions) {
        if (definition.name == weaponName) {
            return &definition;
        }
    }
    return nullptr;
}

// Find by ID
const WeaponDefinition*
FindWeaponDefinitionById(int id) {
    for (const WeaponDefinition& definition : weaponDefinitions) {
        if (definition.id == id)
            return &definition;
    }
    return nullptr;
}