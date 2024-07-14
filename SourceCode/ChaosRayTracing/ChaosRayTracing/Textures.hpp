#pragma once

#include "Math3D.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

class Texture 
{
public:

	Texture(std::string name) : name(std::move(name)) {}

	virtual ~Texture() = default;

	virtual Vector3 GetColor(const Vector2& barycentrics, const Vector2& uv) const = 0;

	std::string name;
};

class AlbedoTexture : public Texture 
{
public:

	AlbedoTexture(std::string name, Vector3 albedo) : Texture(std::move(name)), albedo(std::move(albedo)) {}

	Vector3 GetColor(const Vector2& barycentrics, const Vector2& uv) const override { return albedo; }

private:

	Vector3 albedo;
};

class EdgesTexture : public Texture 
{
public:

	EdgesTexture(std::string name, Vector3 edgeColor, Vector3 innerColor, float edgeWidth) 
		: Texture(std::move(name)), edgeColor(std::move(edgeColor)), innerColor(std::move(innerColor)), edgeWidth(edgeWidth)
	{ }

	Vector3 GetColor(const Vector2& barycentrics, const Vector2& uv) const override 
	{ 
		if (barycentrics.x < edgeWidth || barycentrics.y < edgeWidth)
			return edgeColor;

		if (1.f - barycentrics.x - barycentrics.y < edgeWidth)
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

	Vector3 GetColor(const Vector2& barycentrics, const Vector2& uv) const override 
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
	BitmapTexture(std::string name, const std::string& filePath) 
		: Texture(std::move(name))
	{
		LoadImageTexture(filePath);
	}

	Vector3 GetColor(const Vector2& barycentrics, const Vector2& uv) const override 
	{ 
		assert(image);

		int x = uv.x * width;
		int y = (1.f - uv.y) * height;

		unsigned char *pixel = image + (y * width + x) * channels;

		unsigned char r = pixel[0];
		unsigned char g = pixel[1];
		unsigned char b = pixel[2];

		return { static_cast<float>(r) / 255.f, static_cast<float>(g) / 255.f, static_cast<float>(b) / 255.f };
	}

	~BitmapTexture() override
	{
		stbi_image_free(image);
	}
	
private:

	void LoadImageTexture(const std::string& filePath)
	{
		image = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
	}
	
	int width;
	int height;
	int channels;
	unsigned char* image = nullptr;
};