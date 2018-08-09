#pragma once

/*
template <typename KeyType, typename ValueType>
struct HashArray
{
private:
	Dictionary<KeyType, uint64_t> m_allocatedEntries;
	Array<uint64_t> m_freeEntries;
	Array<ValueType> m_values;

public:
	void Reserve(int size)
	{
	}

	void Clear()
	{
	}

	int Length()
	{

	}

	void Insert(const KeyType& key, const ValueType& value)
	{
		if (m_freeEntries.size() == 0)
		{
			Reserve(m_values.size() * 2);
		}

		uint64_t index = m_freeEntries[m_freeEntries.size() - 1];
		m_freeEntries.pop_back();

		m_allocatedEntries[key] = index;

		return index;
	}

	void Remove(const KeyType& key)
	{
		auto iter = m_allocatedEntries.find(key);
		if (iter == m_allocatedEntries.end())
		{
			return;
		}

		m_freeEntries.push_back(iter.second);
		m_allocatedEntries.erase(iter);
	}
};*/