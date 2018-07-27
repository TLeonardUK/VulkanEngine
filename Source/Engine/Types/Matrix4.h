#pragma once
#include "Pch.h"

#include "Engine/Types/Vector.h"
#include "Engine/Types/Quaternion.h"

template <typename T>
struct BaseMatrix4
{
public:
	float columns[4][4];

	// Constants.
	static const BaseMatrix4<T> Identity;
	static const BaseMatrix4<T> Zero;

	// Accessors.
	T* operator[](int column)
	{
		return columns[column];
	}
	const T* operator[](int column) const
	{
		return columns[column];
	}
	BaseVector4<T> GetColumn(int column) const
	{
		return BaseVector4<T>(
			columns[column][0],
			columns[column][1],
			columns[column][2],
			columns[column][3]
		);
	}
	BaseVector4<T> GetRow(int row) const
	{
		return BaseVector4<T>(
			columns[0][row],
			columns[1][row],
			columns[2][row],
			columns[3][row]
			);
	}
	void SetColumn(int column, const BaseVector4<T>& vec)
	{
		columns[column][0] = vec.x;
		columns[column][1] = vec.y;
		columns[column][2] = vec.z;
		columns[column][3] = vec.w;
	}
	void SetRow(int row, const BaseVector4<T>& vec)
	{
		columns[0][row] = vec.x;
		columns[1][row] = vec.y;
		columns[2][row] = vec.z;
		columns[3][row] = vec.w;
	}

	// Constructors
	BaseMatrix4() = default;
	BaseMatrix4(const BaseMatrix4& other) = default;
	BaseMatrix4(T x0, T y0, T z0, T w0,
				T x1, T y1, T z1, T w1,
				T x2, T y2, T z2, T w2,
				T x3, T y3, T z3, T w3)
	{
		columns[0][0] = x0;
		columns[0][1] = y0;
		columns[0][2] = z0;
		columns[0][3] = w0;

		columns[1][0] = x1;
		columns[1][1] = y1;
		columns[1][2] = z1;
		columns[1][3] = w1;

		columns[2][0] = x2;
		columns[2][1] = y2;
		columns[2][2] = z2;
		columns[2][3] = w2;

		columns[3][0] = x3;
		columns[3][1] = y3;
		columns[3][2] = z3;
		columns[3][3] = w3;
	}

	BaseMatrix4(BaseVector4<T> col1,
				BaseVector4<T> col2,
				BaseVector4<T> col3,
				BaseVector4<T> col4)
	{
		columns[0][0] = col1.x;
		columns[0][1] = col1.y;
		columns[0][2] = col1.z;
		columns[0][3] = col1.w;

		columns[1][0] = col2.x;
		columns[1][1] = col2.y;
		columns[1][2] = col2.z;
		columns[1][3] = col2.w;

		columns[2][0] = col3.x;
		columns[2][1] = col3.y;
		columns[2][2] = col3.z;
		columns[2][3] = col3.w;

		columns[3][0] = col4.x;
		columns[3][1] = col4.y;
		columns[3][2] = col4.z;
		columns[3][3] = col4.w;
	}

	// Operator overloads
	BaseMatrix4& operator=(const BaseMatrix4& vector) = default;
	BaseMatrix4& operator*=(T scalar)
	{
		columns[0][0] *= scalar;
		columns[0][1] *= scalar;
		columns[0][2] *= scalar;
		columns[0][3] *= scalar;

		columns[1][0] *= scalar;
		columns[1][1] *= scalar;
		columns[1][2] *= scalar;
		columns[1][3] *= scalar;

		columns[2][0] *= scalar;
		columns[2][1] *= scalar;
		columns[2][2] *= scalar;
		columns[2][3] *= scalar;

		columns[3][0] *= scalar;
		columns[3][1] *= scalar;
		columns[3][2] *= scalar;
		columns[3][3] *= scalar;
		return *this;
	}
	BaseMatrix4& operator*=(const BaseMatrix4& other)
	{
		*this = (*this * other);
		return *this;
	}

