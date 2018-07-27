#pragma once
#include "Pch.h"

#include "Engine/Types/Vector4.h"

class Color
{
public:
	static Color Red;
	static Color Blue;
	static Color Green;
	static Color Black;
	static Color White;

public:
	float r, g, b, a;

	Color() = default;

	Color(float r_, float g_, float b_, float a_)
		: r(r_)
		, g(g_)
		, b(b_)
		, a(a_)
	{
	}

	Vector4 ToVector()
	{
		return Vector4(r, g, b, a);
	}

};