#pragma once
#include <functional>
#include <mutex>
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
	uint8_t splitAxis;

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

	HitInfo closestHit(const std::vector<Triangle>& triangles, Ray& ray) const
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
						ray.maxT = hitInfo.t;
					}
				}
				return false;
			};
		return traverse(ray, closestHitFunc);
	}

	bool anyHit(const std::vector<Triangle>& triangles, const std::vector<Material>& materials, Ray& ray) const
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

	HitInfo traverse(Ray& ray, const std::function<bool(HitInfo&, uint32_t, uint32_t)>& hitFunction) const
	{
		HitInfo hitInfo;
		if (nodes.empty())
			return hitInfo;


		bool dirIsNegative[3] = { ray.directionN.x < 0.f, ray.directionN.y < 0.f, ray.directionN.z < 0.f };

		// Traversal
		std::vector<uint32_t> nodesToTraverse;
		nodesToTraverse.reserve(16);
		// Insert root node index
		nodesToTraverse.push_back(0);
		// Traverse the tree
		while(!nodesToTraverse.empty())
		{
			const uint32_t nodeIndex = nodesToTraverse.back();
			nodesToTraverse.pop_back();
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
					if(dirIsNegative[node.splitAxis])
					{
						nodesToTraverse.push_back(nodeIndex + 1);
						nodesToTraverse.push_back(node.secondChildOffset);
					}
					else
					{
						nodesToTraverse.push_back(node.secondChildOffset);
						nodesToTraverse.push_back(nodeIndex + 1);
					}
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
			Vector3 extent = boundingBox.extent();    // Find the axis with the largest extent
			uint8_t splitAxis = static_cast<uint8_t>(std::distance(std::begin(extent.data), std::max_element(std::begin(extent.data), std::end(extent.data))));
			std::sort(triangles.begin() + range.start, triangles.begin() + range.end, [splitAxis](const Triangle& triA, const Triangle& triB)
				{
					return triA.Centroid()[splitAxis] < triB.Centroid()[splitAxis];
				});
			BVHNode interiorNode{
				.boundingBox = boundingBox,
				.primitiveCount = 0,
				.splitAxis = splitAxis
			};
			uint32_t interiorNodeIndex = static_cast<uint32_t>(nodes.size());
			nodes.emplace_back(interiorNode);
			build(triangles, Range{ range.start, mid }, depth + 1);
			nodes[interiorNodeIndex].secondChildOffset = static_cast<uint32_t>(nodes.size());
			build(triangles, Range{ mid, range.end }, depth + 1);
		}
	}

	std::vector<BVHNode> nodes;
	static constexpr uint32_t maxDepth = 10;
	static constexpr uint32_t maxTriangleCountPerLeaf = 4;
};