	// Helper methods.

	// This is lossy.
	BaseVector3<T> ExtractScale() const
	{
		return BaseVector3<T>(
			GetColumn(0).Length(),
			GetColumn(1).Length(),
			GetColumn(2).Length()
		);
	}

	BaseVector3<T> ExtractRotation() const
	{
		/*return LookRotation()
		QuaternionBase<T> q;
		q.w = sqrt(Math::Max(0, 1 + m[0][0] + m[1][1] + m[2][2])) / 2;
		q.x = sqrt(Math::Max(0, 1 + m[0][0] - m[1][1] - m[2][2])) / 2;
		q.y = sqrt(Math::Max(0, 1 - m[0][0] + m[1][1] - m[2][2])) / 2;
		q.z = sqrt(Math::Max(0, 1 - m[0][0] - m[1][1] + m[2][2])) / 2;
		q.x *= Math::Sign(q.x * (m[2][1] - m[1][2]));
		q.y *= Math::Sign(q.y * (m[0][2] - m[2][0]));
		q.z *= Math::Sign(q.z * (m[1][0] - m[0][1]));
		return result;*/
		return ToQuaternion();
	}

	// Translate without translation or scaling.
	BaseVector3<T> TransformDirection(const BaseVector3<T>& vec) const
	{
		BaseVector4<T> result(
		           	 1.0f * vec.x + columns[1][0] * vec.y + columns[2][0] * vec.z + columns[3][0] * 0.0f,
			columns[0][1] * vec.x +          1.0f * vec.y + columns[2][1] * vec.z + columns[3][1] * 0.0f,
			columns[0][2] * vec.x + columns[1][2] * vec.y +          1.0f * vec.z + columns[3][2] * 0.0f,
			columns[0][3] * vec.x + columns[1][3] * vec.y + columns[2][3] * vec.z +          1.0f * 0.0f);

		return BaseVector3<T>(result.x, result.y, result.z);
	}

	// Transform without translation.
	BaseVector3<T> TransformVector(const BaseVector3<T>& vec) const
	{
		BaseVector4<T> result(
			columns[0][0] * vec.x + columns[1][0] * vec.y + columns[2][0] * vec.z + columns[3][0] * 0.0f,
			columns[0][1] * vec.x + columns[1][1] * vec.y + columns[2][1] * vec.z + columns[3][1] * 0.0f,
			columns[0][2] * vec.x + columns[1][2] * vec.y + columns[2][2] * vec.z + columns[3][2] * 0.0f,
			columns[0][3] * vec.x + columns[1][3] * vec.y + columns[2][3] * vec.z + columns[3][3] * 0.0f);

		return BaseVector3<T>(result.x, result.y, result.z);
	}

	// Translate point.
	BaseVector3<T> TransformLocation(const BaseVector3<T>& vec) const
	{
		BaseVector4<T> result(
			columns[0][0] * vec.x + columns[1][0] * vec.y + columns[2][0] * vec.z + columns[3][0] * 1.0f,
			columns[0][1] * vec.x + columns[1][1] * vec.y + columns[2][1] * vec.z + columns[3][1] * 1.0f,
			columns[0][2] * vec.x + columns[1][2] * vec.y + columns[2][2] * vec.z + columns[3][2] * 1.0f,
			columns[0][3] * vec.x + columns[1][3] * vec.y + columns[2][3] * vec.z + columns[3][3] * 1.0f);

		return BaseVector3<T>(result.x, result.y, result.z);
	}

