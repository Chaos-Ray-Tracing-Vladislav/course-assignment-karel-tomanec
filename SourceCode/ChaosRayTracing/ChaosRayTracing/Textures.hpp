#pragma once

#include "Math3D.hpp"

class Texture 
{
public:

	Texture(std::string name) : name(std::move(name)) {}

	virtual Vector3 GetColor(float u, float v) = 0;

	virtual ~Texture() = default;

	std::string name;
};

class AlbedoTexture : public Texture 
{
public:

	AlbedoTexture(std::string name, Vector3 albedo) : Texture(std::move(name)), albedo(std::move(albedo)) {}

	Vector3 GetColor(float u, float v) override { return albedo; }

private:

	Vector3 albedo;
};

class EdgesTexture : public Texture 
{
public:

	EdgesTexture(std::string name, Vector3 edgeColor, Vector3 innerColor, float edgeWidth) 
		: Texture(std::move(name)), edgeColor(std::move(edgeColor)), innerColor(std::move(innerColor)), edgeWidth(edgeWidth)
	{ }

	Vector3 GetColor(float u, float v) override { return {}; }

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
	{ }

	Vector3 GetColor(float u, float v) override { return {}; }

private:
	Vector3 colorA;
	Vector3 colorB;
	float squareSize;
};

class BitmapTexture : public Texture 
{
public:
	BitmapTexture(std::string name, std::string filePath) 
		: Texture(std::move(name)), filePath(std::move(filePath))
	{ }

	Vector3 GetColor(float u, float v) override { return {}; }
	
private:
	std::string filePath;
};