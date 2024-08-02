#pragma once

#include <memory>

#include "Math3D.hpp"

class Texture;

class Material
{
public:
	enum Type
	{
		CONSTANT,
		DIFFUSE,
		REFLECTIVE,
		REFRACTIVE,
		EMISSIVE
	};

	Type type;
	float ior;
	bool smoothShading = false;
	std::shared_ptr<const Texture> texture;
	Vector3 emission{0.f};

	void setAlbedo(Vector3 albedo)
	{
		this->albedo = albedo;
	}

	Vector3 getAlbedo(const Vector2& barycentrics, const Vector2& uv) const;

	bool cullBackFace() const
	{
		if (type == REFRACTIVE)
			return false;
		return true;
	}

private:
	Vector3 albedo{1.f};
};
