#pragma once

#include "Math3D.hpp"
#include "Camera.hpp"

#define RAPIDJSON_NOMEMBERITERATORCLASS
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

#include <vector>
#include <algorithm>
#include <iostream>
#include <map>
#include "Textures.hpp"

// Helper functions
Vector3 loadVector(const rapidjson::Value::ConstArray& arr)
{
	assert(arr.Size() == 3);
	return Vector3{
		static_cast<float>(arr[0].GetDouble()),
		static_cast<float>(arr[1].GetDouble()),
		static_cast<float>(arr[2].GetDouble())
	};
}

Matrix4 loadMatrix(const rapidjson::Value::ConstArray& arr) 
{
	assert(arr.Size() == 9);
	Matrix4 result = Matrix4::Identity();
	for(uint32_t i = 0; i < 3; i++)
	{
		for(uint32_t j = 0; j < 3; j++) 
			result(i, j) = static_cast<float>(arr[i + 3 * j].GetDouble());
	}
	return result;
}

std::vector<Vector3> loadVertices(const rapidjson::Value::ConstArray& arr) 
{
	assert(arr.Size() % 3 == 0);
	std::vector<Vector3> result;
	result.reserve(arr.Size() / 3);
	for(uint32_t i = 0; i < arr.Size(); i += 3) {
		result.emplace_back(
			Vector3(
				static_cast<float>(arr[i].GetDouble()), 
				static_cast<float>(arr[i+1].GetDouble()), 
				static_cast<float>(arr[i+2].GetDouble())
			)
		);
	}
	return result;
}

std::vector<Vector2> loadUVs(const rapidjson::Value::ConstArray& arr) 
{
	assert(arr.Size() % 3 == 0);
	std::vector<Vector2> result;
	result.reserve(arr.Size() / 3);
	for(uint32_t i = 0; i < arr.Size(); i += 3) {
		result.emplace_back(
			Vector2(
				static_cast<float>(arr[i].GetDouble()), 
				static_cast<float>(arr[i+1].GetDouble())
			)
		);
	}
	return result;
}

std::vector<uint32_t> loadIndices(const rapidjson::Value::ConstArray& arr) 
{
	assert(arr.Size() % 3 == 0);
	std::vector<uint32_t> result;
	result.reserve(arr.Size() / 3);
	for(uint32_t i = 0; i < arr.Size(); ++i)
		result.emplace_back(arr[i].GetInt());
	return result;
}

struct Light
{
	float intensity;
	Vector3 position;
};

class Material
{
public:
	enum Type
	{
		CONSTANT,
		DIFFUSE,
		REFLECTIVE,
		REFRACTIVE
	};

	Type type;
	float ior;
	bool smoothShading;
	std::shared_ptr<const Texture> texture;

	void SetAlbedo(Vector3 albedo)
	{
		this->albedo = albedo;
	}

	Vector3 GetAlbedo(const Vector2& barycentrics, const Vector2& uv) const
	{
		if (texture)
			return texture->GetColor(barycentrics, uv);

		return albedo;
	}

private:

	Vector3 albedo{ 1.f };
};

