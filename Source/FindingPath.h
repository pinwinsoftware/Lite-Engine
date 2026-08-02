#pragma once

#include <vector>

struct Node {
    int x;
    int y;
};

std::vector<Node> FindPath(
    int startX,
    int startY,
    int targetX,
    int targetY
);