	BaseMatrix4 Inverse() const
	{
		T coef00 = columns[2][2] * columns[3][3] - columns[3][2] * columns[2][3];
		T coef02 = columns[1][2] * columns[3][3] - columns[3][2] * columns[1][3];
		T coef03 = columns[1][2] * columns[2][3] - columns[2][2] * columns[1][3];

		T coef04 = columns[2][1] * columns[3][3] - columns[3][1] * columns[2][3];
		T coef06 = columns[1][1] * columns[3][3] - columns[3][1] * columns[1][3];
		T coef07 = columns[1][1] * columns[2][3] - columns[2][1] * columns[1][3];

		T coef08 = columns[2][1] * columns[3][2] - columns[3][1] * columns[2][2];
		T coef10 = columns[1][1] * columns[3][2] - columns[3][1] * columns[1][2];
		T coef11 = columns[1][1] * columns[2][2] - columns[2][1] * columns[1][2];

		T coef12 = columns[2][0] * columns[3][3] - columns[3][0] * columns[2][3];
		T coef14 = columns[1][0] * columns[3][3] - columns[3][0] * columns[1][3];
		T coef15 = columns[1][0] * columns[2][3] - columns[2][0] * columns[1][3];

		T coef16 = columns[2][0] * columns[3][2] - columns[3][0] * columns[2][2];
		T coef18 = columns[1][0] * columns[3][2] - columns[3][0] * columns[1][2];
		T coef19 = columns[1][0] * columns[2][2] - columns[2][0] * columns[1][2];

		T coef20 = columns[2][0] * columns[3][1] - columns[3][0] * columns[2][1];
		T coef22 = columns[1][0] * columns[3][1] - columns[3][0] * columns[1][1];
		T coef23 = columns[1][0] * columns[2][1] - columns[2][0] * columns[1][1];

		BaseVector4<T> fac0(coef00, coef00, coef02, coef03);
		BaseVector4<T> fac1(coef04, coef04, coef06, coef07);
		BaseVector4<T> fac2(coef08, coef08, coef10, coef11);
		BaseVector4<T> fac3(coef12, coef12, coef14, coef15);
		BaseVector4<T> fac4(coef16, coef16, coef18, coef19);
		BaseVector4<T> fac5(coef20, coef20, coef22, coef23);

		BaseVector4<T> vec0(columns[1][0], columns[0][0], columns[0][0], columns[0][0]);
		BaseVector4<T> vec1(columns[1][1], columns[0][1], columns[0][1], columns[0][1]);
		BaseVector4<T> vec2(columns[1][2], columns[0][2], columns[0][2], columns[0][2]);
		BaseVector4<T> vec3(columns[1][3], columns[0][3], columns[0][3], columns[0][3]);

		BaseVector4<T> inv0(vec1 * fac0 - vec2 * fac1 + vec3 * fac2);
		BaseVector4<T> inv1(vec0 * fac0 - vec2 * fac3 + vec3 * fac4);
		BaseVector4<T> inv2(vec0 * fac1 - vec1 * fac3 + vec3 * fac5);
		BaseVector4<T> inv3(vec0 * fac2 - vec1 * fac4 + vec2 * fac5);

		BaseVector4<T> signA(+1, -1, +1, -1);
		BaseVector4<T> signB(-1, +1, -1, +1);
		BaseMatrix4<T> inverse(inv0 * signA, inv1 * signB, inv2 * signA, inv3 * signB);

		BaseVector4<T> row0(inverse[0][0], inverse[1][0], inverse[2][0], inverse[3][0]);

		BaseVector4<T> dot0(
			columns[0][0] * row0.x,
			columns[0][1] * row0.y,
			columns[0][2] * row0.z,
			columns[0][3] * row0.w);
		T dot1 = (dot0.x + dot0.y) + (dot0.z + dot0.w);

		T OneOverDeterminant = static_cast<T>(1) / dot1;

		return inverse * OneOverDeterminant;
	}

	QuaternionBase<T> ToQuaternion() const
	{
		return BaseMatrix3<T>(
			columns[0][0], columns[0][1], columns[0][2],
			columns[1][0], columns[1][1], columns[1][2],
			columns[2][0], columns[2][1], columns[2][2]
		).ToQuaternion();
	}

