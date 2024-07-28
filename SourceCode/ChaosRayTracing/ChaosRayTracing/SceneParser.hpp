#pragma once

#include <string>

#define RAPIDJSON_NOMEMBERITERATORCLASS
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"


class Scene;

class SceneParser final
{
private:

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
	inline static const std::string kTypeEmissiveStr{ "emissive" };
	inline static const std::string kAlbedoStr{ "albedo" };
	inline static const std::string kEmissionStr{ "emission" };
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

	static rapidjson::Document getJsonDocument(const std::string& fileName);

	Scene& scene;

public:

	SceneParser(Scene& scene) : scene(scene) {}

	void parseSceneFile(const std::string& fileName) const;
};
