#pragma once
#include "Pch.h"

template <typename T>
struct BaseVector2
{
public:
	T x, y;

	// Constants.
	static const BaseVector2<T> Zero;
	static const BaseVector2<T> One;

	// Constructors
	BaseVector2() = default;
	BaseVector2(const BaseVector2& other) = default;
	BaseVector2(T _x, T _y)
		: x(_x)
		, y(_y)
	{
	}

	// Operator overloads
	BaseVector2& operator=(const BaseVector2& vector) = default;
	BaseVector2& operator+=(T scalar)
	{
		x += scalar;
		y += scalar;
		return *this;
	}
	BaseVector2& operator+=(const BaseVector2& vector)
	{
		x += vector.x;
		y += vector.y;
		return *this;
	}
	BaseVector2& operator-=(T scalar)
	{
		x -= scalar;
		y -= scalar;
	}
	BaseVector2& operator-=(const BaseVector2& vector)
	{
		x -= vector.x;
		y -= vector.y;
		return *this;
	}
	BaseVector2& operator*=(T scalar)
	{
		x *= scalar;
		y *= scalar;
	}
	BaseVector2& operator*=(const BaseVector2& vector)
	{
		x *= vector.x;
		y *= vector.y;
		return *this;
	}
	BaseVector2& operator/=(T scalar)
	{
		float inverse = 1.0f / scalar;
		x *= inverse;
		y *= inverse;
	}
	BaseVector2& operator/=(const BaseVector2& vector)
	{
		x /= vector.x;
		y /= vector.y;
		return *this;
	}

	// Helper functions.
	T Length() const
	{
		return sqrt(Dot(*this, *this));
	}
	BaseVector2 Normalize() const
	{
		float inveseLength = 1.0f / Length();
		return BaseVector2<T>(x * inveseLength, y * inveseLength);
	}
	static float Dot(const BaseVector2& a, const BaseVector2& b)
	{
		BaseVector2 tmp(a * b);
		return tmp.x + tmp.y;
	}
};

static_assert(sizeof(BaseVector2<float>) == 4 * 2, "BaseVector2 is not expected size. Data should be tightly packed.");

template <typename T>
__forceinline BaseVector2<T> operator+(const BaseVector2<T>& first)
{
	return BaseVector2<T>(+first.x, +first.y);
}

template <typename T>
__forceinline BaseVector2<T> operator-(const BaseVector2<T>& first)
{
	return BaseVector2<T>(-first.x, -first.y);
}

template <typename T>
__forceinline BaseVector2<T> operator+(const BaseVector2<T>& first, float second)
{
	return BaseVector2<T>(first.x + second, first.y + second);
}

template <typename T>
__forceinline BaseVector2<T> operator+(const BaseVector2<T>& first, const BaseVector2<T>& second)
{
	return BaseVector2<T>(first.x + second.x, first.y + second.y);
}

template <typename T>
__forceinline BaseVector2<T> operator-(const BaseVector2<T>& first, float second)
{
	return BaseVector2<T>(first.x - second, first.y - second);
}

template <typename T>
__forceinline BaseVector2<T> operator-(const BaseVector2<T>& first, const BaseVector2<T>& second)
{
	return BaseVector2<T>(first.x - second.x, first.y - second.y);
}

template <typename T>
__forceinline BaseVector2<T> operator*(const BaseVector2<T>& first, float second)
{
	return BaseVector2<T>(first.x * second, first.y * second);
}

template <typename T>
__forceinline BaseVector2<T> operator*(const BaseVector2<T>& first, const BaseVector2<T>& second)
{
	return BaseVector2<T>(first.x * second.x, first.y * second.y);
}

template <typename T>
__forceinline BaseVector2<T> operator/(const BaseVector2<T>& first, float second)
{
	float inverse = 1.0f / second;
	return BaseVector2<T>(first.x * inverse, first.y * inverse);
}

template <typename T>
__forceinline BaseVector2<T> operator/(const BaseVector2<T>& first, const BaseVector2<T>& second)
{
	return BaseVector2<T>(first.x / second.x, first.y / second.y);
}

template <typename T>
__forceinline bool operator==(const BaseVector2<T>& first, const BaseVector2<T>& second)
{
	return
		first.x == second.x &&
		first.y == second.y;
}

template <typename T>
__forceinline bool operator!=(const BaseVector2<T>& first, const BaseVector2<T>& second)
{
	return !(first == second);
}

typedef BaseVector2<float>			Vector2;
typedef BaseVector2<bool>			BVector2;
typedef BaseVector2<int>			IVector2;
typedef BaseVector2<unsigned int>	UVector2;
typedef BaseVector2<double>			DVector2;
