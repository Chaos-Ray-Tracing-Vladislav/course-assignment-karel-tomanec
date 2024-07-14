#pragma once

#include "Math3D.hpp"

class Texture 
{
public:

	Texture(std::string name) : name(std::move(name)) {}

	virtual ~Texture() = default;

	virtual Vector3 GetColor(const Vector2& uv) const = 0;

	std::string name;
};

class AlbedoTexture : public Texture 
{
public:

	AlbedoTexture(std::string name, Vector3 albedo) : Texture(std::move(name)), albedo(std::move(albedo)) {}

	Vector3 GetColor(const Vector2& uv) const override { return albedo; }

private:

	Vector3 albedo;
};

class EdgesTexture : public Texture 
{
public:

	EdgesTexture(std::string name, Vector3 edgeColor, Vector3 innerColor, float edgeWidth) 
		: Texture(std::move(name)), edgeColor(std::move(edgeColor)), innerColor(std::move(innerColor)), edgeWidth(edgeWidth)
	{ }

	Vector3 GetColor(const Vector2& uv) const override 
	{ 
		const float& u = uv.x;
		const float& v = uv.y;
		if (u < edgeWidth || v < edgeWidth)
			return edgeColor;

		if (1.f - u - v < edgeWidth)
			return edgeColor;

		return innerColor;
	}

private:

	Vector3 edgeColor;
	Vector3 innerColor;
	float edgeWidth;
};

class CheckerTexture : public Texture 
{
public:

	CheckerTexture(std::string name, Vector3 colorA, Vector3 colorB, float squareSize) 
		: Texture(std::move(name)), colorA(std::move(colorA)), colorB(std::move(colorB)), squareSize(squareSize)
	{ 
		numSquares = 1.f / squareSize;
	}

	Vector3 GetColor(const Vector2& uv) const override 
	{ 
		const float& u = uv.x;
		const float& v = uv.y;
		int uIndex = static_cast<int>(u * numSquares);
		int vIndex = static_cast<int>(v * numSquares);

		if (uIndex % 2 == vIndex % 2)
			return colorA;

		return colorB;
	}

private:
	Vector3 colorA;
	Vector3 colorB;
	float squareSize;
	float numSquares;
};

class BitmapTexture : public Texture 
{
public:
	BitmapTexture(std::string name, std::string filePath) 
		: Texture(std::move(name)), filePath(std::move(filePath))
	{ }

	Vector3 GetColor(const Vector2& uv) const override { return {}; }
	
private:
	std::string filePath;
};