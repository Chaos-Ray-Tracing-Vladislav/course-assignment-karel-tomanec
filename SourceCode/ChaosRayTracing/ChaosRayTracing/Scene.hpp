#pragma once

#include "Camera.hpp"
#include "BVH.hpp"
#include "Material.hpp"
#include "SceneParser.hpp"
#include "Light.hpp"

#include <vector>
#include <algorithm>
#include <map>
#include <optional>


class Scene
{
public:

	// Move constructor
	Scene(Scene&& other) noexcept
		: textures(std::move(other.textures)) {}

	// Move assignment operator
	Scene& operator=(Scene&& other) noexcept {
		if (this != &other) {
			textures = std::move(other.textures);
		}
		return *this;
	}

	// Disable copy constructor and copy assignment operator
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;

	struct ImageSettings
	{
		uint32_t width;
		uint32_t height;
		uint32_t bucketSize = 24;
	};
	struct Settings
	{
		std::string sceneName;
		Vector3 backgroundColor;
		ImageSettings imageSettings;
	};


	Scene(const std::string& fileName)
	{
		SceneParser sceneParser(*this);
		sceneParser.parseSceneFile(fileName);
		std::cout << fileName << " parsed.\n";
		bvh = BVH(triangles);
		std::cout << fileName << " BVH built.\n";
	}

	HitInfo ClosestHit(Ray& ray) const
	{
		return bvh.closestHit(triangles, materials, ray);
	}

	bool AnyHit(Ray& ray) const
	{
		return bvh.anyHit(triangles, materials, ray);
	}

	std::optional<LightSample> sampleLight(const Vector3 posW, const Vector2& rnd0, const Vector2& rnd1) const
	{
		if (lights.empty() && emissiveTriangles.empty())
			return std::nullopt;


		auto samplePointLight = [this](float rnd, const Vector2& rnd2, float choosePdf) -> LightSample
			{
				LightSample sample;
				size_t lightIndex = static_cast<size_t>(rnd * lights.size());
				lightIndex = std::min(lightIndex, lights.size() - 1);

				const Light& light = lights[lightIndex];
				sample.position = light.position;
				sample.Le = light.intensity * Vector3{1, 1, 1};


				float pdf = choosePdf / lights.size();
				sample.pdf = pdf;

				return sample;
			};

		auto sampleEmissiveLight = [this, &posW](float rnd, const Vector2& rnd2, float choosePdf) -> LightSample
			{
				size_t emissiveIndex = static_cast<size_t>(rnd * emissiveTriangles.size());
				emissiveIndex = std::min(emissiveIndex, emissiveTriangles.size() - 1);

				LightSample sample = emissiveTriangles[emissiveIndex].sample(posW, rnd2);

				float pdf = choosePdf / emissiveTriangles.size();
				sample.pdf *= pdf;

				return sample;
			};

		std::vector<std::function<LightSample(float, const Vector2&, float)>> sampleFunctions;

		if (!lights.empty()) 
		{
			sampleFunctions.emplace_back(samplePointLight);
		}

		if (!emissiveTriangles.empty()) 
		{
			sampleFunctions.emplace_back(sampleEmissiveLight);
		}

		float choosePdf = 1.f;
		if (!lights.empty() && !emissiveTriangles.empty())
			choosePdf = 0.5f;

		float rndChoose = rnd0.x;
		size_t functionIndex = static_cast<size_t>(rndChoose * sampleFunctions.size());
		functionIndex = std::min(functionIndex, sampleFunctions.size() - 1);

		return sampleFunctions[functionIndex](rnd0.y, rnd1, choosePdf);
	}

	Camera camera;
	std::vector<Triangle> triangles;
	std::vector<Material> materials;
	std::map<std::string, std::shared_ptr<const Texture>> textures;
	std::vector<Light> lights;
	std::vector<EmissiveTriangle> emissiveTriangles;
	BVH bvh;
	Settings settings;
};
