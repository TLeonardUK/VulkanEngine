#pragma once
#include "Pch.h"

#include "Engine/Types/Vector3.h"

template <typename T>
struct BaseVector4
{
public:
	T x, y, z, w;

	// Constants.
	static const BaseVector4<T> Zero;
	static const BaseVector4<T> One;

	// Constructors
	BaseVector4() = default;
	BaseVector4(const BaseVector4& other) = default;
	BaseVector4(BaseVector3<T> v, T _w)
		: w(_w)
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}
	BaseVector4(T _x, T _y, T _z, T _w)
		: x(_x)
		, y(_y)
		, z(_z)
		, w(_w)
	{
	}

	// Operator overloads
	BaseVector4& operator=(const BaseVector4& vector) = default;
	BaseVector4& operator+=(T scalar)
	{
		x += scalar;
		y += scalar;
		z += scalar;
		w += scalar;
		return *this;
	}
	BaseVector4& operator+=(const BaseVector4& vector)
	{
		x += vector.x;
		y += vector.y;
		z += vector.z;
		w += vector.w;
		return *this;
	}
	BaseVector4& operator-=(T scalar)
	{
		x -= scalar;
		y -= scalar;
		z -= scalar;
		w -= scalar;
		return *this;
	}
	BaseVector4& operator-=(const BaseVector4& vector)
	{
		x -= vector.x;
		y -= vector.y;
		z -= vector.z;
		w -= vector.w;
		return *this;
	}
	BaseVector4& operator*=(T scalar)
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		w *= scalar;
		return *this;
	}
	BaseVector4& operator*=(const BaseVector4& vector)
	{
		x *= vector.x;
		y *= vector.y;
		z *= vector.z;
		w *= vector.w;
		return *this;
	}
	BaseVector4& operator/=(T scalar)
	{
		float inverse = 1.0f / scalar;
		x *= inverse;
		y *= inverse;
		z *= inverse;
		w *= inverse;
		return *this;
	}
	BaseVector4& operator/=(const BaseVector4& vector)
	{
		x /= vector.x;
		y /= vector.y;
		z /= vector.z;
		w /= vector.w;
		return *this;
	}

	// Helper functions.
	T Length() const
	{
		return sqrt(SquaredLength());
	}
	T SquaredLength() const
	{
		return Dot(*this, *this);
	}
	BaseVector4 Round() const
	{
		return BaseVector4(Math::Round(x), Math::Round(y), Math::Round(z), Math::Round(w));
	}
	BaseVector4 Normalize() const
	{
		T length = Length();
		if (length == 0.0f)
		{
			return *this;
		}
		else
		{
			T inveseLength = T(1.0) / length;
			return BaseVector3<T>(x * inveseLength, y * inveseLength, z * inveseLength, w * inveseLength);
		}
	}
	static float Dot(const BaseVector4& a, const BaseVector4& b)
	{
		BaseVector4 tmp(a * b);
		return (tmp.x + tmp.y) + (tmp.z + tmp.w);
	}
};

static_assert(sizeof(BaseVector4<float>) == 4 * 4, "BaseVector4 is not expected size. Data should be tightly packed.");

template <typename T>
__forceinline BaseVector4<T> operator+(const BaseVector4<T>& first)
{
	return BaseVector4<T>(+first.x, +first.y, +first.z, +first.w);
}

template <typename T>
__forceinline BaseVector4<T> operator-(const BaseVector4<T>& first)
{
	return BaseVector4<T>(-first.x, -first.y, -first.z, -first.w);
}

template <typename T>
__forceinline BaseVector4<T> operator+(const BaseVector4<T>& first, float second)
{
	return BaseVector4<T>(first.x + second, first.y + second, first.z + second, first.w + second);
}

template <typename T>
__forceinline BaseVector4<T> operator+(const BaseVector4<T>& first, const BaseVector4<T>& second)
{
	return BaseVector4<T>(first.x + second.x, first.y + second.y, first.z + second.z, first.w + second.w);
}

template <typename T>
__forceinline BaseVector4<T> operator-(const BaseVector4<T>& first, float second)
{
	return BaseVector4<T>(first.x - second, first.y - second, first.z - second, first.w - second);
}

template <typename T>
__forceinline BaseVector4<T> operator-(const BaseVector4<T>& first, const BaseVector4<T>& second)
{
	return BaseVector4<T>(first.x - second.x, first.y - second.y, first.z - second.z, first.w - second.w);
}

template <typename T>
__forceinline BaseVector4<T> operator*(const BaseVector4<T>& first, float second)
{
	return BaseVector4<T>(first.x * second, first.y * second, first.z * second, first.w * second);
}

template <typename T>
__forceinline BaseVector4<T> operator*(const BaseVector4<T>& first, const BaseVector4<T>& second)
{
	return BaseVector4<T>(first.x * second.x, first.y * second.y, first.z * second.z, first.w * second.w);
}

template <typename T>
__forceinline BaseVector4<T> operator/(const BaseVector4<T>& first, float second)
{
	float inverse = 1.0f / second;
	return BaseVector4<T>(first.x * inverse, first.y * inverse, first.z * inverse, first.w * inverse);
}

template <typename T>
__forceinline BaseVector4<T> operator/(const BaseVector4<T>& first, const BaseVector4<T>& second)
{
	return BaseVector4<T>(first.x / second.x, first.y / second.y, first.z / second.z, first.w / second.w);
}

template <typename T>
__forceinline bool operator==(const BaseVector4<T>& first, const BaseVector4<T>& second)
{
	return
		first.x == second.x &&
		first.y == second.y &&
		first.z == second.z &&
		first.w == second.w;
}

template <typename T>
__forceinline bool operator!=(const BaseVector4<T>& first, const BaseVector4<T>& second)
{
	return !(first == second);
}

typedef BaseVector4<float>			Vector4;
typedef BaseVector4<bool>			BVector4;
typedef BaseVector4<int>			IVector4;
typedef BaseVector4<unsigned int>	UVector4;
typedef BaseVector4<double>			DVector4;