	BaseMatrix4 Transpose() const
	{
		return BaseMatrix4<T>(
			columns[0][0],
			columns[1][0],
			columns[2][0],
			columns[3][0],

			columns[0][1],
			columns[1][1],
			columns[2][1],
			columns[3][1],

			columns[0][2],
			columns[1][2],
			columns[2][2],
			columns[3][2],

			columns[0][3],
			columns[1][3],
			columns[2][3],
			columns[3][3]
		);
	}

	// Helper constructors.
	static BaseMatrix4 Translate(const Vector3& position)
	{
		BaseMatrix4 result = Identity;
		result[3][0] = position.x;
		result[3][1] = position.y;
		result[3][2] = position.z;
		return result;
	}
	static BaseMatrix4 Scale(const Vector3& scale)
	{
		BaseMatrix4 result = Identity;
		result[0][0] = scale.x;
		result[1][1] = scale.y;
		result[2][2] = scale.z;
		return result;
	}
	static BaseMatrix4 Rotation(const Quaternion& quat)
	{
		BaseMatrix4 result = Identity;

		T qxx(quat.x * quat.x);
		T qyy(quat.y * quat.y);
		T qzz(quat.z * quat.z);
		T qxz(quat.x * quat.z);
		T qxy(quat.x * quat.y);
		T qyz(quat.y * quat.z);
		T qwx(quat.w * quat.x);
		T qwy(quat.w * quat.y);
		T qwz(quat.w * quat.z);

		result[0][0] = T(1) - T(2) * (qyy + qzz);
		result[0][1] = T(2) * (qxy + qwz);
		result[0][2] = T(2) * (qxz - qwy);
		result[0][3] = T(0);

		result[1][0] = T(2) * (qxy - qwz);
		result[1][1] = T(1) - T(2) * (qxx + qzz);
		result[1][2] = T(2) * (qyz + qwx);
		result[1][3] = T(0);

		result[2][0] = T(2) * (qxz + qwy);
		result[2][1] = T(2) * (qyz - qwx);
		result[2][2] = T(1) - T(2) * (qxx + qyy);
		result[2][3] = T(0);

		result[3][0] = T(0);
		result[3][1] = T(0);
		result[3][2] = T(0);
		result[3][3] = T(1);

		return result;
	}
	static BaseMatrix4 LookAt(const BaseVector3<T>& eye, const BaseVector3<T>& center, const BaseVector3<T>& up)
	{
		// Right handed

		BaseVector3<T> f = (center - eye).Normalize();
		BaseVector3<T> s = BaseVector3<T>::Cross(f, up).Normalize();
		BaseVector3<T> u(BaseVector3<T>::Cross(s, f));

		BaseMatrix4<T> result = BaseMatrix4<T>::Identity;
		result[0][0] = s.x;
		result[1][0] = s.y;
		result[2][0] = s.z;
		result[0][1] = u.x;
		result[1][1] = u.y;
		result[2][1] = u.z;
		result[0][2] = -f.x;
		result[1][2] = -f.y;
		result[2][2] = -f.z;
		result[3][0] = -BaseVector3<T>::Dot(s, eye);
		result[3][1] = -BaseVector3<T>::Dot(u, eye);
		result[3][2] =  BaseVector3<T>::Dot(f, eye);

		return result;
	}
	static BaseMatrix4 Perspective(T fovRadians, T aspect, T zNear, T zFar)
	{
		// Right handed
		/*
		T tanHalfFovy = static_cast<T>(tan(fovRadians / static_cast<T>(2)));

		BaseMatrix4<T> result = BaseMatrix4<T>::Zero;
		result[0][0] = static_cast<T>(1) / (aspect * tanHalfFovy);
		result[1][1] = static_cast<T>(1) / (tanHalfFovy);
		result[2][2] = -(zFar + zNear) / (zFar - zNear);
		result[2][3] = -static_cast<T>(1);
		result[3][2] = -(static_cast<T>(2) * zFar * zNear) / (zFar - zNear);
		*/

		T f = 1.0f / tan(0.5f * fovRadians);

		BaseMatrix4<T> result = BaseMatrix4<T>(
			-f / aspect,
			0.0f,
			0.0f,
			0.0f,

			0.0f,
			-f,
			0.0f,
			0.0f,

			0.0f,
			0.0f,
			zFar / (zNear - zFar),
			-1.0f,

			0.0f,
			0.0f,
			(zNear * zFar) / (zNear - zFar),
			0.0f
		);

		return result;
	}
};

