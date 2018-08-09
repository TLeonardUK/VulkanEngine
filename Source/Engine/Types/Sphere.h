#pragma once
#include "Pch.h"

#include "Engine/Types/Vector3.h"
#include "Engine/Types/Bounds.h"

struct Sphere
{
	static const Sphere Empty;

	Vector3 origin;
	float radius;

	Sphere() = default;

	Sphere(const Vector3& origin_, float radius_)
		: origin(origin_)
		, radius(radius_)
	{
	}

	bool Intersects(const Bounds& bounds) const
	{
		Vector3 closest = Vector3::Min(Vector3::Max(origin, bounds.min), bounds.max);
		double distanceSquared = (closest - origin).SquaredLength();
		return distanceSquared < (radius * radius);
	}
};
