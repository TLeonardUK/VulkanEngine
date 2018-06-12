#pragma once

#include <string>

#include "Engine/Types/String.h"

#define enum_begin_declaration(Name) enum class Name {
#define enum_entry(Name)				Name,
#define enum_end_declaration(Name)	 COUNT };	\
extern const char* Name##_Strings[(int)Name::COUNT + 1];	\
															\
template <>													\
String EnumToString(Name value);							\
template <>													\
bool StringToEnum(const String& name, Name& value);			\

int FindEnumIndex(const char** names, const String& name);

template <typename T>
String EnumToString(T value)
{
	assert(false);
	return "";
}

template <typename T>
bool StringToEnum(const String& name, T& value)
{
	assert(false);
	return false;
}