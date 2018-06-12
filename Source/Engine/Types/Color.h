#pragma once

#include <string>
#include <stdarg.h>

class Color
{
public:
	float r, g, b, a;

	Color(float r_, float g_, float b_, float a_)
		: r(r_)
		, g(g_)
		, b(b_)
		, a(a_)
	{
	}

};