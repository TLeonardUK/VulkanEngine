#pragma once
#include "Pch.h"

template <typename ValueType>
class StackAllocator
{
public:
	std::atomic<int> m_top;
	std::vector<ValueType> m_values;

public:
	StackAllocator()
	{
	}

	StackAllocator(const StackAllocator& other)
	{
	}

	void Reset(int capacity)
	{
		m_top = 0;
		m_values.resize(capacity);
	}

	ValueType* New()
	{
		int index = m_top++;
		assert(index < m_values.size());
		return &m_values[index];
	}

};