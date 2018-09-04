#pragma once
#include "Pch.h"

struct Rect
{
	static const Rect Empty;

	float x, y, width, height;

	Rect() = default;
	
	Rect(float x, float y, float w, float h)
		: x(x)
		, y(y)
		, width(w)
		, height(h)
	{
	}
};

__forceinline bool operator==(const Rect& first, const Rect& second)
{
	return 
		first.x == second.x &&
		first.y == second.y &&
		first.width == second.width &&
		first.height == second.height;
}

__forceinline bool operator!=(const Rect& first, const Rect& second)
{
	return !(first == second);
}

