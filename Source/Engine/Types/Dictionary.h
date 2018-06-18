#pragma once

#include <unordered_map>

template <typename Key, typename Value>
using Dictionary = std::unordered_map<Key, Value>;

template <typename Key, typename Value>
using MultiDictionary = std::unordered_multimap<Key, Value>;


