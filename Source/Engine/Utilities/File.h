#pragma once
#include "Pch.h"

#include "Engine/Types/Array.h"
#include "Engine/Types/String.h"

namespace File
{
	bool ReadAllBytes(const String& filename, Array<char>& output);
};