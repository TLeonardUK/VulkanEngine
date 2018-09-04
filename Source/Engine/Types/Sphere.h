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

	Sphere(const Vector3* points, int count)
	{
		Vector3 min = points[0];
		Vector3 max = points[0];

		for (int i = 1; i < count; i++)
		{
			min = Vector3::Min(min, points[i]);
			max = Vector3::Max(max, points[i]);
		}

		radius = (max - min).Length() * 0.5f;
		origin = min + radius;
	}

	Sphere(const Array<Vector3>& points)
		: Sphere(points.data(), (int)points.size())
	{
	}

	bool Intersects(const Bounds& bounds) const
	{
		Vector3 closest = Vector3::Min(Vector3::Max(origin, bounds.min), bounds.max);
		double distanceSquared = (closest - origin).SquaredLength();
		return distanceSquared < (radius * radius);
	}

	Bounds GetBounds() const
	{
		return Bounds(origin - radius, origin + radius);
	}
};
