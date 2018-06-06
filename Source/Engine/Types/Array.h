#pragma once

#include <vector>

template <typename T>
class Array : private std::vector<T>
{
public:
	typedef std::vector<T> Base;

	using Base::vector;
	using Base::at;
	using Base::clear;
	using Base::iterator;
	using Base::const_iterator;
	using Base::begin;
	using Base::end;
	using Base::cbegin;
	using Base::cend;
	using Base::crbegin;
	using Base::crend;
	using Base::empty;
	using Base::size;
	using Base::data;
	using Base::reserve;
	using Base::operator[];
	using Base::assign;
	using Base::insert;
	using Base::erase;
	using Base::front;
	using Base::back;
	using Base::push_back;
	using Base::pop_back;
	using Base::resize;

public:
	Array(std::initializer_list<T> list)
		: std::vector<T>(list)
	{
	}

	Array()
		: std::vector<T>()
	{
	}

	~Array()
	{
	}

};