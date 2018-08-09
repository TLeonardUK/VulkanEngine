#pragma once
#include "Pch.h"

#include "Engine/Types/Vector.h"
#include "Engine/Types/Array.h"
#include "Engine/Types/Math.h"

enum class BoundsCorners
{
	FrontTopLeft,
	FrontTopRight,
	FrontBottomLeft,
	FrontBottomRight,
	BackTopLeft,
	BackTopRight,
	BackBottomLeft,
	BackBottomRight,

	Count
};

struct Bounds
{
public:
	Vector3 min;
	Vector3 max;

	Bounds() = default;

	Bounds(const Vector3& min_, const Vector3& max_)
		: min(min_)
		, max(max_)
	{
	}

	Bounds(const Array<Vector3>& points)
	{
		if (points.size() > 0)
		{
			min = points[0];
			max = points[0];

			for (const Vector3& point : points)
			{
				min = Vector3::Min(min, point);
				max = Vector3::Max(max, point);
			}
		}
		else
		{
			min = Vector3::Zero;
			max = Vector3::Zero;
		}
	}

	Vector3 GetCenter() const
	{
		return Vector3(
			min.x + ((max.x - min.x) * 0.5f),
			min.y + ((max.y - min.y) * 0.5f),
			min.z + ((max.z - min.z) * 0.5f)
		);
	}

	Vector3 GetExtents() const
	{
		return Vector3(
			(max.x - min.x) * 0.5f,
			(max.y - min.y) * 0.5f,
			(max.z - min.z) * 0.5f
		);
	}

	void GetCorners(Vector3 corners[(int)BoundsCorners::Count]) const
	{
		corners[(int)BoundsCorners::FrontTopLeft] = Vector3(min.x, max.y, min.z);
		corners[(int)BoundsCorners::FrontTopRight] = Vector3(max.x, max.y, min.z);
		corners[(int)BoundsCorners::FrontBottomLeft] = Vector3(min.x, min.y, min.z);
		corners[(int)BoundsCorners::FrontBottomRight] = Vector3(max.x, min.y, min.z);
		corners[(int)BoundsCorners::BackTopLeft] = Vector3(min.x, max.y, max.z);
		corners[(int)BoundsCorners::BackTopRight] = Vector3(max.x, max.y, max.z);
		corners[(int)BoundsCorners::BackBottomLeft] = Vector3(min.x, min.y, max.z);
		corners[(int)BoundsCorners::BackBottomRight] = Vector3(max.x, min.y, max.z);
	}

	void GetSubDevisions(Bounds subDevisions[8]) const
	{
		Vector3 center = GetCenter();

		subDevisions[0] = Bounds(Vector3(min.x, min.y, min.z), Vector3(center.x, center.y, center.z));
		subDevisions[1] = Bounds(Vector3(min.x, center.y, min.z), Vector3(center.x, max.y, center.z));
		subDevisions[2] = Bounds(Vector3(center.x, min.y, min.z), Vector3(max.x, center.y, center.z));
		subDevisions[3] = Bounds(Vector3(center.x, center.y, min.z), Vector3(max.x, max.y, center.z));
		subDevisions[4] = Bounds(Vector3(min.x, min.y, center.z), Vector3(center.x, center.y, max.z));
		subDevisions[5] = Bounds(Vector3(min.x, center.y, center.z), Vector3(center.x, max.y, max.z));
		subDevisions[6] = Bounds(Vector3(center.x, min.y, center.z), Vector3(max.x, center.y, max.z));
		subDevisions[7] = Bounds(Vector3(center.x, center.y, center.z), Vector3(max.x, max.y, max.z));
	}

	bool Intersects(const Bounds& bounds) const
	{
		if (max.x < bounds.min.x ||
			max.y < bounds.min.y ||
			max.z < bounds.min.z ||
			min.x > bounds.max.x ||
			min.y > bounds.max.y ||
			min.z > bounds.max.z)
		{
			return false;
		}

		return true;
	}

	bool Contains(const Bounds& bounds) const
	{
		if ((bounds.min.x >= min.x && bounds.max.x <= max.x) &&
			(bounds.min.y >= min.y && bounds.max.y <= max.y) &&
			(bounds.min.z >= min.z && bounds.max.z <= max.z))
		{
			return true;
		}

		return false;
	}
};
