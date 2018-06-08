#pragma once

#include "Engine/Utilities/Enum.h"

int FindEnumIndex(const char** names, const String& name)
{
	for (int i = 0; ; i++)
	{
		if (names[i][0] == '\0')
		{
			break;
		}
		if (names[i] == name)
		{
			return i;
		}
	}
	return -1;
}