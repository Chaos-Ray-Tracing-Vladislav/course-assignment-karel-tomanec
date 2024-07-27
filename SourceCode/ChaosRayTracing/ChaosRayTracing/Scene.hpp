#pragma once

#include "Camera.hpp"
#include "BVH.hpp"
#include "Material.hpp"
#include "SceneParser.hpp"
#include "Light.hpp"

#include <vector>
#include <algorithm>
#include <map>


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
		uint32_t bucketSize;
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
		bvh = BVH(triangles);
	}

	HitInfo ClosestHit(const Ray& ray) const
	{
		return bvh.closestHit(triangles, ray);
	}

	bool AnyHit(const Ray& ray) const
	{
		return bvh.anyHit(triangles, materials, ray);
	}

	Camera camera;
	std::vector<Triangle> triangles;
	std::vector<Material> materials;
	std::map<std::string, std::shared_ptr<const Texture>> textures;
	std::vector<Light> lights;
	BVH bvh;
	Settings settings;
};