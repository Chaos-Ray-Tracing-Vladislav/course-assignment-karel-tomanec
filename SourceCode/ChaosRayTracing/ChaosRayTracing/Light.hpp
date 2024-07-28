#pragma once

#include "Math3D.hpp"

inline float PowerHeuristic(int nf, float fPdf, int ng, float gPdf)
{
    float f = nf * fPdf;
    float g = ng * gPdf;
    return (f * f) / (f * f + g * g);
}

struct LightSample
{
	Vector3 position;
	Vector3 Le;
    float pdf;
};

struct Light
{
	float intensity;
	Vector3 position;
};

struct EmissiveTriangle
{
	Triangle triangle;
	Vector3 emission;

    LightSample sample(const Vector3& posW, const Vector2& rnd) const
	{
        float u = rnd.x;
        float v = rnd.y;

        if (u + v > 1.0f) {
            u = 1.0f - u;
            v = 1.0f - v;
        }

        float w = 1.0f - u - v;

        Vector3 sampledPosition = {
            u * triangle.v0.position.x + v * triangle.v1.position.x + w * triangle.v2.position.x,
            u * triangle.v0.position.y + v * triangle.v1.position.y + w * triangle.v2.position.y,
            u * triangle.v0.position.z + v * triangle.v1.position.z + w * triangle.v2.position.z
        };

        Vector3 toLight = sampledPosition - posW;
        const float distSqr = std::max(FLT_MIN, Dot(toLight, toLight));

        float area = triangle.Area();

        float cosTheta = Dot(triangle.faceNormal, -toLight);


        float pdf = distSqr / (cosTheta * area);

        LightSample sample;
        sample.position = sampledPosition;
        sample.Le = emission;
        sample.pdf = pdf;
        return sample;
    }
};