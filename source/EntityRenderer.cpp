#include <cstdint>
#include <string>
#include <vector>

#include "EntityRenderer.h"

int CreateEntityImage(HIMAGELIST imageList, const EntityDefinition& entity) {
    if (!imageList)
        return -1;

    const std::vector<std::string>& pixels = entity.idle.pixels;

    if (pixels.empty())
        return -1;

    const int spriteHeight = static_cast<int>(pixels.size());

    int spriteWidth = 0;

    for (const std::string& line : pixels) {
        if (static_cast<int>(line.size()) > spriteWidth) {
            spriteWidth = static_cast<int>(line.size());
        }
    }

    if (spriteWidth <= 0 || spriteHeight <= 0) {
        return -1;
    }

    constexpr int imageWidth = 32;
    constexpr int imageHeight = 32;

    constexpr int pixelSize = 2;

    int actualPixelSize = pixelSize;

    int spritePixelWidth = spriteWidth * pixelSize;
    int spritePixelHeight = spriteHeight * pixelSize;

    if (spritePixelWidth > imageWidth || spritePixelHeight > imageHeight) {
        actualPixelSize = 1;
    }

    int drawWidth = spriteWidth * actualPixelSize;

    int drawHeight = spriteHeight * actualPixelSize;

    int x = (imageWidth - drawWidth) / 2;

    int y = (imageHeight - drawHeight) / 2;

    BITMAPINFO bitmapInfo = {};

    bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth = imageWidth;
    bitmapInfo.bmiHeader.biHeight = -imageHeight;
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

    void* bitmapBits = nullptr;

    HDC screenDC = GetDC(nullptr);

    if (!screenDC)
        return -1;

    HBITMAP bitmap =
        CreateDIBSection(
            screenDC,
            &bitmapInfo,
            DIB_RGB_COLORS,
            &bitmapBits,
            nullptr,
            0
        );

    if (!bitmap) {
        ReleaseDC(nullptr, screenDC);
        return -1;
    }

    uint32_t* pixels32 = static_cast<uint32_t*>(bitmapBits);

    const uint32_t blue = RGB(221, 150, 58);
    const uint32_t white = 0xFFFFFFFF;

    for (int i = 0; i < imageWidth * imageHeight; ++i) { pixels32[i] = blue; }

    for (int row = 0; row < spriteHeight; ++row) {
        
        const std::string& line = pixels[row];

        for (int col = 0; col < spriteWidth; ++col) {
            
            char c = ' ';

            if (col < static_cast<int>(line.size())) {
                c = line[col];
            }

            uint32_t color = (c == '1') ? white : blue;

            for (int py = 0; py < actualPixelSize; ++py) {
                for (int px = 0; px < actualPixelSize; ++px) {
                    int imageX = x + col * actualPixelSize + px;

                    int imageY = y + row * actualPixelSize + py;

                    if (imageX >= 0 && imageX < imageWidth && imageY >= 0 && imageY < imageHeight) {
                        pixels32[imageY * imageWidth + imageX ] = color;
                    }
                }
            }
        }
    }

    int imageIndex = ImageList_Add(imageList, bitmap, nullptr);
    DeleteObject(bitmap);
    ReleaseDC(nullptr, screenDC);

    return imageIndex;
}