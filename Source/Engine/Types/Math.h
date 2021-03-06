#pragma once
#include "Pch.h"

#include "Engine/Types/Quaternion.h"
#include "Engine/Types/Vector.h"
#include "Engine/Types/Matrix.h"
#include "Engine/Types/Rectangle.h"

namespace Math
{
	static const float Pi = 3.141592653f;

	template <typename T>
	T Radians(T degrees)
	{
		static const float Factor = Pi / T(180);
		return degrees * Factor;
	}

	template <typename T>
	T Degrees(T radians)
	{
		static const float Factor = T(180) / Pi;
		return radians * Factor;
	}
	
	template <typename T>
	T Min(T a, T b)
	{
		return a < b ? a : b;
	}

	template <typename T>
	T Max(T a, T b)
	{
		return a > b ? a : b;
	}

	template <typename T>
	T Sign(T a)
	{
		return (a >= 0 ? 1 : -1);
	}

	__forceinline float Sqrt(float in)
	{
		return sqrtf(in);
	}

	template <typename T>
	T Abs(T in)
	{
		return abs(in);
	}

	__forceinline float InvSqrt(float in)
	{
		return 1.0f / sqrtf(in);
	}

	template <typename T>
	T Squared(T in)
	{
		return in * in;
	}

	template <typename T>
	T Ceil(T in)
	{
		return ceil(in);
	}

	template <typename T>
	T Pow(T a, T b)
	{
		return pow(a, b);
	}

	template <typename T>
	T Lerp(T a, T b, T delta)
	{
		return a + ((b - a) * delta);
	}

	template <typename T>
	T Mod(T a, T b)
	{
		return std::remainder(a, b);
	}

	template <typename T>
	T Round(T a)
	{
		return (T)round(a);
	}

	__forceinline uint32_t RoundUpToPowerOfTwo(uint32_t v)
	{
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v++;
		return v;
	}
}