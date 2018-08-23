#pragma once
#include "Pch.h"

#include "Engine/Types/Vector4.h"

class Color
{
public:
	static Color PureRed;
	static Color PureBlue;
	static Color PureGreen;
	static Color Black;
	static Color White;

	static Color Red;
	static Color Pink;
	static Color Purple;
	static Color DeepPurple;
	static Color Indigo;
	static Color Blue;
	static Color LightBlue;
	static Color Cyan;
	static Color Teal;
	static Color Green;
	static Color LightGreen;
	static Color Lime;
	static Color Yellow;
	static Color Amber;
	static Color Orange;
	static Color DeepOrange;
	static Color Brown;
	static Color Grey;
	static Color BlueGrey;


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

	static Color FromArgb(int r, int g, int b, int a)
	{
		return Color(
			r / 255.0f,
			g / 255.0f,
			b / 255.0f,
			a / 255.0f
		);
	}

};