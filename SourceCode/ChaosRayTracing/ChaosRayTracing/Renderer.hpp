#pragma once

#include <sstream>

#include "PPMWriter.hpp"
#include "Scene.hpp"

#include <thread>
#include <utility>
#include "ThreadPool.hpp"

class Image
{
public:
    Image(uint32_t width, uint32_t height) : width(width), height(height)
    {
        pixels.resize(width * height);
    }

    void SetPixel(uint32_t x, uint32_t y, const RGB& color)
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

class Renderer
{
public:
    Renderer(Scene& scene) : scene(scene) {}

    void RenderImage()
    {
        Scene::Settings sceneSettings = scene.settings;

        const uint32_t imageWidth = sceneSettings.imageSettings.width;
        const uint32_t imageHeight = sceneSettings.imageSettings.height;

        for(uint32_t frame = 68; frame < frameCount; frame++)
        {
            // Set camera
            float phi = 2.0f * std::numbers::pi * static_cast<float>(frame) / frameCount;
            float radius = 2.2f;
            Vector3 cameraPosition = Vector3(radius * sinf(phi), 1.f, radius * cosf(phi));
            Vector3 center(0.f, 1.f, 0.f);
            Vector3 up(0.f, 1.f, 0.f);
            scene.camera.transform = LookAtInverse(cameraPosition, center, up);

            Image image(imageWidth, imageHeight);

            ThreadPool threadPool;
            std::vector<std::future<void>> results;
            uint32_t bucketSize = sceneSettings.imageSettings.bucketSize;
            for (uint32_t startRow = 0; startRow < imageHeight; startRow += bucketSize)
            {
                uint32_t endRow = startRow + bucketSize;
                for (uint32_t startColumn = 0; startColumn < imageWidth; startColumn += bucketSize)
                {
                    uint32_t endColumn = startColumn + bucketSize;
                    results.emplace_back(threadPool.Enqueue([&, startRow, endRow, startColumn, endColumn]
                        {
                            for (uint32_t rowIdx = startRow; rowIdx < endRow; ++rowIdx)
                            {

                                for (uint32_t colIdx = startColumn; colIdx < endColumn; ++colIdx)
                                {


                                    Vector3 color{ 0.f };

                                    std::random_device rd; // Seed
                                    std::mt19937 gen(rd()); // Mersenne Twister generator
                                    std::uniform_real_distribution<float> dis(-0.5f, 0.5f); // Uniform distribution between -0.5 and 0.5

                                    for(uint32_t sample = 0; sample < sampleCount; sample++)
                                    {
                                        float y = static_cast<float>(rowIdx) + 0.5f + dis(gen); // To pixel center
                                        y /= imageHeight; // To NDC
                                        y = 1.f - (2.f * y); // To screen space

                                        float x = static_cast<float>(colIdx) + 0.5f + dis(gen); // To pixel center
                                        x /= imageWidth; // To NDC
                                        x = 2.f * x - 1.f; // To screen space
                                        x *= static_cast<float>(imageWidth) / imageHeight; // Consider aspect ratio

                                        color += GetPixel(x, y);
                                    }

                                    color /= static_cast<float>(sampleCount);

                                    image.SetPixel(colIdx, rowIdx, color.ToRGB());
                                }
                            }
                        }));
                }
            }

            for (auto&& result : results)
                result.get();

            WriteToFile(image, sceneSettings, frame);
        }
    }

protected:

    static void WriteToFile(const Image& image, const Scene::Settings& sceneSettings, uint32_t frame)
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
                    localBuffer.append(image.GetPixel(colIdx, rowIdx).ToString()).append("\t");
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

