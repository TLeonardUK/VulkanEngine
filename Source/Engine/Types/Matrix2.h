#pragma once
#include "Pch.h"

#include "Engine/Types/Vector2.h"

template <typename T>
struct BaseMatrix2
{
public:
	float columns[2][2];

	// Constants.
	static const BaseMatrix2<T> Identity;
	static const BaseMatrix2<T> Zero;

	// Accessors.
	T* operator[](int column)
	{
		return columns[column];
	}
	const T* operator[](int column) const
	{
		return columns[column];
	}
	BaseVector2<T> GetColumn(int column) const
	{
		return BaseVector3<T>(
			columns[column][0],
			columns[column][1]
			);
	}
	BaseVector2<T> GetRow(int row) const
	{
		return BaseVector2<T>(
			columns[0][row],
			columns[1][row]
			);
	}
	void SetColumn(int column, const BaseVector2<T>& vec)
	{
		columns[column][0] = vec.x;
		columns[column][1] = vec.y;
	}
	void SetRow(int row, const BaseVector2<T>& vec)
	{
		columns[0][row] = vec.x;
		columns[1][row] = vec.y;
	}

	// Constructors
	BaseMatrix2() = default;
	BaseMatrix2(const BaseMatrix2& other) = default;
	BaseMatrix2(T x0, T y0,
				T x1, T y1)
	{
		columns[0][0] = x0;
		columns[0][1] = y0;
		columns[1][0] = x1;
		columns[1][1] = y1;
	}

	// Operator overloads
	BaseMatrix2& operator=(const BaseMatrix2& vector) = default;
	BaseMatrix2& operator*=(T scalar)
	{
		columns[0][0] *= scalar;
		columns[0][1] *= scalar;
		columns[1][0] *= scalar;
		columns[1][1] *= scalar;
		return *this;
	}
	BaseMatrix2& operator*=(const BaseMatrix2& other)
	{
		*this = (*this * other);
		return *this;
	}
};

template <typename T>
__forceinline BaseMatrix2<T> operator*(const BaseMatrix2<T>& first, const BaseMatrix2<T>& second)
{
	BaseMatrix2<T> result;
	result[0][0] = first[0][0] * second[0][0] + first[1][0] * second[0][1];
	result[0][1] = first[0][1] * second[0][0] + first[1][1] * second[0][1];
	result[1][0] = first[0][0] * second[1][0] + first[1][0] * second[1][1];
	result[1][1] = first[0][1] * second[1][0] + first[1][1] * second[1][1];
	return result;
}

template <typename T>
__forceinline BaseMatrix2<T> operator/(const BaseMatrix2<T>& first, float second)
{
	return BaseMatrix2<T>(first) /= second;
}


template <typename T>
__forceinline bool operator==(const BaseMatrix2<T>& first, const BaseMatrix2<T>& second)
{
	return
		first.columns[0][0] == second.columns[0][0] &&
		first.columns[0][1] == second.columns[0][1] &&

		first.columns[1][0] == second.columns[1][0] &&
		first.columns[1][1] == second.columns[1][1];
}

template <typename T>
__forceinline bool operator!=(const BaseMatrix2<T>& first, const BaseMatrix2<T>& second)
{
	return !(first == second);
}

static_assert(sizeof(BaseMatrix2<float>) == 4 * 2 * 2, "BaseMatrix2 is not expected size. Data should be tightly packed.");

typedef BaseMatrix2<float> Matrix2;