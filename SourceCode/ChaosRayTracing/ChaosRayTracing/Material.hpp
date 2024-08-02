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

	Vector3 GetAlbedo(const Vector2& barycentrics, const Vector2& uv) const;

private:

	Vector3 albedo{ 1.f };
};
