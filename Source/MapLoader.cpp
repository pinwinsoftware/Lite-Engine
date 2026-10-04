#include "MapLoader.h"
#include "main.h"
#include "LedReader.h"

#include <sstream>

bool LoadMapFromLED(const std::string& ledPath, const std::string& mapFile) {
    LedReader reader;

    if (!reader.Open(ledPath))
        return false;

    std::string text;

    if (!reader.ReadTextFile(mapFile, text)) {
        return false;
    }

    currentMap->rows.clear();

    std::istringstream stream(text);
    std::string line;

    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        currentMap->rows.push_back(line);
    }

    currentMap->height = static_cast<int>(currentMap->rows.size());

    return true;
}