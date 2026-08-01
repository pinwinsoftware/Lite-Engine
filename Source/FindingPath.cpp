#include "FindingPath.h"
#include "Game.h"

#include <queue>
#include <map>

std::vector<Node> FindPath(
    int startX,
    int startY,
    int targetX,
    int targetY
)
{
    std::vector<Node> empty;

    struct Cell
    {
        int x;
        int y;
    };

    std::queue<Cell> open;

    // Stores where each visited cell came from.
    // Used later to reconstruct the final path.
    std::map<std::pair<int, int>, std::pair<int, int>> parent;

    // Start searching from the starting position.
    open.push({ startX, startY });

    // The starting cell has no parent.
    parent[{startX, startY}] = { -1, -1 };

    int dirs[8][2] = {
        { 1, 0 },   // Right
        {-1, 0 },   // Left
        { 0, 1 },   // Down
        { 0,-1 },   // Up

        { 1, 1 },   // Down-right
        {-1, 1 },   // Down-left
        { 1,-1 },   // Up-right
        {-1,-1 }    // Up-left
    };

    while (!open.empty()) {
        // Get the next cell to explore.
        Cell current = open.front();
        open.pop();

        // Stop searching once it reach the target.
        if (current.x == targetX && current.y == targetY) {
            break;
        }

        // Check all neighbouring cells.
        for (int i = 0; i < 8; i++) {
            int nx = current.x + dirs[i][0];
            int ny = current.y + dirs[i][1];

            // Skip walls.
            if (GetMapCell(nx, ny) == '1')
                continue;

            // Prevent cutting diagonally through corners.
            if (dirs[i][0] != 0 && dirs[i][1] != 0) {
                if (GetMapCell(current.x + dirs[i][0], current.y) == '1')
                    continue;

                if (GetMapCell(current.x, current.y + dirs[i][1]) == '1')
                    continue;
            }

            // Skip cells enemy already visited.
            if (parent.find({ nx, ny }) != parent.end())
                continue;

            // Remember how enemy reached this cell.
            parent[{nx, ny}] = { current.x, current.y };

            // Explore this cell later.
            open.push({ nx, ny });
        }
    }

    if (parent.find({ targetX, targetY }) == parent.end())
        return empty;

    std::vector<Node> path;

    int x = targetX;
    int y = targetY;

    while (x != -1 && y != -1) {
        path.push_back({ x, y });

        auto p = parent[{x, y}];

        x = p.first;
        y = p.second;
    }

    std::reverse(path.begin(), path.end());

    return path;
}