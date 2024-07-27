#include "Material.hpp"
#include "Textures.hpp"

Vector3 Material::GetAlbedo(const Vector2& barycentrics, const Vector2& uv) const
{
	if (texture)
		return texture->GetColor(barycentrics, uv);

	return albedo;
}