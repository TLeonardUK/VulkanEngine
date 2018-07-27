#pragma once
#include "Pch.h"

#include "Engine/Types/Vector3.h"
#include "Engine/Types/Vector4.h"

template <typename T>
struct QuaternionBase
{
public:
	T x, y, z, w;

	// Constants.
	static const QuaternionBase<T> Identity;

	// Constructors
	QuaternionBase() = default;
	QuaternionBase(const QuaternionBase& other) = default;
	QuaternionBase(T _x, T _y, T _z, T _w)
		: x(_x)
		, y(_y)
		, z(_z)
		, w(_w)
	{
	}

	// Operator overloads
	QuaternionBase& operator=(const QuaternionBase& other) = default;
	QuaternionBase& operator+=(const QuaternionBase& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;
		return *this;
	}
	QuaternionBase& operator-=(const QuaternionBase& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		w -= other.w;
		return *this;
	}
	QuaternionBase& operator*=(const QuaternionBase& other)
	{
		QuaternionBase a(*this);
		QuaternionBase b(other);

		w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
		x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
		y = a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z;
		z = a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x;
		return *this;
	}
	QuaternionBase& operator*=(T scalar)
	{
		x *= other.x;
		y *= other.y;
		z *= other.z;
		w *= other.w;
		return *this;
	}
	QuaternionBase& operator/=(const QuaternionBase& other)
	{
		x /= other.x;
		y /= other.y;
		z /= other.z;
		w /= other.w;
		return *this;
	}
	QuaternionBase& operator/=(T scalar)
	{
		x /= scalar;
		y /= scalar;
		z /= scalar;
		w /= scalar;
		return *this;
	}

	// Helper constructors.
	T Length() const
	{
		return sqrt(Dot(*this, *this));
	}
	QuaternionBase<T> Normalize() const
	{
		T len = Length();
		if (len <= T(0))
		{
			return QuaternionBase<T>(T(0), T(0), T(0), T(1));
		}
		T oneOverLen = T(1) / len;
		return QuaternionBase<T>(x * oneOverLen, y * oneOverLen, z * oneOverLen, w * oneOverLen);
	}
	QuaternionBase<T> Conjugate() const
	{
		return QuaternionBase<T>(-x, -y, -z, w);
	}
	QuaternionBase<T> Inverse() const
	{
		return Conjugate() / Dot(*this, *this);
	}
	/*static QuaternionBase<T> LookAt(const BaseVector3<T>& direction, const BaseVector3<T>& up = BaseVector3<T>::Up)
	{
		Matrix3 result;

		BaseVector3<T> column2 = -direction;
		BaseVector3<T> column0 = BaseVector3<T>::Cross(up, column2).Normalize();
		BaseVector3<T> column1 = BaseVector3<T>::Cross(column2, column0);

		result.SetColumn(0, column0);
		result.SetColumn(0, column1);
		result.SetColumn(0, column2);

		return result.ToQuaternion();
	}*/
	static T Dot(const QuaternionBase<T>& a, const QuaternionBase<T>& b)
	{
		BaseVector4<T> tmp(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
		return (tmp.x + tmp.y) + (tmp.z + tmp.w);
	}
	static QuaternionBase<T> AngleAxis(T angle, const BaseVector3<T>& axis)
	{
		QuaternionBase<T> result;

		T s = static_cast<T>(sin(angle * static_cast<T>(0.5)));

		result.w = static_cast<T>(cos(angle * static_cast<T>(0.5)));
		result.x = axis.x * s;
		result.y = axis.y * s;
		result.z = axis.z * s;

		return result;
	}
	static QuaternionBase<T> Euler(const BaseVector3<T>& angles)
	{
		QuaternionBase<T> result;

		BaseVector3<T> c(cos(angles.x * T(0.5)), cos(angles.y * T(0.5)), cos(angles.z * T(0.5)));
		BaseVector3<T> s(sin(angles.x * T(0.5)), sin(angles.y * T(0.5)), sin(angles.z * T(0.5)));

		result.w = c.x * c.y * c.z + s.x * s.y * s.z;
		result.x = s.x * c.y * c.z - c.x * s.y * s.z;
		result.y = c.x * s.y * c.z + s.x * c.y * s.z;
		result.z = c.x * c.y * s.z - s.x * s.y * c.z;

		return result;
	}

	//Slerp
	//Lerp
	//Rotate
};

template <typename T>
__forceinline QuaternionBase<T> operator+(const QuaternionBase<T>& first)
{
	return first;
}

template <typename T>
__forceinline QuaternionBase<T> operator-(const QuaternionBase<T>& first)
{
	return QuaternionBase<T>(-first.x, -first.y, -first.z, -first.w);
}

template <typename T>
__forceinline QuaternionBase<T> operator+(const QuaternionBase<T>& first, const QuaternionBase<T>& second)
{
	return QuaternionBase<T>(first) += second;
}

template <typename T>
__forceinline QuaternionBase<T> operator*(const QuaternionBase<T>& first, float second)
{
	return QuaternionBase<T>(first) *= second;
}

template <typename T>
__forceinline QuaternionBase<T> operator*(const QuaternionBase<T>& first, const QuaternionBase<T>& second)
{
	return QuaternionBase<T>(first) *= second;
}

template <typename T>
__forceinline BaseVector3<T> operator*(const BaseVector3<T>& vec, const QuaternionBase<T>& quat)
{
	return quat.Inverse() * vec;
}

template <typename T>
__forceinline BaseVector3<T> operator*(const QuaternionBase<T>& quat, const BaseVector3<T>& vec)
{
	BaseVector3<T> quatVector(quat.x, quat.y, quat.z);
	BaseVector3<T> uv(BaseVector3<T>::Cross(quatVector, vec));
	BaseVector3<T> uuv(BaseVector3<T>::Cross(quatVector, uv));

	return vec + ((uv * quat.w) + uuv) * static_cast<T>(2);
}

template <typename T>
__forceinline BaseVector4<T> operator*(const BaseVector4<T>& vec, const QuaternionBase<T>& quat)
{
	return quat.Inverse() * vec;
}

template <typename T>
__forceinline BaseVector4<T> operator*(const QuaternionBase<T>& quat, const BaseVector4<T>& vec)
{
	BaseVector3<T> quatVector(vec.x, vec.y, vec.z);
	return BaseVector4<T>(quat * quatVector, 1.0f);
}

template <typename T>
__forceinline QuaternionBase<T> operator/(const QuaternionBase<T>& first, float second)
{
	return QuaternionBase<T>(first) /= second;
}

typedef QuaternionBase<float> Quaternion;