template <typename T>
__forceinline BaseMatrix4<T> operator*(const BaseMatrix4<T>& first, const BaseMatrix4<T>& second)
{
	BaseMatrix4<T> result;

	result[0][0] = first[0][0] * second[0][0] + first[1][0] * second[0][1] + first[2][0] * second[0][2] + first[3][0] * second[0][3];
	result[0][1] = first[0][1] * second[0][0] + first[1][1] * second[0][1] + first[2][1] * second[0][2] + first[3][1] * second[0][3];
	result[0][2] = first[0][2] * second[0][0] + first[1][2] * second[0][1] + first[2][2] * second[0][2] + first[3][2] * second[0][3];
	result[0][3] = first[0][3] * second[0][0] + first[1][3] * second[0][1] + first[2][3] * second[0][2] + first[3][3] * second[0][3];

	result[1][0] = first[0][0] * second[1][0] + first[1][0] * second[1][1] + first[2][0] * second[1][2] + first[3][0] * second[1][3];
	result[1][1] = first[0][1] * second[1][0] + first[1][1] * second[1][1] + first[2][1] * second[1][2] + first[3][1] * second[1][3];
	result[1][2] = first[0][2] * second[1][0] + first[1][2] * second[1][1] + first[2][2] * second[1][2] + first[3][2] * second[1][3];
	result[1][3] = first[0][3] * second[1][0] + first[1][3] * second[1][1] + first[2][3] * second[1][2] + first[3][3] * second[1][3];

	result[2][0] = first[0][0] * second[2][0] + first[1][0] * second[2][1] + first[2][0] * second[2][2] + first[3][0] * second[2][3];
	result[2][1] = first[0][1] * second[2][0] + first[1][1] * second[2][1] + first[2][1] * second[2][2] + first[3][1] * second[2][3];
	result[2][2] = first[0][2] * second[2][0] + first[1][2] * second[2][1] + first[2][2] * second[2][2] + first[3][2] * second[2][3];
	result[2][3] = first[0][3] * second[2][0] + first[1][3] * second[2][1] + first[2][3] * second[2][2] + first[3][3] * second[2][3];

	result[3][0] = first[0][0] * second[3][0] + first[1][0] * second[3][1] + first[2][0] * second[3][2] + first[3][0] * second[3][3];
	result[3][1] = first[0][1] * second[3][0] + first[1][1] * second[3][1] + first[2][1] * second[3][2] + first[3][1] * second[3][3];
	result[3][2] = first[0][2] * second[3][0] + first[1][2] * second[3][1] + first[2][2] * second[3][2] + first[3][2] * second[3][3];
	result[3][3] = first[0][3] * second[3][0] + first[1][3] * second[3][1] + first[2][3] * second[3][2] + first[3][3] * second[3][3];

	return result;
}

template <typename T>
__forceinline BaseMatrix4<T> operator*(const BaseMatrix4<T>& first, float second)
{
	BaseMatrix4<T> result = first;
	result *= second;
	return result;
}

template <typename T>
__forceinline BaseVector3<T> operator*(const BaseVector3<T>& vec, const BaseMatrix4<T>& mat)
{
	return mat.TransformLocation(vec);
}

template <typename T>
__forceinline BaseMatrix4<T> operator/(const BaseMatrix4<T>& first, float second)
{
	return BaseMatrix4<T>(first) /= second;
}

static_assert(sizeof(BaseMatrix4<float>) == 4 * 4 * 4, "BaseMatrix4 is not expected size. Data should be tightly packed.");

typedef BaseMatrix4<float> Matrix4;