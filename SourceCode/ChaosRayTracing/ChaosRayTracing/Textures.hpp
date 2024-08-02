#pragma once

#include "Math3D.hpp"

class Texture
{
public:
	Texture(std::string name) : name(std::move(name))
	{
	}

	virtual ~Texture() = default;

	virtual Vector3 GetColor(const Vector2& barycentrics, const Vector2& uv) const = 0;

	std::string name;
};

class AlbedoTexture : public Texture
{
public:
	AlbedoTexture(std::string name, Vector3 albedo) : Texture(std::move(name)), albedo(albedo)
	{
	}

	Vector3 GetColor(const Vector2& barycentrics, const Vector2& uv) const override { return albedo; }

private:
	Vector3 albedo;
};

class EdgesTexture : public Texture
{
public:
	EdgesTexture(std::string name, Vector3 edgeColor, Vector3 innerColor, float edgeWidth)
		: Texture(std::move(name)), edgeColor(edgeColor), innerColor(innerColor), edgeWidth(edgeWidth)
	{
	}

	Vector3 GetColor(const Vector2& barycentrics, const Vector2& uv) const override;

private:
	Vector3 edgeColor;
	Vector3 innerColor;
	float edgeWidth;
};

class CheckerTexture : public Texture
{
public:
	CheckerTexture(std::string name, Vector3 colorA, Vector3 colorB, float squareSize)
		: Texture(std::move(name)), colorA(colorA), colorB(colorB), squareSize(squareSize)
	{
		numSquares = 1.f / squareSize;
	}

	Vector3 GetColor(const Vector2& barycentrics, const Vector2& uv) const override;

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

	Vector3 GetColor(const Vector2& barycentrics, const Vector2& uv) const override;

	~BitmapTexture() override;

private:
	void LoadImageTexture(const std::string& filePath);

	int width;
	int height;
	int channels;
	unsigned char* image = nullptr;
};
