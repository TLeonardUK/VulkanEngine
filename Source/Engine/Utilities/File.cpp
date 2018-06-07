#pragma once

#include "Engine/Utilities/File.h"

#include <stdio.h>
#include <fstream>

bool File::ReadAllBytes(const String& filename, Array<char>& buffer)
{
	std::ifstream file(filename.c_str(), std::ios::ate | std::ios::binary);

	if (!file.is_open()) 
	{
		return false;
	}

	size_t fileSize = (size_t)file.tellg();

	buffer.resize(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return true;
}
