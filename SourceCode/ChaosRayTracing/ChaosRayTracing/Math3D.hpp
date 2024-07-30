#pragma once

#include <cstdint>
#include <numbers>
#include <string>
#include <cmath>
#include <algorithm>
#include <random>
#include <stdexcept>

constexpr float PI = std::numbers::pi_v<float>;

constexpr float degToRad(float degrees)
{
	return degrees * (PI / 180.f);
}

constexpr float radToDeg(float radians)
{
	return radians * (180.f / PI);
}

struct RGB
{
	uint8_t r;
	uint8_t g;
	uint8_t b;

	std::string toString() const
	{
		char buffer[12];
		std::snprintf(buffer, sizeof(buffer), "%u %u %u", r, g, b);
		return {buffer};
	}
};

struct Vector2
{
	float x;
	float y;

	Vector2() = default;

	Vector2(float v) : x(v), y(v)
	{
	}

	Vector2(float x, float y) : x(x), y(y)
	{
	}
};

inline Vector2 operator +(const Vector2& a, const Vector2& b)
{
	return {a.x + b.x, a.y + b.y};
}

inline Vector2 operator -(const Vector2& a, const Vector2& b)
{
	return {a.x - b.x, a.y - b.y};
}

inline Vector2 operator *(const Vector2& v, float s)
{
	return {v.x * s, v.y * s};
}

inline Vector2 operator *(const Vector2& v, const Vector2& u)
{
	return {v.x * u.x, v.y * u.y};
}

inline Vector2 operator /(const Vector2& v, float s)
{
	s = 1.f / s;
	return {v.x * s, v.y * s};
}

struct Vector3
{
	union
	{
		struct
		{
			float x;
			float y;
			float z;
		};

		float data[3];
	};

	Vector3() = default;

	Vector3(float v) : x(v), y(v), z(v)
	{
	}

	Vector3(float x, float y, float z) : x(x), y(y), z(z)
	{
	}

	float& operator[](size_t index)
	{
		if (index < 3)
			return data[index];
		throw std::out_of_range("Index out of range");
	}

	const float& operator[](size_t index) const
	{
		if (index < 3)
			return data[index];
		throw std::out_of_range("Index out of range");
	}

	Vector3 operator -() const
	{
		return {-x, -y, -z};
	}

	Vector3& operator *=(float s)
	{
		x *= s;
		y *= s;
		z *= s;
		return (*this);
	}

	Vector3& operator *=(const Vector3& v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		return (*this);
	}

	Vector3& operator /=(float s)
	{
		s = 1.f / s;
		x *= s;
		y *= s;
		z *= s;
		return (*this);
	}

	Vector3& operator +=(const Vector3& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return (*this);
	}

	Vector3& operator -=(const Vector3& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return (*this);
	}

	float magnitude() const
	{
		return std::sqrt(x * x + y * y + z * z);
	}

	RGB toRGB() const
	{
		return RGB{
			static_cast<uint8_t>(std::clamp(x, 0.f, 1.f) * 255), static_cast<uint8_t>(std::clamp(y, 0.f, 1.f) * 255),
			static_cast<uint8_t>(std::clamp(z, 0.f, 1.f) * 255)
		};
	}

	std::string ToString() const
	{
		return "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
	}

	Vector2 xy() const
	{
		return {x, y};
	}

	Vector2 yz() const
	{
		return {y, z};
	}
};

inline Vector3 operator +(const Vector3& a, const Vector3& b)
{
	return {a.x + b.x, a.y + b.y, a.z + b.z};
}

inline Vector3 operator -(const Vector3& a, const Vector3& b)
{
	return {a.x - b.x, a.y - b.y, a.z - b.z};
}

inline Vector3 operator *(const Vector3& v, float s)
{
	return {v.x * s, v.y * s, v.z * s};
}

inline Vector3 operator *(const Vector3& v, const Vector3& u)
{
	return {v.x * u.x, v.y * u.y, v.z * u.z};
}

inline Vector3 operator /(const Vector3& v, float s)
{
	s = 1.f / s;
	return {v.x * s, v.y * s, v.z * s};
}

inline Vector3 Normalize(const Vector3& v)
{
	return v / v.magnitude();
}

inline float Magnitude(const Vector3& v)
{
	return v.magnitude();
}

inline Vector3 Cross(const Vector3& a, const Vector3& b)
{
	return {
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}

inline float Dot(const Vector3& a, const Vector3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vector3 min(const Vector3& a, const Vector3& b)
{
	return Vector3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));
}

