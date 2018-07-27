#pragma once
#include "Pch.h"

#include "Engine/Types/Bounds.h"

struct OrientedBounds
{
public:
	Bounds bounds;
	Matrix4 transform;

	OrientedBounds() = default;

	OrientedBounds(const Bounds& bounds_, const Matrix4& transform_)
		: bounds(bounds_)
		, transform(transform_)
	{
	}

	Vector3 GetCenter() const
	{
		return bounds.GetCenter() * transform;
	}

	Vector3 GetExtents() const
	{
		// This is lossy. Better bet is to have scale baked in bounds and have transform
		// be purely location/rotation?.
		return bounds.GetExtents() * transform.ExtractScale();
	}

	Vector3 GetUpVector() const
	{
		return transform.TransformDirection(Vector3::Up);
	}

	Vector3 GetRightVector() const
	{
		return transform.TransformDirection(Vector3::Right);
	}

	Vector3 GetForwardVector() const
	{
		return transform.TransformDirection(Vector3::Forward);
	}

	void GetCorners(Vector3 corners[(int)BoundsCorners::Count]) const
	{
		bounds.GetCorners(corners);

		for (int i = 0; i < (int)BoundsCorners::Count; i++)
		{
			corners[i] = corners[i] * transform;
		}
	}

	Bounds GetAlignedBounds() const
	{
		Vector3 worldCorners[(int)BoundsCorners::Count];
		bounds.GetCorners(worldCorners);

		Array<Vector3> points;
		points.reserve(8);
		for (int i = 0; i < (int)BoundsCorners::Count; i++)
		{
			points.push_back(worldCorners[i]);
		}

		return Bounds(points);
	}
};
