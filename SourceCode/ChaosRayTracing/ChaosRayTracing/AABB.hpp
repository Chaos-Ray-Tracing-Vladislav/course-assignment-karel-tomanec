#pragma once
#include <vector>

#include "Math3D.hpp"

class AABB 
{
public:
	Vector3 minPoint = Vector3(std::numeric_limits<float>::infinity());
	Vector3 maxPoint = Vector3(-std::numeric_limits<float>::infinity());

	AABB() = default;

	AABB(Vector3 minPoint, Vector3 maxPoint) : minPoint(std::move(minPoint)), maxPoint(std::move(maxPoint)) {}

	AABB(const Triangle& triangle)
	{
		minPoint = min(minPoint, min(triangle.v0.position, min(triangle.v1.position, triangle.v2.position)));
		minPoint = max(minPoint, max(triangle.v0.position, max(triangle.v1.position, triangle.v2.position)));
	}

	AABB(const std::vector<Triangle>& triangles)
	{
		AABB resAABB;
		for (const auto& triangle : triangles)
		{
			AABB triAABB(triangle);
			resAABB |= triAABB;
		}

		minPoint = resAABB.minPoint;
		maxPoint = resAABB.maxPoint;
	}

	bool isValid() const
	{
		return maxPoint.x >= minPoint.x && maxPoint.y >= minPoint.y && maxPoint.z >= minPoint.z;
	}

	Vector3 center() const 
	{ 
		return (minPoint + maxPoint) * 0.5f; 
	}

	Vector3 extent() const 
	{ 
		return maxPoint - minPoint; 
	}

	float area() const
	{
		Vector3 e = extent();
		return (e.x * e.y + e.x * e.z + e.y * e.z) * 2.f;
	}

	float volume() const
	{
		Vector3 e = extent();
		return e.x * e.y * e.z;
	}

	AABB& include(const AABB& other)
	{
		minPoint = min(minPoint, other.minPoint);
		maxPoint = max(maxPoint, other.maxPoint);
		return *this;
	}

	AABB& intersection(const AABB& b)
	{
		minPoint = max(minPoint, b.minPoint);
		maxPoint = min(maxPoint, b.maxPoint);
		return *this;
	}

	bool overlaps(AABB b)
	{
		b.intersection(*this);
		return b.isValid() && b.volume() > 0.f;
	}

	AABB& operator|=(const AABB& rhs) { return include(rhs); }
};