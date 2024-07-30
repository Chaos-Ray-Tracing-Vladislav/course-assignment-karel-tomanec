#pragma once

#include <sstream>

#include "PPMWriter.hpp"
#include "Scene.hpp"
#include "Image.hpp"
#include "ThreadPool.hpp"

#include <thread>
#include <utility>

#include "Sampling.hpp"

class Renderer final
{
public:
	Renderer(Scene& scene) : scene(scene)
	{
	}

	void renderImage()
	{
		Scene::Settings sceneSettings = scene.settings;

		const uint32_t imageWidth = sceneSettings.imageSettings.width;
		const uint32_t imageHeight = sceneSettings.imageSettings.height;

		for (uint32_t frame = 0; frame < frameCount; frame++)
		{
			// Set camera for the final scene
			float phi = 2.f * PI * static_cast<float>(frame) / frameCount;
			float radius = 2.2f;
			Vector3 cameraPosition = Vector3(radius * sinf(phi), 1.f, radius * cosf(phi));
			Vector3 center(0.f, 1.f, 0.f);
			Vector3 up(0.f, 1.f, 0.f);
			scene.camera.transform = lookAtInverse(cameraPosition, center, up);

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
								Vector3 color{0.f};

								Sampling::RandomSampler randomSampler;

								for (uint32_t sample = 0; sample < sampleCount; sample++)
								{
									float y = static_cast<float>(rowIdx) + randomSampler.next1D();
									y /= static_cast<float>(imageHeight); // To NDC
									y = 1.f - (2.f * y); // To screen space

									float x = static_cast<float>(colIdx) + randomSampler.next1D();
									x /= static_cast<float>(imageWidth); // To NDC
									x = 2.f * x - 1.f; // To screen space
									x *= static_cast<float>(imageWidth) / static_cast<float>(imageHeight);
									// Consider aspect ratio

									color += getPixel(x, y);
								}

								color /= static_cast<float>(sampleCount);

								image.setPixel(colIdx, rowIdx, color.toRGB());
							}
						}
					}));
				}
			}

			for (auto&& result : results)
				result.get();

			writeToFile(image, sceneSettings, frame);
		}
	}

