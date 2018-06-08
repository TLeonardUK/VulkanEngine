#pragma once

#include <string>

#include "Engine/Types/String.h"

#define enum_begin_declaration(Name) enum class Name {
#define enum_entry(Name)				Name,
#define enum_end_declaration(Name)	 COUNT };	\
extern const char* Name##_Strings[(int)Name::COUNT + 1];	\
															\
template <>													\
bool EnumToString(Name value, String& name);				\
template <>													\
bool StringToEnum(const String& name, Name& value);			\

int FindEnumIndex(const char** names, const String& name);

template <typename T>
bool EnumToString(T value, String& name)
{
	assert(false);
	return false;
}

template <typename T>
bool StringToEnum(const String& name, T& value)
{
	assert(false);
	return false;
}