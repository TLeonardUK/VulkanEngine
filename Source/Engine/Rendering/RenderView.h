#pragma once

#include "Engine/Types/Math.h"

class RenderView
{
public:
	RenderView() { }

	Matrix4 ViewMatrix;
	Matrix4 ProjectionMatrix;

	Rectangle Viewport;

};