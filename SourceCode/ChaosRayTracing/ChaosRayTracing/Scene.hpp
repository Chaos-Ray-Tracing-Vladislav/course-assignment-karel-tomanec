#pragma once

#include "Camera.hpp"
#include "BVH.hpp"
#include "Material.hpp"
#include "SceneParser.hpp"
#include "Light.hpp"
#include "EmissiveSampler.hpp"

#include <vector>
#include <algorithm>
#include <map>
#include <optional>
#include <iostream>


class Scene
{
public:
	// Move constructor
	Scene(Scene&& other) noexcept
		: textures(std::move(other.textures))
	{
	}

	// Move assignment operator
	Scene& operator=(Scene&& other) noexcept
	{
		if (this != &other)
		{
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

	HitInfo closestHit(Ray& ray) const
	{
		return bvh.closestHit(triangles, materials, ray);
	}

	bool anyHit(Ray& ray) const
	{
		return bvh.anyHit(triangles, materials, ray);
	}

	Camera camera;

	std::vector<Triangle> triangles;
	BVH bvh;

	std::vector<Material> materials;
	std::map<std::string, std::shared_ptr<const Texture>> textures;

	std::vector<Light> lights;
	EmissiveSampler emissiveSampler;

	Settings settings;
};