    Vector3 TraceRay(Ray& ray, uint32_t depth = 0)
    {
        Vector3 L{ 0.f };
        if (depth > maxDepth)
            return L;

        HitInfo hitInfo = scene.ClosestHit(ray);
        if (hitInfo.hit)
        {
            const auto& material = scene.materials[hitInfo.materialIndex];
            Vector3 normal = hitInfo.normal;
            const auto& triangle = scene.triangles[hitInfo.triangleIndex];
            if (material.smoothShading)
            {
                normal = triangle.GetNormal(hitInfo.barycentrics);
            }

            Vector3 offsetOrigin = OffsetRayOrigin(hitInfo.point, hitInfo.normal);
            if (material.type == Material::Type::DIFFUSE || material.type == Material::Type::CONSTANT)
            {
                for (const auto& light : scene.lights)
                {
                    Vector3 dirToLight = Normalize(light.position - offsetOrigin);
                    float distanceToLight = (light.position - offsetOrigin).Magnitude();
                    Ray shadowRay{ offsetOrigin, dirToLight, distanceToLight};
                    if (!scene.AnyHit(shadowRay))
                    {
                        float attenuation = 1.0f / (distanceToLight * distanceToLight);
                        L += material.GetAlbedo(hitInfo.barycentrics, triangle.GetUVs(hitInfo.barycentrics)) * std::max(0.f, Dot(normal, dirToLight)) * attenuation * light.intensity;
                    }
                }

                // Generate random direction
                Vector3 randomDirection = RandomInHemisphere(hitInfo.normal);
                Ray nextRay{ offsetOrigin, randomDirection };
                L += material.GetAlbedo(hitInfo.barycentrics, triangle.GetUVs(hitInfo.barycentrics)) * TraceRay(nextRay, depth + 1);
            }
            else if (material.type == Material::Type::EMISSIVE)
            {
                L += material.emission;
            }
            else if (material.type == Material::Type::REFLECTIVE)
            {
                Vector3 reflectionDir = Normalize(ray.directionN - normal * 2.f * Dot(normal, ray.directionN));
                Ray reflectionRay{ offsetOrigin,  reflectionDir };
                L += material.GetAlbedo(hitInfo.barycentrics, triangle.GetUVs(hitInfo.barycentrics)) * TraceRay(reflectionRay, depth + 1);
            }
            else if (material.type == Material::Type::REFRACTIVE)
            {
                float eta = material.ior;
                Vector3 wi = -ray.directionN;
                float cosThetaI = Dot(normal, wi);
                bool flipOrientation = cosThetaI < 0.f;
                if (flipOrientation)
                {
                    eta = 1.f / eta;
                    cosThetaI = -cosThetaI;
                    normal = -normal;
                }

                float sin2ThetaI = std::max(0.f, 1.f - cosThetaI * cosThetaI);
                float sin2ThetaT = sin2ThetaI / (eta * eta);
                if (sin2ThetaT >= 1.f)
                {
                    // Total internal reflection case
                    Vector3 reflectionDir = Normalize(ray.directionN - normal * 2.f * Dot(normal, ray.directionN));
                    Ray reflectionRay{ offsetOrigin,  reflectionDir };
                    L += TraceRay(reflectionRay, depth + 1);
                }
                else
                {
                    float cosThetaT = std::sqrt(1.f - sin2ThetaT);
                    Vector3 wt = -wi / eta + (cosThetaI / eta - cosThetaT) * normal;
                    Vector3 offsetOriginRefraction = OffsetRayOrigin(hitInfo.point, flipOrientation ? hitInfo.normal : -hitInfo.normal);
                    Ray refractionRay{ offsetOriginRefraction, wt };
                    Vector3 refractionL = TraceRay(refractionRay, depth + 1);

                    Vector3 reflectionDir = Normalize(ray.directionN - normal * 2.f * Dot(normal, ray.directionN));
                    Vector3 offsetOriginReflection = OffsetRayOrigin(hitInfo.point, flipOrientation ? -hitInfo.normal : hitInfo.normal);
                    Ray reflectionRay{ offsetOriginReflection,  reflectionDir };
                    Vector3 reflectionL = TraceRay(reflectionRay, depth + 1);

                    float fresnel = 0.5f * std::powf(1.f + Dot(ray.directionN, normal), 5);

                    L += fresnel * reflectionL + (1.f - fresnel) * refractionL;

                }
            }
        }
        else
        {
            L += scene.settings.backgroundColor;
        }

        return L;
    }

    Vector3 GetPixel(float x, float y)
    {
        Vector3 origin = scene.camera.GetPosition();
        Vector3 forward = scene.camera.GetLookDirection();

        // Assume up vector is Y axis in camera space and right vector is X axis in camera space
        Vector3 up = Normalize(scene.camera.transform * Vector3(0.f, 1.f, 0.f));
        Vector3 right = Cross(forward, up);

        // Calculate direction to pixel in camera space
        Vector3 direction = Normalize(forward + right * x + up * y);

        Ray ray{ origin, direction };

        Vector3 L = TraceRay(ray);
        return L;
    }

    static constexpr uint32_t maxDepth = 5;
    static constexpr uint32_t maxColorComponent = 255;
    static constexpr uint32_t sampleCount = 2048;
    static constexpr uint32_t frameCount = 144;
    Scene& scene;
};