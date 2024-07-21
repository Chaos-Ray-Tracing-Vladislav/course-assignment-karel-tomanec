#pragma once
#include <vector>

#include "Math3D.hpp"
#include "AABB.hpp"

struct BVHNode 
{
	AABB boundingBox;
	union {
		int primitivesOffset; // leaf;
		int secondChildOffset; // interior
	};
	uint16_t primitiveCount; // 0 -> interior node
	uint8_t axis; // interior node: xyz

	bool isLeaf() const
	{
		return primitiveCount != 0;
	}
};


class BVH 
{
public:

	BVH() = default;

	BVH(std::vector<Triangle>& triangles)
	{
		Range range{ 0, triangles.size() };
		build(triangles, range, 0);
	}

private:

	void build(std::vector<Triangle>& triangles, Range range, uint32_t depth)
	{
		if (depth >= maxDepth || range.count() <= maxTriangleCountPerLeaf)
		{
			// Create leaf node
			BVHNode leafNode{
				.boundingBox = AABB(triangles, range),
				.primitivesOffset = range.start,
				.primitiveCount = range.count()
			};
		}
		else
		{
			uint32_t mid = (range.start + range.end) / 2;
		}
	}

	std::vector<BVHNode> nodes;
	const uint32_t maxDepth = 10;
	const uint32_t maxTriangleCountPerLeaf = 10;
};