#pragma once
#include "Pch.h"

#include <pix3.h>

#include "Engine/Types/String.h"
#include "Engine/Types/Color.h"

struct ProfileScope
{
	ProfileScope(const Color& color, const String& str)
	{
#if defined(USE_PIX)
		UINT pixColor = PIX_COLOR((int)(color.r * 255.0f), (int)(color.g * 255.0f), (int)(color.b * 255.0f));
		PIXBeginEvent(pixColor, "%s", str.c_str());
#endif
	}

	~ProfileScope()
	{
#if defined(USE_PIX)
		PIXEndEvent();
#endif
	}
};