#pragma once
#include "Pch.h"

#include "Engine/Types/Vector2.h"

template <typename T>
struct BaseVector3
{
public:
	T x, y, z;

	// Constants.
	static const BaseVector3<T> Zero;
	static const BaseVector3<T> One;
	static const BaseVector3<T> Forward;
	static const BaseVector3<T> Right;
	static const BaseVector3<T> Up;

	// Constructors
	BaseVector3() = default;
	BaseVector3(const BaseVector3& other) = default;
	BaseVector3(BaseVector2<T> v, T _z)
		: z(_z)
	{
		x = v.x;
		y = v.y;
	}
	BaseVector3(T _x, T _y, T _z)
		: x(_x)
		, y(_y)
		, z(_z)
	{
	}

	// Operator overloads
	BaseVector3& operator=(const BaseVector3& vector) = default;
	BaseVector3& operator+=(T scalar)
	{
		x += scalar;
		y += scalar;
		z += scalar;
		return *this;
	}
	BaseVector3& operator+=(const BaseVector3& vector)
	{
		x += vector.x;
		y += vector.y;
		z += vector.z;
		return *this;
	}
	BaseVector3& operator-=(T scalar)
	{
		x -= scalar;
		y -= scalar;
		z -= scalar;
	}
	BaseVector3& operator-=(const BaseVector3& vector)
	{
		x -= vector.x;
		y -= vector.y;
		z -= vector.z;
		return *this;
	}
	BaseVector3& operator*=(T scalar)
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
	}
	BaseVector3& operator*=(const BaseVector3& vector)
	{
		x *= vector.x;
		y *= vector.y;
		z *= vector.z;
		return *this;
	}
	BaseVector3& operator/=(T scalar)
	{
		float inverse = 1.0f / scalar;
		x *= inverse;
		y *= inverse;
		z *= inverse;
	}
	BaseVector3& operator/=(const BaseVector3& vector)
	{
		x /= vector.x;
		y /= vector.y;
		z /= vector.z;
		return *this;
	}

	// Helper functions.
	T Length() const
	{
		return sqrt(Dot(*this, *this));
	}
	BaseVector3 Abs() const
	{
		return BaseVector3(abs(x), abs(y), abs(z));
	}
	BaseVector3 Normalize() const
	{
		float inveseLength = 1.0f / Length();
		return BaseVector3<T>(x * inveseLength, y * inveseLength, z * inveseLength);
	}
	static BaseVector3 Determinant(const BaseVector3& a, const BaseVector3& b)
	{
		return BaseVector3(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x
		);
	}
	static float Dot(const BaseVector3& a, const BaseVector3& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}
	static BaseVector3 Cross(const BaseVector3& a, const BaseVector3& b)
	{
		return BaseVector3(
			a.y * b.z - b.y * a.z,
			a.z * b.x - b.z * a.x,
			a.x * b.y - b.x * a.y
		);
	}
	static BaseVector3 Min(const BaseVector3& a, const BaseVector3& b)
	{
		return BaseVector3(
			a.x < b.x ? a.x : b.x,
			a.y < b.y ? a.y : b.y,
			a.z < b.z ? a.z : b.z
		);
	}
	static BaseVector3 Max(const BaseVector3& a, const BaseVector3& b)
	{
		return BaseVector3(
			a.x > b.x ? a.x : b.x,
			a.y > b.y ? a.y : b.y,
			a.z > b.z ? a.z : b.z
		);
	}
};

static_assert(sizeof(BaseVector3<float>) == 4 * 3, "BaseVector3 is not expected size. Data should be tightly packed.");

template <typename T>
__forceinline BaseVector3<T> operator+(const BaseVector3<T>& first)
{
	return BaseVector3<T>(+first.x, +first.y, +first.z);
}

template <typename T>
__forceinline BaseVector3<T> operator-(const BaseVector3<T>& first)
{
	return BaseVector3<T>(-first.x, -first.y, -first.z);
}

template <typename T>
__forceinline BaseVector3<T> operator+(const BaseVector3<T>& first, float second)
{
	return BaseVector3<T>(first.x + second, first.y + second, first.z + second);
}

template <typename T>
__forceinline BaseVector3<T> operator+(const BaseVector3<T>& first, const BaseVector3<T>& second)
{
	return BaseVector3<T>(first.x + second.x, first.y + second.y, first.z + second.z);
}

template <typename T>
__forceinline BaseVector3<T> operator-(const BaseVector3<T>& first, float second)
{
	return BaseVector3<T>(first.x - second, first.y - second, first.z - second);
}

template <typename T>
__forceinline BaseVector3<T> operator-(const BaseVector3<T>& first, const BaseVector3<T>& second)
{
	return BaseVector3<T>(first.x - second.x, first.y - second.y, first.z - second.z);
}

template <typename T>
__forceinline BaseVector3<T> operator*(const BaseVector3<T>& first, float second)
{
	return BaseVector3<T>(first.x * second, first.y * second, first.z * second);
}

template <typename T>
__forceinline BaseVector3<T> operator*(float second, const BaseVector3<T>& first)
{
	return BaseVector3<T>(first.x * second, first.y * second, first.z * second);
}

template <typename T>
__forceinline BaseVector3<T> operator*(const BaseVector3<T>& first, const BaseVector3<T>& second)
{
	return BaseVector3<T>(first.x * second.x, first.y * second.y, first.z * second.z);
}

template <typename T>
__forceinline BaseVector3<T> operator/(const BaseVector3<T>& first, float second)
{
	float inverse = 1.0f / second;
	return BaseVector3<T>(first.x * inverse, first.y * inverse, first.z * inverse);
}

template <typename T>
__forceinline BaseVector3<T> operator/(const BaseVector3<T>& first, const BaseVector3<T>& second)
{
	return BaseVector3<T>(first.x / second.x, first.y / second.y, first.z / second.z);
}

template <typename T>
__forceinline bool operator==(const BaseVector3<T>& first, const BaseVector3<T>& second)
{
	return
		first.x == second.x &&
		first.y == second.y &&
		first.z == second.z;
}

template <typename T>
__forceinline bool operator!=(const BaseVector3<T>& first, const BaseVector3<T>& second)
{
	return !(first == second);
}

typedef BaseVector3<float>			Vector3;
typedef BaseVector3<bool>			BVector3;
typedef BaseVector3<int>			IVector3;
typedef BaseVector3<unsigned int>	UVector3;
typedef BaseVector3<double>			DVector3;
