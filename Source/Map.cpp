#include "Map.h"
#include "main.h"

void DrawMap() {
    int tileWidth = (screenWidth >= 172) ? 2 : 1;

    int mapWidth = currentMap->rows[0].size() * tileWidth;
    int mapHeight = currentMap->height;

    int startX = (screenWidth - mapWidth) / 2;
    int startY = (screenHeight - mapHeight) / 2;

    for (int my = 0; my < currentMap->height; my++) {
        for (int mx = 0; mx < (int)currentMap->rows[my].size(); mx++) {
            char c = currentMap->rows[my][mx];

            // Map
            int sx = startX + mx * tileWidth;

            if (c == '1')
            {
                screen[startY + my][sx] = char(219);

                if (tileWidth == 2)
                    screen[startY + my][sx + 1] = char(219);
            }
            else
            {
                screen[startY + my][sx] = ' ';

                if (tileWidth == 2)
                    screen[startY + my][sx + 1] = ' ';
            }

            // Player
            int px = startX + (int)x * tileWidth;
            int py = startY + (int)y;

            screen[py][px] = char(219);

            if (tileWidth == 2)
                screen[py][px + 1] = char(219);
        }
    }
}