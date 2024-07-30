#pragma once

#include "Light.hpp"

#include <vector>
#include <algorithm>
#include <functional>
#include <optional>

class EmissiveSampler
{
public:
	std::optional<EmissiveLightSample> sample(const Vector3 posW, const Vector3& rnd) const
	{
		if (emissiveTriangles.empty())
			return std::nullopt;

		size_t emissiveIndex = static_cast<size_t>(rnd.x * emissiveTriangles.size());
		emissiveIndex = std::min(emissiveIndex, emissiveTriangles.size() - 1);

		EmissiveLightSample sample = emissiveTriangles[emissiveIndex].sample(posW, rnd.yz());

		sample.pdf *= 1.f / static_cast<float>(emissiveTriangles.size());

		return sample;
	}

	float evalPdf(size_t emissiveTriangleIndex, const Vector3& posW, const Vector3& sampledPosition) const
	{
		return emissiveTriangles[emissiveTriangleIndex].pdf(posW, sampledPosition) / static_cast<float>(
			emissiveTriangles.size());
	}

	std::vector<EmissiveTriangle> emissiveTriangles;
};
