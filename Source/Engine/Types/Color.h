#pragma once

#include <string>
#include <stdarg.h>

class Color
{
public:
	float R, G, B, A;

	Color(float r, float g, float b, float a)
		: R(r)
		, G(g)
		, B(b)
		, A(a)
	{
	}

};