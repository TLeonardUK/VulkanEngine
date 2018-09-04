#pragma once
#include "Pch.h"

#include "Engine/Types/Bounds.h"

enum class PlaneClassification
{
	Behind,
	Infront,
	Intersecting
};

struct Plane
{
public:
	float x;
	float y;
	float z;
	float w;

	Plane() = default;

	Plane(float x_, float y_, float z_, float w_)
		: x(x_)
		, y(y_)
		, z(z_)
		, w(w_)
	{
	}

	Plane(const Vector3& normal, const Vector3& origin)
	{
		x = normal.x;
		y = normal.y;
		z = normal.z;
		w = Vector3::Dot(normal, origin);
	}

	Vector3 GetNormal() const
	{
		return Vector3(x, y, z);
	}

	float GetDistance() const
	{
		return w;
	}

	PlaneClassification Classify(const Vector3& point) const
	{
		float dot = Dot(*this, point);
		if (dot < 0.0f)
		{
			return PlaneClassification::Behind;
		}
		else if (dot > 0.0f)
		{
			return PlaneClassification::Infront;
		}
		else
		{
			return PlaneClassification::Intersecting;
		}
	}

	Plane Normalize() const
	{
		float sum = x * x + y * y + z * z;
		float invSqrt = Math::InvSqrt(sum);

		return Plane(x * invSqrt, y * invSqrt, z * invSqrt, w * invSqrt);
	}

	Plane Flip() const
	{
		return Plane(-x, -y, -z, -w);
	}

	static Vector3 Intersect(const Plane& plane1, const Plane& plane2, const Plane& plane3)
	{
		// Faster method, but will not handle situations such as planes not all intersecting,
		// thus giving multiple line-segments intersections.

		Vector3 p0 = plane1.GetNormal();
		Vector3 p1 = plane2.GetNormal();
		Vector3 p2 = plane3.GetNormal();

		Vector3 bxc = Vector3::Cross(p1, p2);
		Vector3 cxa = Vector3::Cross(p2, p0);
		Vector3 axb = Vector3::Cross(p0, p1);
		Vector3 r = -plane1.w * bxc - plane2.w * cxa - plane3.w * axb;

		return r * (1 / Vector3::Dot(p0, bxc));

		/*
		Vector3 abDet = Vector3::Determinant(plane1Vec, plane2Vec);
		float determinant = Vector3::Dot(abDet, plane3Vec);
		if (Math::Squared(determinant) < Math::Squared(0.001f))
		{
			return Vector3::Zero;
		}
		else
		{
			Vector3 bcDet = Vector3::Determinant(plane2Vec, plane3Vec);
			Vector3 caDet = Vector3::Determinant(plane3Vec, plane1Vec);
			return (plane1.w * bcDet + plane2.w * caDet + plane3.w * abDet) / determinant;
		}*/
	}

	static float Dot(const Plane& plane1, const Plane& plane2)
	{
		return plane1.x * plane2.x + plane1.y * plane2.y + plane1.z * plane2.z + plane1.w * plane2.w;
	}

	static float Dot(const Plane& plane, const Vector3& point)
	{
		return plane.x * point.x + plane.y * point.y + plane.z * point.z - plane.w;
	}
};
