#pragma once
#include <vector>

#include "Math3D.hpp"
#include "AABB.hpp"

struct BVHNode 
{
	AABB boundingBox;
	union {
		uint32_t primitivesOffset; // leaf;
		uint32_t secondChildOffset; // interior
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
		AABB boundingBox{ triangles, range };
		if (depth >= maxDepth || range.count() <= maxTriangleCountPerLeaf)
		{
			// Create leaf node
			BVHNode leafNode{
				.boundingBox = boundingBox,
				.primitivesOffset = range.start,
				.primitiveCount = range.count()
			};
			nodes.emplace_back(leafNode);
		}
		else
		{
			uint32_t mid = (range.start + range.end) / 2;
			uint32_t splitAxis = 0;
			std::sort(triangles.begin() + range.start, triangles.begin() + range.end, [splitAxis](const Triangle& triA, const Triangle& triB)
				{
					triA.Centroid()[splitAxis] < triB.Centroid()[splitAxis];
				});
			BVHNode interiorNode{
				.boundingBox = boundingBox,
				.primitiveCount = 0
			};
			uint32_t interiorNodeIndex = nodes.size();
			nodes.emplace_back(interiorNode);
			build(triangles, Range{ range.start, mid }, depth + 1);
			nodes[interiorNodeIndex].secondChildOffset = nodes.size();
			build(triangles, Range{ mid + 1, range.end }, depth + 1);
		}
	}

	std::vector<BVHNode> nodes;
	const uint32_t maxDepth = 10;
	const uint32_t maxTriangleCountPerLeaf = 10;
};