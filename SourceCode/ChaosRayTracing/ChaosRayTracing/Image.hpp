#pragma once

#include <cstdint>
#include <vector>

#include "Math3D.hpp"

class Image
{
public:
    Image(uint32_t width, uint32_t height) : width(width), height(height)
    {
        pixels.resize(width * height);
    }

    void setPixel(uint32_t x, uint32_t y, const RGB& color)
    {
        pixels[y * width + x] = color;
    }

    const RGB& GetPixel(uint32_t x, uint32_t y) const
    {
        return pixels[y * width + x];
    }

    uint32_t GetWidth() const { return width; }
    uint32_t GetHeight() const { return height; }

private:
    uint32_t width, height;
    std::vector<RGB> pixels;
};
