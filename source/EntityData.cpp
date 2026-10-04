#include "EntityData.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <cstdint>

#pragma pack(push, 1)

struct LedHeader {
    char magic[4];
    uint32_t fileCount;
};

struct LedFileEntry {
    char name[64];
    uint32_t offset;
    uint32_t size;
};

#pragma pack(pop)

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


static std::string ToLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        }
    );

    return value;
}

struct LEDAsset {
    std::string name;
    std::vector<char> data;
};

static bool LoadLEDAssets(const std::string& path, std::vector<LEDAsset>& assets) {
    assets.clear();

    std::ifstream led(path, std::ios::binary);

    if (!led)
        return false;

    LedHeader header{};

    led.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (!led)
        return false;

    if (std::memcmp(header.magic, "LED1", 4) != 0) {
        return false;
    }

    std::vector<LedFileEntry> entries;

    entries.reserve(header.fileCount);

    for (uint32_t i = 0; i < header.fileCount; ++i) {
        LedFileEntry entry{};

        led.read(reinterpret_cast<char*>(&entry), sizeof(entry));

        if (!led)
            return false;

        entry.name[sizeof(entry.name) - 1] = '\0';

        entries.push_back(entry);
    }

    for (const LedFileEntry& entry : entries) {
        LEDAsset asset;

        asset.name = entry.name;

        asset.data.resize(entry.size);

        led.clear();

        led.seekg(static_cast<std::streamoff>(entry.offset), std::ios::beg);

        if (!led)
            return false;

        if (entry.size > 0) {
            led.read(asset.data.data(), static_cast<std::streamsize>(entry.size));

            if (!led)
                return false;
        }

        assets.push_back(std::move(asset));
    }

    return true;
}

static const LEDAsset* FindAsset(const std::vector<LEDAsset>& assets, const std::string& fileName) {
    for (const LEDAsset& asset : assets) {
        if (asset.name == fileName)
            return &asset;
    }

    return nullptr;
}

static bool GetAssetText(const std::vector<LEDAsset>& assets, const std::string& fileName, std::string& text) {
    
    const LEDAsset* asset = FindAsset(assets, fileName);

    if (!asset)
        return false;

    text.assign(asset->data.begin(), asset->data.end());

    return true;
}

static bool GetProperty(const std::string& text, const std::string& property, std::string& value) {
    std::istringstream stream(text);

    std::string line;

    while (std::getline(stream, line)) {
        line = RemoveComment(line);
        line = Trim(line);

        if (line.empty())
            continue;

        size_t equalPos = line.find('=');

        if (equalPos == std::string::npos)
            continue;

        std::string left = Trim(line.substr(0, equalPos));

        if (left != property)
            continue;

        value = RemoveSemicolon(line.substr(equalPos + 1));

        return true;
    }

    return false;
}

static bool FindStateSprite(const std::string& text, const std::string& stateName, std::string& spriteName) {
    std::istringstream stream(text);

    std::string line;

    bool insideState = false;
    bool foundStateBrace = false;

    while (std::getline(stream, line)) {
        line = RemoveComment(line);
        line = Trim(line);

        if (line.empty())
            continue;

        // Match a state declaration such as "state IDLE"
        if (!insideState) {
            std::string lowerLine = ToLower(line);
            std::string lowerState = ToLower(stateName);

            if (lowerLine == "state " + lowerState) {
                insideState = true;
                foundStateBrace = false;
            }

            continue;
        }

        // We found the opening {
        if (!foundStateBrace) {
            if (line == "{") {
                foundStateBrace = true;
                continue;
            }

            // Abort this state if its opening brace is malformed
            if (line == "}") {
                insideState = false;
                continue;
            }

            continue;
        }

        // End of state
        if (line == "}") {
            return false;
        }

        std::string candidate = RemoveSemicolon(line);

        if (candidate.empty())
            continue;

        // Ignore another possible block
        if (candidate == "{")
            continue;

        spriteName = candidate;

        return true;
    }

    return false;
}

