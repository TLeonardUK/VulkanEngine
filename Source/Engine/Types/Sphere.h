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
		origin = points[0];
		for (int i = 1; i < count; i++)
		{
			origin += points[i];
		}
		origin /= count;

		radius = 0.0f;
		for (int i = 0; i < count; i++)
		{
			float distance = (points[i] - origin).Length();
			if (distance > radius)
			{
				radius = distance;
			}
		}
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
