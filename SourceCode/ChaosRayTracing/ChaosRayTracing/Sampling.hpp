#pragma once

#include "Math3D.hpp"

namespace Sampling
{
	inline float powerHeuristic(float fPdf, float gPdf)
	{
		float f = fPdf;
		float g = gPdf;
		return (f * f) / (f * f + g * g);
	}

	struct RandomSampler
	{
		std::random_device rd;
		std::mt19937 gen{rd()};
		std::uniform_real_distribution<float> dis{0.0f, 1.0f};

		float next1D()
		{
			return dis(gen);
		}

		Vector2 next2D()
		{
			return {dis(gen), dis(gen)};
		}

		Vector3 next3D()
		{
			return {dis(gen), dis(gen), dis(gen)};
		}
	};
}