inline Vector3 max(const Vector3& a, const Vector3& b)
{
	return Vector3(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));
}

struct Point3 : Vector3
{
	Point3() = default;

	Point3(float a, float b, float c) : Vector3(a, b, c)
	{
	}

	Point3& operator =(const Vector3& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		return (*this);
	}
};

inline Point3 operator +(const Point3& a, const Vector3& b)
{
	return Point3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline Point3 operator -(const Point3& a, const Vector3& b)
{
	return Point3(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline Vector3 operator -(const Point3& a, const Point3& b)
{
	return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline Vector3 OffsetRayOrigin(const Vector3& origin, const Vector3& normal)
{
	constexpr float rayOffset = 0.001f;
	return origin + normal * rayOffset;
}

struct Ray
{
	Vector3 origin;
	Vector3 directionN;
	Vector3 directionNInv;
	float maxT;

	Ray(const Vector3& origin, const Vector3& directionN, float maxT = std::numeric_limits<float>::max())
		: origin(origin), directionN(directionN), maxT(maxT)
	{
		directionNInv = Vector3(1.f / directionN.x, 1.f / directionN.y, 1.f / directionN.z);
	}

	Vector3 operator()(float t) const
	{
		return origin + directionN * t;
	}
};

struct HitInfo
{
	bool hit = false;
	float t = std::numeric_limits<float>::max();
	Vector3 point;
	Vector3 normal;
	Vector2 barycentrics;
	uint32_t materialIndex;
	uint32_t triangleIndex;
};

struct Vertex
{
	Vector3 position;
	Vector3 normal;
	Vector2 uv;
};

struct Triangle
{
	Vertex v0;
	Vertex v1;
	Vertex v2;

	uint32_t materialIndex;

	Vector3 faceNormal;

	int32_t emissiveIndex = -1;

	Triangle(const Vertex& a, const Vertex& b, const Vertex& c, uint32_t materialIndex, int32_t emissiveIndex)
		: v0(a), v1(b), v2(c), materialIndex(materialIndex), emissiveIndex(emissiveIndex)
	{
		this->faceNormal = Normalize(Cross(v1.position - v0.position, v2.position - v0.position));
	}

	Vector3 centroid() const
	{
		return (v0.position + v1.position + v2.position) / 3.f;
	}

	float area() const
	{
		return Cross(v1.position - v0.position, v2.position - v0.position).magnitude() * 0.5f;
	}

	Vector3 getNormal(const Vector2& barycentrics) const
	{
		float w = 1.f - barycentrics.x - barycentrics.y;
		return Normalize(v1.normal * barycentrics.x + v2.normal * barycentrics.y + v0.normal * w);
	}

	Vector2 getUVs(const Vector2& barycentrics) const
	{
		float w = 1.f - barycentrics.x - barycentrics.y;
		return v1.uv * barycentrics.x + v2.uv * barycentrics.y + v0.uv * w;
	}

	HitInfo intersect(const Ray& ray, bool backFaceCull) const
	{
		HitInfo info;

		const Vector3& a = v0.position;
		const Vector3& b = v1.position;
		const Vector3& c = v2.position;

		float dirDotNorm = Dot(ray.directionN, faceNormal);
		if (backFaceCull && dirDotNorm >= 0.f)
			return info;

		float t = Dot(a - ray.origin, faceNormal) / dirDotNorm;
		if (t < 0.f || t > ray.maxT)
			return info;

		Vector3 p = ray(t);

		Vector3 edge0 = b - a;
		Vector3 edge1 = c - b;
		Vector3 edge2 = a - c;
		Vector3 C0 = p - a;
		Vector3 C1 = p - b;
		Vector3 C2 = p - c;

		if (Dot(faceNormal, Cross(edge0, C0)) < 0.f)
			return info;
		if (Dot(faceNormal, Cross(edge1, C1)) < 0.f)
			return info;
		if (Dot(faceNormal, Cross(edge2, C2)) < 0.f)
			return info;

		// Calculate the barycentric coordinates
		float triArea = Magnitude(Cross(b - a, c - a)); // area of the whole triangle
		info.barycentrics.x = Magnitude(Cross(p - a, c - a)) / triArea;
		info.barycentrics.y = Magnitude(Cross(b - a, p - a)) / triArea;

		info.hit = true;
		info.t = t;
		info.point = p;
		info.normal = faceNormal;
		info.materialIndex = materialIndex;

		return info;
	}
};

struct Matrix4
{
protected:
	float n[4][4];

public:
	Matrix4() = default;

	Matrix4(float n00, float n01, float n02, float n03,
	        float n10, float n11, float n12, float n13,
	        float n20, float n21, float n22, float n23,
	        float n30, float n31, float n32, float n33)
	{
		n[0][0] = n00;
		n[0][1] = n10;
		n[0][2] = n20;
		n[0][3] = n30;
		n[1][0] = n01;
		n[1][1] = n11;
		n[1][2] = n21;
		n[1][3] = n31;
		n[2][0] = n02;
		n[2][1] = n12;
		n[2][2] = n22;
		n[2][3] = n32;
		n[3][0] = n03;
		n[3][1] = n13;
		n[3][2] = n23;
		n[3][3] = n33;
	}


	float& operator ()(int i, int j)
	{
		return n[j][i];
	}

	const float& operator ()(int i, int j) const
	{
		return n[j][i];
	}

	const Point3& getTranslation(void) const
	{
		return *reinterpret_cast<const Point3*>(n[3]);
	}

	static Matrix4 identity()
	{
		return {
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		};
	}
};

static Matrix4 makeTranslation(Vector3 t)
{
	return {
		1.f, 0.f, 0.f, t.x,
		0.f, 1.f, 0.f, t.y,
		0.f, 0.f, 1.f, t.z,
		0.f, 0.f, 0.f, 1.f
	};
}

static Matrix4 makeRotationX(float t)
{
	float c = cos(t);
	float s = sin(t);

	return {
		1.f, 0.f, 0.f, 0.f,
		0.f, c, -s, 0.f,
		0.f, s, c, 0.f,
		0.f, 0.f, 0.f, 1.f
	};
}

static Matrix4 makeRotationY(float t)
{
	float c = cos(t);
	float s = sin(t);

	return {
		c, 0.f, s, 0.f,
		0.f, 1.f, 0.f, 0.f,
		-s, 0.f, c, 0.f,
		0.f, 0.f, 0.f, 1.f
	};
}

static Matrix4 makeRotationZ(float t)
{
	float c = cos(t);
	float s = sin(t);

	return {
		c, -s, 0.f, 0.f,
		s, c, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
	};
}

static Matrix4 lookAtInverse(const Vector3& eye, const Vector3& center, const Vector3& up)
{
	Vector3 f = Normalize(center - eye);
	Vector3 s = Normalize(Cross(f, up));
	Vector3 u = Cross(s, f);

	return {
		s.x, u.x, -f.x, eye.x,
		s.y, u.y, -f.y, eye.y,
		s.z, u.z, -f.z, eye.z,
		0.f, 0.f, 0.f, 1.f
	};
}

inline Vector3 operator *(const Matrix4& H, const Vector3& v)
{
	return {
		H(0, 0) * v.x + H(0, 1) * v.y + H(0, 2) * v.z,
		H(1, 0) * v.x + H(1, 1) * v.y + H(1, 2) * v.z,
		H(2, 0) * v.x + H(2, 1) * v.y + H(2, 2) * v.z
	};
}

inline Point3 operator *(const Matrix4& H, const Point3& p)
{
	return {
		H(0, 0) * p.x + H(0, 1) * p.y + H(0, 2) * p.z + H(0, 3),
		H(1, 0) * p.x + H(1, 1) * p.y + H(1, 2) * p.z + H(1, 3),
		H(2, 0) * p.x + H(2, 1) * p.y + H(2, 2) * p.z + H(2, 3)
	};
}

inline Matrix4 operator*(const Matrix4& A, const Matrix4& B)
{
	Matrix4 result;
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			result(i, j) = 0;
			for (int k = 0; k < 4; ++k)
			{
				result(i, j) += A(i, k) * B(k, j);
			}
		}
	}
	return result;
}

struct Range
{
	uint32_t start;
	uint32_t end;

	uint32_t count() const { return end - start; }
};

inline Vector3 randomInHemisphereCosine(const Vector3& normal, const Vector2& rnd)
{
	float theta = acos(sqrt(1.0f - rnd.x)); // Theta follows a cosine distribution
	float phi = 2.0f * PI * rnd.y;

	float x = sin(theta) * cos(phi);
	float y = sin(theta) * sin(phi);
	float z = cos(theta);

	Vector3 randomDirection(x, y, z);

	// Ensure it's in the same hemisphere as the normal
	if (Dot(randomDirection, normal) < 0.0f)
	{
		randomDirection = -randomDirection;
	}

	return randomDirection;
}
