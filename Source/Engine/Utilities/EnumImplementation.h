#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"

#ifdef enum_entry
#undef enum_entry
#endif

#define enum_begin_implementation(Name) const char* Name##_Strings[] = {
#define enum_entry(Name)				#Name,
#define enum_end_implementation(Name)	"" };		\
															\
template <>													\
String EnumToString(Name value)								\
{															\
	return Name##_Strings[(int)value];						\
}															\
															\
template <>													\
bool StringToEnum(const String& name, Name& value)			\
{															\
	int index = FindEnumIndex(Name##_Strings, name);		\
	if (index < 0)											\
	{														\
		return false;										\
	}														\
	value = (Name)index;									\
	return true;											\
}