static bool FindGeometryTexture(const std::string& text, std::string& textureName) {
    std::istringstream stream(text);

    std::string line;

    bool insideGeometry = false;
    bool geometryBrace = false;
    bool insideTexture = false;
    bool textureBrace = false;

    while (std::getline(stream, line)) {
        line = RemoveComment(line);
        line = Trim(line);

        if (line.empty())
            continue;

        // Find a geometry block declaration
        if (!insideGeometry) {
            std::string lowerLine = ToLower(line);

            if (lowerLine.rfind("geometry ", 0) == 0) {
                insideGeometry = true;
                geometryBrace = false;
            }

            continue;
        }

        // Opening brace of geometry
        if (!geometryBrace) {
            if (line == "{") {
                geometryBrace = true;
                continue;
            }

            continue;
        }

        // Look for: texture
        if (!insideTexture)
        {
            if (ToLower(line) == "texture")
            {
                insideTexture = true;
                textureBrace = false;
            }

            // End of geometry
            if (line == "}")
            {
                insideGeometry = false;
                geometryBrace = false;
            }

            continue;
        }

        // Opening brace of texture
        if (!textureBrace) {
            if (line == "{") {
                textureBrace = true;
                continue;
            }

            continue;
        }

        // End of texture block
        if (line == "}") {
            return false;
        }

        // This should be: WALL.LSP;
        std::string candidate = RemoveSemicolon(line);

        if (candidate.empty())
            continue;

        textureName = candidate;
        return true;
    }

    return false;
}

static bool LoadSprite(const std::vector<LEDAsset>& assets, const std::string& fileName, EntitySprite& sprite, int width, int height) {

    std::string text;

    if (!GetAssetText(assets, fileName, text)) {
        return false;
    }

    sprite.pixels.clear();

    std::istringstream stream(text);

    std::string line;

    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        sprite.pixels.push_back(line);
    }

    sprite.width = width;
    sprite.height = height;

    return true;
}

static bool GetLESObjectType(const std::string& text, std::string& objectType, std::string& objectName) {
    std::istringstream stream(text);

    std::string line;

    while (std::getline(stream, line)) {
        line = RemoveComment(line);
        line = Trim(line);

        if (line.empty())
            continue;

        std::istringstream lineStream(line);

        std::string type;
        std::string name;

        lineStream >> type >> name;

        type = ToLower(type);

        if (type != "entity" && type != "geometry") {
            continue;
        }

        if (name.empty())
            return false;

        objectType = type;
        objectName = name;

        return true;
    }

    return false;
}

bool LoadLEDFile(const std::string& ledPath, std::vector<EntityDefinition>& entities) {
    entities.clear();

    std::vector<LEDAsset> assets;

    if (!LoadLEDAssets(ledPath, assets)) {
        return false;
    }

    for (const LEDAsset& asset : assets) {

        if (asset.name.size() < 4)
            continue;

        const std::string extension =
            ToLower(asset.name.substr(asset.name.size() - 4));

        if (extension != ".les")
            continue;

        std::string text(asset.data.begin(), asset.data.end());

        std::string objectType;
        std::string objectName;

        if (!GetLESObjectType(text, objectType, objectName)) {
            continue;
        }

        EntityDefinition entity{};

        entity.id = -1;
        entity.width = 16;
        entity.height = 16;
        entity.health = 100.0f;
        entity.speed = 3.0f;
        entity.ranged = false;

        // Entity name DEMON.LES -> DEMON
        entity.name = asset.name.substr(0, asset.name.size() - 4);

        std::string value;

        if (GetProperty(text, "Id", value)) {
            try {
                entity.id = std::stoi(value);
            }
            catch (...) {
                entity.id = -1;
            }
        }

        // Width
        if (GetProperty(text, "Width", value)) {
            try {
                entity.width = std::stoi(value);
            }
            catch (...) { }
        }

        // Height
        if (GetProperty(text, "Height", value)) {
            try {
                entity.height = std::stoi(value);
            }
            catch (...) { }
        }

        // Health
        if (GetProperty(text, "Health", value)) {
            try {
                entity.health = std::stof(value);
            }
            catch (...) { }
        }

        // Speed
        if (GetProperty(text, "Speed", value)) {
            try {
                entity.speed =
                    std::stof(value);
            }
            catch (...) { }
        }

        // Ranged
        if (GetProperty(text, "Ranged", value)) {
            entity.ranged = ToLower(value) == "true";
        }

        // IDLE SPRITE
        if (FindStateSprite(text, "IDLE", entity.idleSprite)) {
            LoadSprite(
                assets,
                entity.idleSprite,
                entity.idle,
                entity.width,
                entity.height
            );
        }

        std::string textureName;

        if (FindGeometryTexture(text, textureName)) {
            LoadSprite(
                assets,
                textureName,
                entity.idle,
                entity.width,
                entity.height
            );
        }

        // Death sprite
        if (FindStateSprite(text, "DEATH", entity.deathSprite)) {
            LoadSprite(
                assets,
                entity.deathSprite,
                entity.death,
                entity.width,
                entity.height
            );
        }

        // Add entity
        entities.push_back(std::move(entity));
    }

    return true;
}