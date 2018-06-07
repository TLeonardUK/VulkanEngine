#pragma once

#include "Engine/Types/Array.h"
#include "Engine/Types/String.h"

struct File
{
	static bool ReadAllBytes(const String& filename, Array<char>& output);
};