class Mesh
{
public:
	std::vector<Triangle> triangles;
	uint32_t materialIndex;
};

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
	};
	struct Settings
	{
		std::string sceneName;
		Vector3 backgroundColor;
		ImageSettings imageSettings;
		uint32_t bucketSize;
	};


	Scene(const std::string& fileName)
	{
		parseSceneFile(fileName);
	}

	HitInfo ClosestHit(const Ray& ray) const
	{
		HitInfo hitInfo;
		for (uint32_t meshIndex = 0; meshIndex < meshes.size(); ++meshIndex)
		{
			const auto& mesh = meshes[meshIndex];
			for (uint32_t triangleIndex = 0; triangleIndex < mesh.triangles.size(); ++triangleIndex)
			{
				const auto& triangle = mesh.triangles[triangleIndex];
				HitInfo currHitInfo = triangle.Intersect(ray);
				if (currHitInfo.hit && currHitInfo.t < hitInfo.t)
				{
					currHitInfo.meshIndex = meshIndex;
					currHitInfo.triangleIndex = triangleIndex;
					hitInfo = std::move(currHitInfo);
				}
			}
		}
		return hitInfo;
	}

	bool AnyHit(const Ray& ray) const
	{
		bool hit = false;
		for (const auto& mesh : meshes)
		{
			const auto& material = materials[mesh.materialIndex];
			if (material.type == Material::Type::REFRACTIVE)
				continue;
			hit = std::any_of(mesh.triangles.begin(), mesh.triangles.end(), [&ray](const Triangle& triangle)
				{
					HitInfo hitInfo = triangle.Intersect(ray);
					return hitInfo.hit;
				});
			if (hit)
				break;
		}
		return hit;
	}

	Camera camera;
	std::vector<Mesh> meshes;
	std::vector<Material> materials;
	std::map<std::string, std::shared_ptr<const Texture>> textures;
	std::vector<Light> lights;
	Settings settings;

