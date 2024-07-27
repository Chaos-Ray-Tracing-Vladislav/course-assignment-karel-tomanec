#pragma once
#include <functional>
#include <stack>
#include <vector>

#include "AABB.hpp"
#include "Material.hpp"

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
		Range range{ 0, static_cast<uint32_t>(triangles.size()) };
		build(triangles, range, 0);
	}

	HitInfo closestHit(const std::vector<Triangle>& triangles, const Ray& ray) const
	{
		std::function closestHitFunc = [&triangles, &ray](HitInfo& hitInfo, uint32_t trianglesStart, uint32_t trianglesEnd)
			{
				for (uint32_t triangleIndex = trianglesStart; triangleIndex < trianglesEnd; ++triangleIndex)
				{
					const auto& triangle = triangles[triangleIndex];
					HitInfo currHitInfo = triangle.Intersect(ray);

					if (currHitInfo.hit && currHitInfo.t < hitInfo.t)
					{
						currHitInfo.triangleIndex = triangleIndex;
						hitInfo = currHitInfo;
					}
				}
				return false;
			};
		return traverse(ray, closestHitFunc);
	}

	bool anyHit(const std::vector<Triangle>& triangles, const std::vector<Material>& materials, const Ray& ray) const
	{
		std::function anyHitFunc = [&triangles, &materials, &ray](HitInfo& hitInfo, uint32_t trianglesStart, uint32_t trianglesEnd)
			{
				for (uint32_t triangleIndex = trianglesStart; triangleIndex < trianglesEnd; ++triangleIndex)
				{
					const auto& triangle = triangles[triangleIndex];
					HitInfo currHitInfo = triangle.Intersect(ray);

					if (currHitInfo.hit)
					{
						const auto& material = materials[triangle.materialIndex];
						if (material.type != Material::Type::REFRACTIVE)
						{
							hitInfo.hit = true;
							return true;
						}
					}
				}
				return false;
			};
		HitInfo hitInfo = traverse(ray, anyHitFunc);
		return hitInfo.hit;
	}

	HitInfo traverse(const Ray& ray, std::function<bool(HitInfo&, uint32_t, uint32_t)> hitFunction) const
	{
		HitInfo hitInfo;
		if (nodes.empty())
			return hitInfo;

		// Traversal
		std::stack<uint32_t> nodesToTraverse;
		// Insert root node index
		nodesToTraverse.emplace(0);
		// Traverse the tree
		while(!nodesToTraverse.empty())
		{
			const uint32_t nodeIndex = nodesToTraverse.top();
			nodesToTraverse.pop();
			const BVHNode& node = nodes[nodeIndex];
			if(node.boundingBox.intersect(ray))
			{
				if(node.isLeaf())
				{
					uint32_t trianglesOffset = node.primitivesOffset;
					uint32_t trianglesCount = node.primitiveCount;
					if (hitFunction(hitInfo, trianglesOffset, trianglesOffset + trianglesCount))
						return hitInfo;
				}
				else
				{
					// TODO: decide the order
					nodesToTraverse.emplace(nodeIndex + 1);
					nodesToTraverse.emplace(node.secondChildOffset);
				}
			}
		}

		return hitInfo;
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
				.primitiveCount = static_cast<uint16_t>(range.count())
			};
			nodes.emplace_back(leafNode);
		}
		else
		{
			uint32_t mid = (range.start + range.end) / 2;
			uint32_t splitAxis = 0;
			std::sort(triangles.begin() + range.start, triangles.begin() + range.end, [splitAxis](const Triangle& triA, const Triangle& triB)
				{
					return triA.Centroid()[splitAxis] < triB.Centroid()[splitAxis];
				});
			BVHNode interiorNode{
				.boundingBox = boundingBox,
				.primitiveCount = 0
			};
			uint32_t interiorNodeIndex = nodes.size();
			nodes.emplace_back(interiorNode);
			build(triangles, Range{ range.start, mid }, depth + 1);
			nodes[interiorNodeIndex].secondChildOffset = nodes.size();
			build(triangles, Range{ mid, range.end }, depth + 1);
		}
	}

	std::vector<BVHNode> nodes;
	static constexpr uint32_t maxDepth = 10;
	static constexpr uint32_t maxTriangleCountPerLeaf = 10;
};