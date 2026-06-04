#include <cmath>

#include "Entity.h"

const char* monsterSprite[17] = {
    "     111111     ",
    "    10000001111 ",
    "   1101001011111",
    "   1100110011111",
    "  11110000111111",
    " 111  11111 1111",
    "111     11  111 ",
    " 1   111  11111 ",
    "    11111 11111 ",
    "    1111   111  ",
    "    111         ",
    "    111     11  ",
    "    111     11  ",
    "     1      111 ",
    "    11          ",
    "   1111         ",
    "  11111         ",
};

const char* coinSprite[10] = {
    "           ",
    "    111    ",
    "   10001   ",
    "  1001001  ",
    "  1011001  ",
    "  1001001  ",
    "  1001001  ",
    "   10001   ",
    "    111    ",
    "           ",
};

const char* trollSprite[13] = {
    "  11111111111   ",
    " 1000000000001  ",
    " 1000000000001  ",
    "101110111100001 ",
    "1001100011000001",
    " 100001000000001",
    " 100110001110001",
    " 11111111010001 ",
    " 1010101010001  ",
    "1011111110001   ",
    "100000000011    ",
    "1000001111      ",
    " 11111          ",
};

const char* exitSprite[5] = {
    "11111001000100111110011111",
    "10000000101000001000000100",
    "11111000010000001000000100",
    "10000000101000001000000100",
    "11111001000100111110000100",
};

std::vector<Entity> entities;

void RenderSprite(
    const Entity& sprite,
    float playerX,
    float playerY,
    float playerRad,
    float fov,
    int screenWidth,
    int screenHeight,
    const std::vector<float>& depthBuffer,
    std::vector<std::vector<char>>& screen
)
{
    float dx = sprite.x - playerX;
    float dy = sprite.y - playerY;

    float spriteDistance = sqrt(dx * dx + dy * dy);
    float spriteAngle = atan2(dy, dx) - playerRad;

    while (spriteAngle > 3.14159f) spriteAngle -= 2.0f * 3.14159f;
    while (spriteAngle < -3.14159f) spriteAngle += 2.0f * 3.14159f;

    float fovRad = fov * 3.14159f / 180.0f;

    if (fabs(spriteAngle) < fovRad / 2.0f)
    {
        int spriteScreenX = (int)((spriteAngle + fovRad / 2.0f) / fovRad * screenWidth);

        float aspectRatio = 2.0f;

        int spriteHeight = (int)(screenHeight / spriteDistance);
        int spriteWidth = (int)(spriteHeight * aspectRatio);

        int top = (screenHeight - spriteHeight) / 2;
        int left = spriteScreenX - spriteWidth / 2;

        for (int sx = 0; sx < spriteWidth; sx++)
        {
            int screenX = left + sx;
            if (screenX < 0 || screenX >= screenWidth) continue;
            if (spriteDistance >= depthBuffer[screenX]) continue;

            for (int sy = 0; sy < spriteHeight; sy++)
            {
                int screenY = top + sy;
                if (screenY < 0 || screenY >= screenHeight) continue;

                int texX = sx * sprite.w / spriteWidth;
                int texY = sy * sprite.h / spriteHeight;

                if (sprite.shape[texY][texX] == '1')
                    screen[screenY][screenX] = char(219);
                else if (sprite.shape[texY][texX] == '0')
                    screen[screenY][screenX] = ' ';
            }
        }
    }
}