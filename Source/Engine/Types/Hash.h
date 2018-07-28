#include "Pch.h"

template <class T>
inline void CombineHash(std::size_t& seed, T v)
{
	std::hash<T> hasher;
	size_t hash = hasher(v);
	seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