protected:

	inline static const std::string kSceneSettingsStr{ "settings" };
	inline static const std::string kBackgroundColorStr{ "background_color" };
	inline static const std::string kImageSettingsStr{ "image_settings" };
	inline static const std::string kImageWidthStr{ "width" };
	inline static const std::string kImageHeightStr{ "height" };
	inline static const std::string kBucketSizeStr{ "bucket_size" };
	inline static const std::string kCameraStr{ "camera" };
	inline static const std::string kMatrixStr{ "matrix" };
	inline static const std::string kLightsStr{ "lights" };
	inline static const std::string kIntensityStr{ "intensity" };
	inline static const std::string kPositionStr{ "position" };
	inline static const std::string kObjectsStr{ "objects" };
	inline static const std::string kVerticesStr{ "vertices" };
	inline static const std::string kUVsStr{ "uvs" };
	inline static const std::string kTrianglesStr{ "triangles" };
	inline static const std::string kMaterialsStr{ "materials" };
	inline static const std::string kTypeStr{ "type" };
	inline static const std::string kTypeConstantStr{ "constant" };
	inline static const std::string kTypeDiffuseStr{ "diffuse" };
	inline static const std::string kTypeReflectiveStr{ "reflective" };
	inline static const std::string kTypeRefractiveStr{ "refractive" };
	inline static const std::string kAlbedoStr{ "albedo" };
	inline static const std::string kIorStr{ "ior" };
	inline static const std::string kSmoothShadingStr{ "smooth_shading" };
	inline static const std::string kMaterialIndexStr{ "material_index" };

	inline static const std::string kTexturesStr{ "textures" };
	inline static const std::string kTexturesNameStr{ "name" };
	inline static const std::string kTexturesTypeStr{ "type" };
	inline static const std::string kTexturesAlbedoStr{ "albedo" };
	inline static const std::string kTexturesEdgeColorStr{ "edge_color" };
	inline static const std::string kTexturesInnerColorStr{ "inner_color" };
	inline static const std::string kTexturesEdgeWidthStr{ "edge_width" };
	inline static const std::string kTexturesColorAStr{ "color_A" };
	inline static const std::string kTexturesColorBStr{ "color_B" };
	inline static const std::string kTexturesSquareSizeStr{ "square_size" };
	inline static const std::string kTexturesFilePathStr{ "file_path" };

	const std::map<std::string, Material::Type> materialTypeMap = {
		{ kTypeConstantStr, Material::Type::CONSTANT},
		{ kTypeDiffuseStr, Material::Type::DIFFUSE},
		{ kTypeReflectiveStr, Material::Type::REFLECTIVE},
		{ kTypeRefractiveStr, Material::Type::REFRACTIVE},
	};
	
	void parseSceneFile(const std::string& fileName)
	{
		using namespace rapidjson;
		Document doc = getJsonDocument(fileName);
		settings.sceneName = fileName;

		const Value& settingsVal = doc.FindMember(kSceneSettingsStr.c_str())->value;
		if (!settingsVal.IsNull() && settingsVal.IsObject())
		{
			const Value& bgColorVal = settingsVal.FindMember(kBackgroundColorStr.c_str())->value;
			assert(!bgColorVal.IsNull() && bgColorVal.IsArray());
			settings.backgroundColor = loadVector(bgColorVal.GetArray());

			const Value& imageSettingsVal = settingsVal.FindMember(kImageSettingsStr.c_str())->value;
			if (!imageSettingsVal.IsNull() && imageSettingsVal.IsObject())
			{
				const Value& imageWidthVal = imageSettingsVal.FindMember(kImageWidthStr.c_str())->value;
				const Value& imageHeightVal = imageSettingsVal.FindMember(kImageHeightStr.c_str())->value;
				assert(!imageWidthVal.IsNull() && imageWidthVal.IsInt() && !imageHeightVal.IsNull() && imageHeightVal.IsInt());
				settings.imageSettings.width = imageWidthVal.GetInt();
				settings.imageSettings.height = imageHeightVal.GetInt();
			}
			
			if (settingsVal.HasMember(kBucketSizeStr.c_str()))
			{
				const Value& bucketSizeVal = settingsVal.FindMember(kBucketSizeStr.c_str())->value;
				assert(!bucketSizeVal.IsNull() && bucketSizeVal.IsInt());
				settings.bucketSize = bucketSizeVal.GetInt();
			}
		}

		const Value& cameraVal = doc.FindMember(kCameraStr.c_str())->value;
		if (!cameraVal.IsNull() && cameraVal.IsObject())
		{
			const Value& matrixVal = cameraVal.FindMember(kMatrixStr.c_str())->value;
			assert(!matrixVal.IsNull() && matrixVal.IsArray());
			Matrix4 rotation = loadMatrix(matrixVal.GetArray());

			const Value& positionVal = cameraVal.FindMember(kPositionStr.c_str())->value;
			assert(!positionVal.IsNull() && positionVal.IsArray());
			Matrix4 translation = MakeTranslation(loadVector(positionVal.GetArray()));

			camera.transform =  translation * rotation;
		}

		const Value& lightsValue = doc.FindMember(kLightsStr.c_str())->value;
		if (!lightsValue.IsNull() && lightsValue.IsArray())
		{
			for (Value::ConstValueIterator it = lightsValue.Begin(); it != lightsValue.End(); ++it)
			{
				Light light;
				const Value& intensityValue = it->FindMember(kIntensityStr.c_str())->value;
				assert(!intensityValue.IsNull() && intensityValue.IsInt());
				light.intensity = static_cast<float>(intensityValue.GetInt()) * 0.1f; // lights seems to be too bright

				const Value& positionVal = it->FindMember(kPositionStr.c_str())->value;
				assert(!positionVal.IsNull() && positionVal.IsArray());
				light.position = loadVector(positionVal.GetArray());

				lights.push_back(light);
			}
		}

		const Value& objectsValue = doc.FindMember(kObjectsStr.c_str())->value;
		if(!objectsValue.IsNull() && objectsValue.IsArray()) 
		{
			for(Value::ConstValueIterator it = objectsValue.Begin(); it != objectsValue.End(); ++it)
			{
				Mesh mesh;

				const Value& verticesValue = it->FindMember(kVerticesStr.c_str())->value;
				assert(!verticesValue.IsNull() && verticesValue.IsArray());
				std::vector<Vector3> vertices = loadVertices(verticesValue.GetArray());

				std::vector<Vector2> uvs;
				if (it->HasMember(kUVsStr.c_str()))
				{
					const Value& uvsValue = it->FindMember(kUVsStr.c_str())->value;
					assert(!uvsValue.IsNull() && uvsValue.IsArray());
					uvs = loadUVs(uvsValue.GetArray());
				}

				const Value& trianglesValue = it->FindMember(kTrianglesStr.c_str())->value;
				assert(!trianglesValue.IsNull() && trianglesValue.IsArray());
				std::vector<uint32_t> indices = loadIndices(trianglesValue.GetArray());

				// Compute vertex normals
				std::vector<Vector3> vertexNormals(vertices.size(), { 0.0f, 0.0f, 0.0f });
				for (uint32_t i = 0; i < indices.size(); i += 3)
				{
					const auto& i0 = indices[i];
					const auto& i1 = indices[i + 1];
					const auto& i2 = indices[i + 2];
					const auto& v0 = vertices[i0];
					const auto& v1 = vertices[i1];
					const auto& v2 = vertices[i2];
					Vector3 faceNormal = Normalize(Cross(v1 - v0, v2 - v0));

					vertexNormals[i0] += faceNormal;
					vertexNormals[i1] += faceNormal;
					vertexNormals[i2] += faceNormal;
				}
				// Normalize
				for (uint32_t i = 0; i < vertexNormals.size(); ++i)
					vertexNormals[i] = Normalize(vertexNormals[i]);


				mesh.triangles.reserve(indices.size() / 3);
				for (uint32_t i = 0; i < indices.size(); i += 3)
				{
					const auto& i0 = indices[i];
					const auto& i1 = indices[i + 1];
					const auto& i2 = indices[i + 2];

					const auto& v0 = vertices[i0];
					const auto& v1 = vertices[i1];
					const auto& v2 = vertices[i2];

					const auto& n0 = vertexNormals[i0];
					const auto& n1 = vertexNormals[i1];
					const auto& n2 = vertexNormals[i2];

					const auto& uv0 = !uvs.empty() ? uvs[i0] : 1.f;
					const auto& uv1 = !uvs.empty() ? uvs[i1] : 1.f;
					const auto& uv2 = !uvs.empty() ? uvs[i2] : 1.f;

					mesh.triangles.emplace_back(
						Vertex{v0, n0, uv0},
						Vertex{v1, n1, uv1},
						Vertex{v2, n2, uv2}
					);
				}

				const Value& materialIndexValue = it->FindMember(kMaterialIndexStr.c_str())->value;
				assert(!materialIndexValue.IsNull() && materialIndexValue.IsInt());
				mesh.materialIndex = materialIndexValue.GetInt();

				meshes.push_back(mesh);
			}
		}

		// Load textures
		const Value& texturesValue = doc.FindMember(kTexturesStr.c_str())->value;
		if (!texturesValue.IsNull() && texturesValue.IsArray())
		{
			for (Value::ConstValueIterator it = texturesValue.Begin(); it != texturesValue.End(); ++it)
			{
				const Value& nameValue = it->FindMember(kTexturesNameStr.c_str())->value;
				assert(!nameValue.IsNull() && nameValue.IsString());
				std::string name = std::string(nameValue.GetString());
				
				const Value& typeValue = it->FindMember(kTexturesTypeStr.c_str())->value;
				assert(!typeValue.IsNull() && typeValue.IsString());
				std::string type = std::string(typeValue.GetString());

				if (type == "albedo")
				{
					const Value& albedoValue = it->FindMember(kTexturesAlbedoStr.c_str())->value;
					assert(!albedoValue.IsNull() && albedoValue.IsArray());
					Vector3 albedo = loadVector(albedoValue.GetArray());
					textures.emplace(name, std::make_shared<const AlbedoTexture>(name, std::move(albedo)));
				}
				else if (type == "edges")
				{
					const Value& edgeColorValue = it->FindMember(kTexturesEdgeColorStr.c_str())->value;
					assert(!edgeColorValue.IsNull() && edgeColorValue.IsArray());
					Vector3 edgeColor = loadVector(edgeColorValue.GetArray());

					const Value& innerColorValue = it->FindMember(kTexturesInnerColorStr.c_str())->value;
					assert(!innerColorValue.IsNull() && innerColorValue.IsArray());
					Vector3 innerColor = loadVector(innerColorValue.GetArray());

					const Value& edgeWidthValue = it->FindMember(kTexturesEdgeWidthStr.c_str())->value;
					assert(!edgeWidthValue.IsNull() && edgeWidthValue.IsFloat());
					float edgeWidth = edgeWidthValue.GetFloat();

					textures.emplace(name, std::make_shared<const EdgesTexture>(name, std::move(edgeColor), std::move(innerColor), edgeWidth));
				}
				else if (type == "checker")
				{
					const Value& colorAValue = it->FindMember(kTexturesColorAStr.c_str())->value;
					assert(!colorAValue.IsNull() && colorAValue.IsArray());
					Vector3 colorA = loadVector(colorAValue.GetArray());

					const Value& colorBValue = it->FindMember(kTexturesColorBStr.c_str())->value;
					assert(!colorBValue.IsNull() && colorBValue.IsArray());
					Vector3 colorB = loadVector(colorBValue.GetArray());

					const Value& squareSizeValue = it->FindMember(kTexturesSquareSizeStr.c_str())->value;
					assert(!squareSizeValue.IsNull() && squareSizeValue.IsFloat());
					float squareSize = squareSizeValue.GetFloat();

					textures.emplace(name, std::make_shared<const CheckerTexture>(name, std::move(colorA), std::move(colorB), squareSize));
				}
				else if (type == "bitmap")
				{
					const Value& pathValue = it->FindMember(kTexturesFilePathStr.c_str())->value;
					assert(!pathValue.IsNull() && pathValue.IsString());
					std::string path = std::string(pathValue.GetString());

					if (!path.empty() && path[0] == '/')
						path.erase(0, 1);

					textures.emplace(name, std::make_shared<const BitmapTexture>(name, path));
				}
				else
				{
					std::cout << "Invalid texture type." << std::endl;
				}
			}
		}

		// Load materials
		const Value& materialsValue = doc.FindMember(kMaterialsStr.c_str())->value;
		if (!materialsValue.IsNull() && materialsValue.IsArray())
		{
			for (Value::ConstValueIterator it = materialsValue.Begin(); it != materialsValue.End(); ++it)
			{
				Material material;

				const Value& typeValue = it->FindMember(kTypeStr.c_str())->value;
				assert(!typeValue.IsNull() && typeValue.IsString());
				const auto typeStr = std::string(typeValue.GetString());
				material.type = materialTypeMap.at(typeStr);
				if (material.type == Material::Type::REFRACTIVE)
				{
					const Value& iorVal = it->FindMember(kIorStr.c_str())->value;
					assert(!iorVal.IsNull() && iorVal.IsFloat());
					material.ior = iorVal.GetFloat();

				} else {
					const Value& albedoVal = it->FindMember(kAlbedoStr.c_str())->value;
					assert(!albedoVal.IsNull());
					if (albedoVal.IsArray())
					{
						material.SetAlbedo(loadVector(albedoVal.GetArray()));
					}
					else if (albedoVal.IsString())
					{
						const auto typeStr = std::string(typeValue.GetString());
						std::string textureName = std::string(albedoVal.GetString());
						auto it = textures.find(textureName);
						if (it != textures.end())
							material.texture = it->second;
					}
					else
					{
						std::cout << "Invalid material" << std::endl;
					}
				}

				const Value& smoothShadingVal = it->FindMember(kSmoothShadingStr.c_str())->value;
				assert(!smoothShadingVal.IsNull() && smoothShadingVal.IsBool());
				material.smoothShading = smoothShadingVal.GetBool();

				materials.push_back(material);
			}
		}

	}

	rapidjson::Document getJsonDocument(const std::string& fileName)
	{
		using namespace rapidjson;

		std::ifstream ifs(fileName);
		assert(ifs.is_open());

		IStreamWrapper isw(ifs);
		Document doc;
		doc.ParseStream(isw);

		if (doc.HasParseError())
		{
			std::cout << "Error : " << doc.GetParseError() << '\n';
			std::cout << "Offset : " << doc.GetErrorOffset() << '\n';
			assert(false);
		}
		assert(doc.IsObject());

		return doc;	// RVO
	}
};