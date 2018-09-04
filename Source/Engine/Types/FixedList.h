#pragma once
#include "Pch.h"

template <int Capacity, typename ValueType>
struct FixedList
{
private:
	ValueType m_values[Capacity];

	int m_freeIndexList[Capacity];
	int m_freeIndexCount;

	int m_allocIndexList[Capacity];
	int m_allocIndexCount;

public:
	FixedList()
		: m_freeIndexCount(Capacity)
		, m_allocIndexCount(0)
	{
		for (int i = 0; i < Capacity; i++)
		{
			m_freeIndexList[i] = i;
		}
	}

	ValueType& operator[](int index)
	{
		return m_values[m_allocIndexList[index]];
	}

	const ValueType& operator[](int index) const
	{
		return m_values[m_allocIndexList[index]];
	}

	int Size()
	{
		return m_allocIndexCount;
	}

	void Add(const ValueType& value)
	{
		assert(m_freeIndexCount > 0);
	
		int index = m_freeIndexList[--m_freeIndexCount];
		m_values[index] = value;

		m_allocIndexList[m_allocIndexCount++] = index;
	}

	void Remove(const ValueType& value)
	{
		for (int i = 0; i < m_allocIndexCount; i++)
		{
			int index = m_allocIndexList[i];
			if (m_values[i] == value)
			{
				m_allocIndexList[i] = m_allocIndexList[--m_allocIndexCount];
				m_freeIndexList[m_allocIndexCount++] = index;
			}
		}
	}

};
