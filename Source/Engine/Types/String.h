#pragma once

#include <string>
#include <stdarg.h>

class String : private std::string
{
public:
	typedef std::string Base;

	using Base::append;
	using Base::assign;
	using Base::at;
	using Base::back;
	using Base::basic_string;
	using Base::begin;
	using Base::capacity;
	using Base::cbegin;
	using Base::cend;
	using Base::clear;
	using Base::compare;
	using Base::const_iterator;
	using Base::const_pointer;
	using Base::const_reference;
	using Base::const_reverse_iterator;
	using Base::copy;
	using Base::crbegin;
	using Base::crend;
	using Base::c_str;
	using Base::data;
	using Base::difference_type;
	using Base::empty;
	using Base::end;
	using Base::erase;
	using Base::find;
	using Base::find_first_not_of;
	using Base::find_last_not_of;
	using Base::find_last_of;
	using Base::front;
	using Base::get_allocator;
	using Base::insert;
	using Base::iterator;
	using Base::length;
	using Base::max_size;
	using Base::npos;
	using Base::operator+=;
	using Base::operator=;
	using Base::operator[];
	using Base::pointer;
	using Base::pop_back;
	using Base::push_back;
	using Base::rbegin;
	using Base::reference;
	using Base::rend;
	using Base::replace;
	using Base::reserve;
	using Base::resize;
	using Base::reverse_iterator;
	using Base::rfind;
	using Base::shrink_to_fit;
	using Base::size;
	using Base::size_type;
	using Base::substr;
	using Base::swap;
	using Base::traits_type;
	using Base::value_type;

public:
	String()
		: std::string()
	{
	}

	~String()
	{
	}

	static String Format(const String& Format, va_list List)
	{
		String buffer;
		int bufferSize = 1024;

		while (true)
		{
			buffer.resize(bufferSize);

			va_list listCopy;
			va_copy(listCopy, List);
			int n = vsnprintf((char*)buffer.data(), buffer.size(), Format.c_str(), listCopy);
			va_end(listCopy);

			if (n > bufferSize)
			{
				bufferSize = n;
			}
			else if (n < 0)
			{
				// failed, abort.
				return "";
			}
			else
			{
				buffer.resize(n);
				break;
			}
		}

		return buffer;
	}

	static String Format(String Format, ...)
	{
		va_list list;
		va_start(list, Format);
		String buffer = String::Format(Format, list);
		va_end(list);
		return buffer;
	}
};