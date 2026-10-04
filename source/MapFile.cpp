#include <fstream>
#include <sstream>

#include "MapFile.h"
#include "EditorState.h"

bool NewMap() {
    g_mapRows.clear();

    for (int y = 0; y < g_mapSize; ++y) {
        g_mapRows.emplace_back(g_mapSize, ' ');
    }

    return true;
}

bool OpenMap(const std::string& filename) {
    std::ifstream file(filename);

    if (!file)
        return false;

    std::vector<std::string> rows;

    std::string row;

    while (std::getline(file, row)) {
        // Remove Windows CR from CRLF files
        if (!row.empty() && row.back() == '\r') {
            row.pop_back();
        }

        if (row.empty())
            continue;

        rows.push_back(row);
    }

    if (rows.empty())
        return false;

    int mapSize = static_cast<int>(rows[0].size());

    if (mapSize < MIN_MAP_SIZE || mapSize > MAX_MAP_SIZE) {
        return false;
    }

    if (static_cast<int>(rows.size()) != mapSize)
        return false;

    for (const std::string& currentRow : rows) {
        if (static_cast<int>(currentRow.size()) != mapSize)
            return false;
    }

    g_mapSize = mapSize;
    g_mapRows = rows;

    g_undoStack.clear();
    g_redoStack.clear();

    return true;
}

bool SaveMap(const std::string& filename) {
    std::ofstream file(filename);

    if (!file)
        return false;

    for (int y = 0; y < g_mapSize; ++y) {
        if (y >= static_cast<int>(g_mapRows.size())) {
            return false;
        }

        const std::string& row = g_mapRows[y];

        if (static_cast<int>(row.size()) != g_mapSize)
            return false;

        file << row << "\n";
    }

    return true;
}

bool SaveMapAs(std::string& filename) {
    if (!SaveMap(filename))
        return false;

    return true;
}