#pragma once

#include <sstream>

#include "PPMWriter.hpp"
#include "Scene.hpp"
#include "Image.hpp"
#include "ThreadPool.hpp"

#include <thread>
#include <utility>

#include "Sampling.hpp"

class Renderer
{
public:

    Renderer(Scene& scene) : scene(scene) {}

    void RenderImage()
    {
        Scene::Settings sceneSettings = scene.settings;

        const uint32_t imageWidth = sceneSettings.imageSettings.width;
        const uint32_t imageHeight = sceneSettings.imageSettings.height;

        for(uint32_t frame = 0; frame < frameCount; frame++)
        {
            // Set camera
            float phi = 2.f * PI * static_cast<float>(frame) / frameCount;
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

private:

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

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(0.f, 1.f);

        Vector3 L = TraceRay(ray, false, 1.f, gen, dis);
        return L;
    }

    Vector3 TraceRay(Ray& ray, bool lightSampledByNEE, float prevBounceBrdfPdf, std::mt19937& gen, std::uniform_real_distribution<float>& dis, uint32_t depth = 0)
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
                normal = triangle.GetNormal(hitInfo.barycentrics);

            Vector3 offsetOrigin = OffsetRayOrigin(hitInfo.point, hitInfo.normal);
            if (material.type == Material::Type::DIFFUSE || material.type == Material::Type::CONSTANT)
            {
                Vector3 albedo = material.GetAlbedo(hitInfo.barycentrics, triangle.GetUVs(hitInfo.barycentrics));
                Vector3 brdf = albedo / PI;

                // Sample light
                std::optional<EmissiveLightSample> lightSampleOpt = scene.emissiveSampler.sample(offsetOrigin, Vector3(dis(gen), dis(gen),dis(gen)));
                if (lightSampleOpt.has_value())
                {
                    EmissiveLightSample lightSample = lightSampleOpt.value();
                    Vector3 dirToLight = Normalize(lightSample.position - offsetOrigin);
                    float distanceToLight = (lightSample.position - offsetOrigin).Magnitude();
                    Ray shadowRay{ offsetOrigin, dirToLight, distanceToLight };
                    if (!scene.AnyHit(shadowRay))
                    {
                        float nDotL = std::max(0.f, Dot(normal, dirToLight));

                        float lightPdf = lightSample.pdf;
                        float brdfPdf = std::max(0.f, Dot(hitInfo.normal, dirToLight)) / PI;

                        // Multiple importance sampling (MIS) weight
                        float misWeight = PowerHeuristic(lightPdf, brdfPdf);

                        if(lightPdf > 0.f)
                            L += misWeight * brdf * nDotL * lightSample.Le / lightPdf;
                    }
                }

                Vector3 randomDirection = RandomInHemisphereCosine(hitInfo.normal);
                Ray nextRay{ offsetOrigin, randomDirection };

                float pdf = std::max(0.f, Dot(hitInfo.normal, randomDirection)) / PI;

                Vector3 indirectLighting = TraceRay(nextRay, true, pdf, gen, dis, depth + 1);
                float nDotL = std::max(0.f, Dot(normal, randomDirection));

                if(pdf > 0.f)
					L += brdf * nDotL * indirectLighting / pdf;
            }
            else if (material.type == Material::Type::EMISSIVE)
            {
                float misWeight = 1.f;
                if (lightSampledByNEE)
                {
                    assert(triangle.emissiveIndex != -1);
                    float lightPdf = scene.emissiveSampler.evalPdf(triangle.emissiveIndex, ray.origin, hitInfo.point);
                    misWeight = PowerHeuristic(prevBounceBrdfPdf, lightPdf );
                }
            	L += material.emission * misWeight;
            }
            else if (material.type == Material::Type::REFLECTIVE)
            {
                Vector3 reflectionDir = Normalize(ray.directionN - normal * 2.f * Dot(normal, ray.directionN));
                Ray reflectionRay{ offsetOrigin,  reflectionDir };
                L += material.GetAlbedo(hitInfo.barycentrics, triangle.GetUVs(hitInfo.barycentrics)) * TraceRay(reflectionRay, false, 1.f, gen, dis, depth + 1);
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
                    L += TraceRay(reflectionRay, false, 1.f, gen, dis, depth + 1);
                }
                else
                {
                    float cosThetaT = std::sqrt(1.f - sin2ThetaT);
                    Vector3 wt = -wi / eta + (cosThetaI / eta - cosThetaT) * normal;
                    Vector3 offsetOriginRefraction = OffsetRayOrigin(hitInfo.point, flipOrientation ? hitInfo.normal : -hitInfo.normal);
                    Ray refractionRay{ offsetOriginRefraction, wt };
                    Vector3 refractionL = TraceRay(refractionRay, false, 1.f, gen, dis, depth + 1);

                    Vector3 reflectionDir = Normalize(ray.directionN - normal * 2.f * Dot(normal, ray.directionN));
                    Vector3 offsetOriginReflection = OffsetRayOrigin(hitInfo.point, flipOrientation ? -hitInfo.normal : hitInfo.normal);
                    Ray reflectionRay{ offsetOriginReflection,  reflectionDir };
                    Vector3 reflectionL = TraceRay(reflectionRay, false, 1.f, gen, dis, depth + 1);

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

    static void WriteToFile(const Image& image, const Scene::Settings& sceneSettings, uint32_t frame);

    static constexpr uint32_t maxDepth = 6;
    static constexpr uint32_t maxColorComponent = 255;
    static constexpr uint32_t sampleCount = 256;
    static constexpr uint32_t frameCount = 1;

    Scene& scene;
};
