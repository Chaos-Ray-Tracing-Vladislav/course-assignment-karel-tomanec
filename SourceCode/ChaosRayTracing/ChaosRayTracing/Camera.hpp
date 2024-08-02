#pragma once

#include "Math3D.hpp"

class Camera
{
public:
	Camera() = default;

	Matrix4 transform = Matrix4::identity();

	Point3 getPosition() const
	{
		return transform.getTranslation();
	}

	Vector3 getLookDirection() const
	{
		return Normalize(transform * Vector3(0.f, 0.f, -1.f));
	}
};
