#pragma once

#include "Engine/Streaming/File.h"

#include <stdio.h>
#include <fstream>

Array<char> File::ReadAllBytes(const String& filename)
{
	std::ifstream file(filename.c_str(), std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	Array<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}
