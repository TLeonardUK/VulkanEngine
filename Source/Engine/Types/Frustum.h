#pragma once
#include "Pch.h"

#include "Engine/Types/Bounds.h"
#include "Engine/Types/OrientedBounds.h"
#include "Engine/Types/Plane.h"

enum class FrustumIntersection
{
	Outside,
	Inside,
	Intersects
};

enum class FrustumPlanes
{
	Left,
	Right,
	Top,
	Bottom,
	Near,
	Far,

	Count
};

enum class FrustumCorners
{
	FarTopLeft,
	FarTopRight,
	FarBottomLeft,
	FarBottomRight,
	NearTopLeft,
	NearTopRight,
	NearBottomLeft,
	NearBottomRight,

	Count
};

struct Frustum
{
public:
	static const Frustum Empty;

	Plane planes[(int)FrustumPlanes::Count];

	Frustum() = default;

private:
	static Plane MakePlane(float a, float b, float c, float d)
	{
		return Plane(a, b, c, d).Normalize();
		//float invLength = Math::InvSqrt(a * a + b * b + c * c);
		//return Plane(-a * invLength, -b * invLength, -c * invLength, d * invLength);
	}

public:

	Frustum(const Matrix4& viewProjection)
	{
		planes[(int)FrustumPlanes::Left] = MakePlane(
			viewProjection[0][3] + viewProjection[0][0],
			viewProjection[1][3] + viewProjection[1][0],
			viewProjection[2][3] + viewProjection[2][0],
			viewProjection[3][3] + viewProjection[3][0]
		);
		planes[(int)FrustumPlanes::Right] = MakePlane(
			viewProjection[0][3] - viewProjection[0][0],
			viewProjection[1][3] - viewProjection[1][0],
			viewProjection[2][3] - viewProjection[2][0],
			viewProjection[3][3] - viewProjection[3][0]
		);
		planes[(int)FrustumPlanes::Top] = MakePlane(
			viewProjection[0][3] - viewProjection[0][1],
			viewProjection[1][3] - viewProjection[1][1],
			viewProjection[2][3] - viewProjection[2][1],
			viewProjection[3][3] - viewProjection[3][1]
		);
		planes[(int)FrustumPlanes::Bottom] = MakePlane(
			viewProjection[0][3] + viewProjection[0][1],
			viewProjection[1][3] + viewProjection[1][1],
			viewProjection[2][3] + viewProjection[2][1],
			viewProjection[3][3] + viewProjection[3][1]
		);
		planes[(int)FrustumPlanes::Near] = MakePlane(
			viewProjection[0][3] + viewProjection[0][2],
			viewProjection[1][3] + viewProjection[1][2],
			viewProjection[2][3] + viewProjection[2][2],
			viewProjection[3][3] + viewProjection[3][2]
		);
		planes[(int)FrustumPlanes::Far] = MakePlane(
			viewProjection[0][3] - viewProjection[0][2],
			viewProjection[1][3] - viewProjection[1][2],
			viewProjection[2][3] - viewProjection[2][2],
			viewProjection[3][3] - viewProjection[3][2]
		);
	}

	void GetCorners(Vector3 corners[(int)FrustumCorners::Count]) const
	{
		corners[(int)FrustumCorners::FarTopLeft] = Plane::Intersect(planes[(int)FrustumPlanes::Far], planes[(int)FrustumPlanes::Top], planes[(int)FrustumPlanes::Left]);
		corners[(int)FrustumCorners::FarTopRight] = Plane::Intersect(planes[(int)FrustumPlanes::Far], planes[(int)FrustumPlanes::Top], planes[(int)FrustumPlanes::Right]);
		corners[(int)FrustumCorners::FarBottomLeft] = Plane::Intersect(planes[(int)FrustumPlanes::Far], planes[(int)FrustumPlanes::Bottom], planes[(int)FrustumPlanes::Left]);
		corners[(int)FrustumCorners::FarBottomRight] = Plane::Intersect(planes[(int)FrustumPlanes::Far], planes[(int)FrustumPlanes::Bottom], planes[(int)FrustumPlanes::Right]);
		corners[(int)FrustumCorners::NearTopLeft] = Plane::Intersect(planes[(int)FrustumPlanes::Near], planes[(int)FrustumPlanes::Top], planes[(int)FrustumPlanes::Left]);
		corners[(int)FrustumCorners::NearTopRight] = Plane::Intersect(planes[(int)FrustumPlanes::Near], planes[(int)FrustumPlanes::Top], planes[(int)FrustumPlanes::Right]);
		corners[(int)FrustumCorners::NearBottomLeft] = Plane::Intersect(planes[(int)FrustumPlanes::Near], planes[(int)FrustumPlanes::Bottom], planes[(int)FrustumPlanes::Left]);
		corners[(int)FrustumCorners::NearBottomRight] = Plane::Intersect(planes[(int)FrustumPlanes::Near], planes[(int)FrustumPlanes::Bottom], planes[(int)FrustumPlanes::Right]);
	}

	FrustumIntersection Intersects(const Bounds& bounds) const
	{
		const float padding = 0.0f;

		Vector3 extents = bounds.GetExtents();
		Vector3 center = bounds.GetCenter();

		bool intersecting = false;

		for (int i = 0; i < (int)FrustumPlanes::Count; i++)
		{
			Vector3 abs = planes[i].GetNormal().Abs();

			Vector3 planeNormal = planes[i].GetNormal();
			float planeDistance = planes[i].GetDistance();

			float r = extents.x * abs.x + extents.y * abs.y + extents.z * abs.z;
			float s = planeNormal.x * center.x + planeNormal.y * center.y + planeNormal.z * center.z;

			if (s + r < -planeDistance - padding)
			{
				return FrustumIntersection::Outside;
			}

			intersecting |= (s - r <= -planeDistance);
		}

		return intersecting ? FrustumIntersection::Intersects : FrustumIntersection::Inside;
	}

	FrustumIntersection Intersects(const OrientedBounds& bounds) const
	{
		const float padding = 0.0f;

		Vector3 extents = bounds.GetExtents();
		Vector3 center = bounds.GetCenter();

		Vector3 up = bounds.GetUpVector();
		Vector3 right = bounds.GetRightVector();
		Vector3 forward = bounds.GetForwardVector();

		bool intersecting = false;

		for (int i = 0; i < (int)FrustumPlanes::Count; i++)
		{
			if (i == (int)FrustumPlanes::Near ||
				i == (int)FrustumPlanes::Far)
			{
				continue;
			}

			Vector3 planeNormal = planes[i].GetNormal();
			float planeDistance = planes[i].GetDistance();

			float r =
				extents.x * Math::Abs(Vector3::Dot(planeNormal, right)) +
				extents.y * Math::Abs(Vector3::Dot(planeNormal, up)) +
				extents.z * Math::Abs(Vector3::Dot(planeNormal, forward));

			float s = planeNormal.x * center.x + planeNormal.y * center.y + planeNormal.z * center.z;

			if (s + r < -planeDistance - padding)
			{
				return FrustumIntersection::Outside;
			}

			intersecting |= (s - r <= -planeDistance);
		}

		return intersecting ? FrustumIntersection::Intersects : FrustumIntersection::Inside;
	}
};
