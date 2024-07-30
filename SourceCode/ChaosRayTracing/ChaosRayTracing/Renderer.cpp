#include "Renderer.hpp"

#include "PPMWriter.hpp"
#include "Scene.hpp"
#include "Image.hpp"

#include <thread>

void Renderer::writeToFile(const Image& image, const Scene::Settings& sceneSettings, uint32_t frame)
{
    const auto imageWidth = image.GetWidth();
    const auto imageHeight = image.GetHeight();
    PPMWriter writer(sceneSettings.sceneName + "_render_" + std::to_string(frame), imageWidth, imageHeight, maxColorComponent);

    // Reserve enough space in the string buffer
    std::string buffer;
    buffer.reserve(imageWidth * imageHeight * 12);

    const unsigned int numThreads = std::thread::hardware_concurrency();
    const unsigned int rowsPerThread = imageHeight / numThreads;

    std::vector<std::string> threadBuffers(numThreads);
    std::vector<std::thread> threads;

    auto processRows = [&](unsigned int startRow, unsigned int endRow, std::string& localBuffer) {
        localBuffer.reserve((endRow - startRow) * imageWidth * 12);
        for (uint32_t rowIdx = startRow; rowIdx < endRow; ++rowIdx) 
        {
            for (uint32_t colIdx = 0; colIdx < imageWidth; ++colIdx) 
            {
                localBuffer.append(image.GetPixel(colIdx, rowIdx).toString()).append("\t");
            }
            localBuffer.append("\n");
        }
        };

    for (unsigned int i = 0; i < numThreads; ++i) 
    {
        unsigned int startRow = i * rowsPerThread;
        unsigned int endRow = (i == numThreads - 1) ? imageHeight : startRow + rowsPerThread;
        threads.emplace_back(processRows, startRow, endRow, std::ref(threadBuffers[i]));
    }

    for (auto& thread : threads)
        thread.join();

    for (const auto& localBuffer : threadBuffers)
        buffer.append(localBuffer);

    writer << buffer;
}