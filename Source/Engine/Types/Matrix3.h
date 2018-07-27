#pragma once
#include "Pch.h"

#include "Engine/Types/Quaternion.h"

template <typename T>
struct BaseMatrix3
{
public:
	float columns[3][3];

	// Constants.
	static const BaseMatrix3<T> Identity;
	static const BaseMatrix3<T> Zero;

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
		return BaseVector3<T>(
			columns[column][0],
			columns[column][1],
			columns[column][2]
		);
	}
	BaseVector4<T> GetRow(int row) const
	{
		return BaseVector3<T>(
			columns[0][row],
			columns[1][row],
			columns[2][row]
		);
	}
	void SetColumn(int column, const BaseVector3<T>& vec)
	{
		columns[column][0] = vec.x;
		columns[column][1] = vec.y;
		columns[column][2] = vec.z;
	}
	void SetRow(int row, const BaseVector3<T>& vec)
	{
		columns[0][row] = vec.x;
		columns[1][row] = vec.y;
		columns[2][row] = vec.z;
	}

	// Constructors
	BaseMatrix3() = default;
	BaseMatrix3(const BaseMatrix3& other) = default;
	BaseMatrix3(T x0, T y0, T z0, 
				T x1, T y1, T z1, 
				T x2, T y2, T z2)
	{
		columns[0][0] = x0;
		columns[0][1] = y0;
		columns[0][2] = z0;
		columns[1][0] = x1;
		columns[1][1] = y1;
		columns[1][2] = z1;
		columns[2][0] = x2;
		columns[2][1] = y2;
		columns[2][2] = z2;
	}

	// Operator overloads
	BaseMatrix3& operator=(const BaseMatrix3& vector) = default;
	BaseMatrix3& operator*=(T scalar)
	{
		columns[0][0] *= scalar;
		columns[0][1] *= scalar;
		columns[0][2] *= scalar;
		columns[1][0] *= scalar;
		columns[1][1] *= scalar;
		columns[1][2] *= scalar;
		columns[2][0] *= scalar;
		columns[2][1] *= scalar;
		columns[2][2] *= scalar;
		return *this;
	}
	BaseMatrix3& operator*=(const BaseMatrix3& other)
	{
		*this = (*this * other);
		return *this;
	}

	// Helper functions.
	QuaternionBase<T> ToQuaternion()
	{
		T fourXSquaredMinus1 = columns[0][0] - columns[1][1] - columns[2][2];
		T fourYSquaredMinus1 = columns[1][1] - columns[0][0] - columns[2][2];
		T fourZSquaredMinus1 = columns[2][2] - columns[0][0] - columns[1][1];
		T fourWSquaredMinus1 = columns[0][0] + columns[1][1] + columns[2][2];

		int biggestIndex = 0;
		T fourBiggestSquaredMinus1 = fourWSquaredMinus1;
		if (fourXSquaredMinus1 > fourBiggestSquaredMinus1)
		{
			fourBiggestSquaredMinus1 = fourXSquaredMinus1;
			biggestIndex = 1;
		}
		if (fourYSquaredMinus1 > fourBiggestSquaredMinus1)
		{
			fourBiggestSquaredMinus1 = fourYSquaredMinus1;
			biggestIndex = 2;
		}
		if (fourZSquaredMinus1 > fourBiggestSquaredMinus1)
		{
			fourBiggestSquaredMinus1 = fourZSquaredMinus1;
			biggestIndex = 3;
		}

		T biggestVal = sqrt(fourBiggestSquaredMinus1 + static_cast<T>(1)) * static_cast<T>(0.5);
		T mult = static_cast<T>(0.25) / biggestVal;

		switch (biggestIndex)
		{
		case 0:
			return QuaternionBase<T>((columns[1][2] - columns[2][1]) * mult, (columns[2][0] - columns[0][2]) * mult, (columns[0][1] - columns[1][0]) * mult, biggestVal);
		case 1:
			return QuaternionBase<T>(biggestVal, (columns[0][1] + columns[1][0]) * mult, (columns[2][0] + columns[0][2]) * mult, (columns[1][2] - columns[2][1]) * mult);
		case 2:
			return QuaternionBase<T>((columns[0][1] + columns[1][0]) * mult, biggestVal, (columns[1][2] + columns[2][1]) * mult, (columns[2][0] - columns[0][2]) * mult);
		case 3:
			return QuaternionBase<T>((columns[2][0] + columns[0][2]) * mult, (columns[1][2] + columns[2][1]) * mult, biggestVal, (columns[0][1] - columns[1][0]) * mult);
		default: 
			assert(false);
			return QuaternionBase<T>(0, 0, 0, 1);
		}
	}
};

template <typename T>
__forceinline BaseMatrix3<T> operator*(const BaseMatrix3<T>& first, const BaseMatrix3<T>& second)
{
	BaseMatrix2<T> result;

	result[0][0] = first[0][0] * second[0][0] + first[1][0] * second[0][1] + first[2][0] * second[0][2];
	result[0][1] = first[0][1] * second[0][0] + first[1][1] * second[0][1] + first[2][1] * second[0][2];
	result[0][2] = first[0][2] * second[0][0] + first[1][2] * second[0][1] + first[2][2] * second[0][2];
	result[1][0] = first[0][0] * second[1][0] + first[1][0] * second[1][1] + first[2][0] * second[1][2];
	result[1][1] = first[0][1] * second[1][0] + first[1][1] * second[1][1] + first[2][1] * second[1][2];
	result[1][2] = first[0][2] * second[1][0] + first[1][2] * second[1][1] + first[2][2] * second[1][2];
	result[2][0] = first[0][0] * second[2][0] + first[1][0] * second[2][1] + first[2][0] * second[2][2];
	result[2][1] = first[0][1] * second[2][0] + first[1][1] * second[2][1] + first[2][1] * second[2][2];
	result[2][2] = first[0][2] * second[2][0] + first[1][2] * second[2][1] + first[2][2] * second[2][2];

	return result;
}

template <typename T>
__forceinline BaseMatrix3<T> operator/(const BaseMatrix3<T>& first, float second)
{
	return BaseMatrix3<T>(first) /= second;
}

static_assert(sizeof(BaseMatrix3<float>) == 4 * 3 * 3, "BaseMatrix3 is not expected size. Data should be tightly packed.");

typedef BaseMatrix3<float> Matrix3;