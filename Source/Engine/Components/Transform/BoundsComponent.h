#pragma once
#include "Pch.h"

#include "Engine/Types/OrientedBounds.h"

// Describes the bounds of an entity.
struct BoundsComponent
{
	OrientedBounds bounds;

	int lastTransformVersion = -1;
};
