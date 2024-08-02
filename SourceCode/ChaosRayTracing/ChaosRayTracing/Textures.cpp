#include "Textures.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Vector3 EdgesTexture::GetColor(const Vector2& barycentrics, const Vector2& uv) const
{
	if (barycentrics.x < edgeWidth || barycentrics.y < edgeWidth)
		return edgeColor;

	if (1.f - barycentrics.x - barycentrics.y < edgeWidth)
		return edgeColor;

	return innerColor;
}

Vector3 CheckerTexture::GetColor(const Vector2& barycentrics, const Vector2& uv) const
{
	const float& u = uv.x;
	const float& v = uv.y;
	int uIndex = static_cast<int>(u * numSquares);
	int vIndex = static_cast<int>(v * numSquares);

	if (uIndex % 2 == vIndex % 2)
		return colorA;

	return colorB;
}

Vector3 BitmapTexture::GetColor(const Vector2& barycentrics, const Vector2& uv) const
{
	assert(image);

	int x = static_cast<int>(uv.x * width);
	int y = static_cast<int>((1.f - uv.y) * height);

	unsigned char* pixel = image + (y * width + x) * channels;

	unsigned char r = pixel[0];
	unsigned char g = pixel[1];
	unsigned char b = pixel[2];

	return {static_cast<float>(r) / 255.f, static_cast<float>(g) / 255.f, static_cast<float>(b) / 255.f};
}

BitmapTexture::~BitmapTexture()
{
	stbi_image_free(image);
}

void BitmapTexture::LoadImageTexture(const std::string& filePath)
{
	image = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
}
