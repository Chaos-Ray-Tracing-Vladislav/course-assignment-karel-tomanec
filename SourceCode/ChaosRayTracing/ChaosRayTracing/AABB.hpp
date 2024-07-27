#pragma once
#include <vector>

#include "Math3D.hpp"

class AABB 
{
public:
	Vector3 minPoint = Vector3(std::numeric_limits<float>::max());
	Vector3 maxPoint = Vector3(-std::numeric_limits<float>::max());

	AABB() = default;

	AABB(Vector3 minPoint, Vector3 maxPoint) : minPoint(minPoint), maxPoint(maxPoint) {}

	AABB(const Triangle& triangle)
	{
		minPoint = min(minPoint, min(triangle.v0.position, min(triangle.v1.position, triangle.v2.position)));
		maxPoint = max(maxPoint, max(triangle.v0.position, max(triangle.v1.position, triangle.v2.position)));
	}

	AABB(const std::vector<Triangle>& triangles, Range range)
	{
		AABB resAABB;

		auto start = triangles.begin() + range.start;
		auto end = triangles.begin() + range.end;

		std::for_each(start, end, [&resAABB](const auto& triangle)
			{
				AABB triAABB(triangle);
				resAABB |= triAABB;
			});

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

	bool overlaps(AABB b) const
	{
		b.intersection(*this);
		return b.isValid() && b.volume() > 0.f;
	}

	AABB& operator|=(const AABB& rhs) { return include(rhs); }

	bool intersect(const Ray& ray) const {
		float t1 = (minPoint.x - ray.origin.x) / ray.directionN.x;
		float t2 = (maxPoint.x - ray.origin.x) / ray.directionN.x;
		float t3 = (minPoint.y - ray.origin.y) / ray.directionN.y;
		float t4 = (maxPoint.y - ray.origin.y) / ray.directionN.y;
		float t5 = (minPoint.z - ray.origin.z) / ray.directionN.z;
		float t6 = (maxPoint.z - ray.origin.z) / ray.directionN.z;

		float tmin = std::max({std::min(t1, t2), std::min(t3, t4), std::min(t5, t6)});
		float tmax = std::min({std::max(t1, t2), std::max(t3, t4), std::max(t5, t6)});

		return tmax >= tmin && tmax >= 0;
	}
};