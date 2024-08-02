#pragma once

#include <fstream>
#include <string>
#include <stdexcept>
#include <utility>

class PPMWriter
{
public:
    PPMWriter(const std::string& filename, uint32_t imageWidth, uint32_t imageHeight, uint32_t maxColorComponent)
        : ppmFileStream(filename + ".ppm", std::ios::out | std::ios::binary),
        imageWidth(imageWidth),
        imageHeight(imageHeight),
        maxColorComponent(maxColorComponent)
    {
        if (!ppmFileStream.is_open())
            throw std::runtime_error("Failed to open file: " + filename + ".ppm");

        ppmFileStream << "P3\n";
        ppmFileStream << imageWidth << " " << imageHeight << "\n";
        ppmFileStream << maxColorComponent << "\n";
    }

    ~PPMWriter()
    {
        if (ppmFileStream.is_open())
            ppmFileStream.close();
    }

    PPMWriter(PPMWriter&& other) noexcept
        : ppmFileStream(std::move(other.ppmFileStream)),
        imageWidth(other.imageWidth),
        imageHeight(other.imageHeight),
        maxColorComponent(other.maxColorComponent)
    {
        other.imageWidth = 0;
        other.imageHeight = 0;
        other.maxColorComponent = 0;
    }

    PPMWriter& operator=(PPMWriter&& other) noexcept
    {
        if (this != &other)
        {
            if (ppmFileStream.is_open())
                ppmFileStream.close();

            ppmFileStream = std::move(other.ppmFileStream);
            imageWidth = other.imageWidth;
            imageHeight = other.imageHeight;
            maxColorComponent = other.maxColorComponent;

            other.imageWidth = 0;
            other.imageHeight = 0;
            other.maxColorComponent = 0;
        }
        return *this;
    }

    PPMWriter(const PPMWriter&) = delete;
    PPMWriter& operator=(const PPMWriter&) = delete;

    PPMWriter& operator<<(const std::string& data)
    {
        if (!ppmFileStream.is_open())
            throw std::runtime_error("File stream is not open");

        ppmFileStream << data;
        return *this;
    }

private:
    std::ofstream ppmFileStream;
    uint32_t imageWidth;
    uint32_t imageHeight;
    uint32_t maxColorComponent;
};
