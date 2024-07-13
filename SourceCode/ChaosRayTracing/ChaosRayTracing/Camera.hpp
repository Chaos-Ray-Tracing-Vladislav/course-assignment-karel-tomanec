#pragma once

#include "Math3D.hpp"

class Camera
{
public:
	Camera() = default;

	Matrix4 transform = Matrix4::Identity();

	Point3 GetPosition() const
	{
		return transform.GetTranslation();
	}

	Vector3 GetLookDirection() const
	{
		return Normalize(transform * Vector3(0.f, 0.f, -1.f));
	}
};