private:
	Vector3 getPixel(float x, float y)
	{
		Vector3 origin = scene.camera.getPosition();
		Vector3 forward = scene.camera.getLookDirection();

		// Assume up vector is Y axis in camera space and right vector is X axis in camera space
		Vector3 up = Normalize(scene.camera.transform * Vector3(0.f, 1.f, 0.f));
		Vector3 right = Cross(forward, up);

		// Calculate direction to pixel in camera space
		Vector3 direction = Normalize(forward + right * x + up * y);

		Ray ray{origin, direction};

		Sampling::RandomSampler randomSampler;
		Vector3 L = traceRay(ray, {}, randomSampler, 0);

		return L;
	}

	struct PrevBounceInfo
	{
		bool lightSampledByNEE = false;
		float bsdfPdf = 1.f;
	};

	Vector3 traceRay(Ray& ray, PrevBounceInfo prevBounceInfo, Sampling::RandomSampler& rnd, uint32_t depth)
	{
		Vector3 L{0.f};
		if (depth > maxDepth)
			return L;

		HitInfo hitInfo = scene.closestHit(ray);
		if (hitInfo.hit)
		{
			const auto& material = scene.materials[hitInfo.materialIndex];
			Vector3 normal = hitInfo.normal;
			const auto& triangle = scene.triangles[hitInfo.triangleIndex];

			if (material.smoothShading)
				normal = triangle.getNormal(hitInfo.barycentrics);

			Vector3 offsetOrigin = OffsetRayOrigin(hitInfo.point, hitInfo.normal);
			if (material.type == Material::Type::DIFFUSE || material.type == Material::Type::CONSTANT)
			{
				Vector3 albedo = material.getAlbedo(hitInfo.barycentrics, triangle.getUVs(hitInfo.barycentrics));
				Vector3 bsdf = albedo / PI;

				// Iterate over explicit lights
				for (const auto& light : scene.lights)
				{
					Vector3 dirToLight = Normalize(light.position - offsetOrigin);
					float distanceToLight = (light.position - offsetOrigin).magnitude();
					Ray shadowRay{ offsetOrigin, dirToLight, distanceToLight};
					if (!scene.anyHit(shadowRay))
					{
						float attenuation = 1.0f / (distanceToLight * distanceToLight);
						float nDotL = std::max(0.f, Dot(normal, dirToLight));
						L += albedo * nDotL * attenuation * light.intensity;
					}
				}

				// Sample emissive geometry
				std::optional<EmissiveLightSample> lightSampleOpt = scene.emissiveSampler.sample(
					offsetOrigin, rnd.next3D());
				if (lightSampleOpt.has_value())
				{
					EmissiveLightSample lightSample = lightSampleOpt.value();
					Vector3 dirToLight = Normalize(lightSample.position - offsetOrigin);
					float distanceToLight = (lightSample.position - offsetOrigin).magnitude();
					Ray shadowRay{offsetOrigin, dirToLight, distanceToLight};
					if (!scene.anyHit(shadowRay))
					{
						float nDotL = std::max(0.f, Dot(normal, dirToLight));

						float lightPdf = lightSample.pdf;
						float bsdfPdf = std::max(0.f, Dot(hitInfo.normal, dirToLight)) / PI;

						// Multiple importance sampling (MIS) weight
						float misWeight = Sampling::powerHeuristic(lightPdf, bsdfPdf);

						if (lightPdf > 0.f)
							L += misWeight * bsdf * nDotL * lightSample.Le / lightPdf;
					}
				}

				Vector3 randomDirection = randomInHemisphereCosine(hitInfo.normal, rnd.next2D());
				Ray nextRay{offsetOrigin, randomDirection};

				float pdf = std::max(0.f, Dot(hitInfo.normal, randomDirection)) / PI;

				Vector3 indirectLighting = traceRay(nextRay, { true, pdf }, rnd, depth + 1);
				float nDotL = std::max(0.f, Dot(normal, randomDirection));

				if (pdf > 0.f)
					L += bsdf * nDotL * indirectLighting / pdf;
			}
			else if (material.type == Material::Type::EMISSIVE)
			{
				float misWeight = 1.f;
				if (prevBounceInfo.lightSampledByNEE)
				{
					assert(triangle.emissiveIndex != -1);
					float lightPdf = scene.emissiveSampler.evalPdf(triangle.emissiveIndex, ray.origin, hitInfo.point);
					misWeight = Sampling::powerHeuristic(prevBounceInfo.bsdfPdf, lightPdf);
				}
				L += material.emission * misWeight;
			}
			else if (material.type == Material::Type::REFLECTIVE)
			{
				Vector3 reflectionDir = Normalize(ray.directionN - normal * 2.f * Dot(normal, ray.directionN));
				Ray reflectionRay{offsetOrigin, reflectionDir};
				Vector3 albedo = material.getAlbedo(hitInfo.barycentrics, triangle.getUVs(hitInfo.barycentrics));
				L += albedo * traceRay(reflectionRay, {}, rnd, depth + 1);
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
					Ray reflectionRay{offsetOrigin, reflectionDir};
					L += traceRay(reflectionRay, {}, rnd, depth + 1);
				}
				else
				{
					float cosThetaT = std::sqrt(1.f - sin2ThetaT);
					Vector3 wt = -wi / eta + (cosThetaI / eta - cosThetaT) * normal;
					Vector3 offsetOriginRefraction = OffsetRayOrigin(hitInfo.point,
					                                                 flipOrientation
						                                                 ? hitInfo.normal
						                                                 : -hitInfo.normal);
					Ray refractionRay{offsetOriginRefraction, wt};
					Vector3 refractionL = traceRay(refractionRay, {}, rnd, depth + 1);

					Vector3 reflectionDir = Normalize(ray.directionN - normal * 2.f * Dot(normal, ray.directionN));
					Vector3 offsetOriginReflection = OffsetRayOrigin(hitInfo.point,
					                                                 flipOrientation
						                                                 ? -hitInfo.normal
						                                                 : hitInfo.normal);
					Ray reflectionRay{offsetOriginReflection, reflectionDir};
					Vector3 reflectionL = traceRay(reflectionRay, {}, rnd, depth + 1);

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

	static void writeToFile(const Image& image, const Scene::Settings& sceneSettings, uint32_t frame);

	static constexpr uint32_t maxDepth = 5;
	static constexpr uint32_t maxColorComponent = 255;
	static constexpr uint32_t sampleCount = 16;
	static constexpr uint32_t frameCount = 1;

	Scene& scene;
};
