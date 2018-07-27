#pragma once
#include "Pch.h"

struct Rect
{
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
