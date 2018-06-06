#pragma once

#include "Engine/Types/Array.h"
#include "Engine/Types/String.h"

// StreamingManager::BeginLoad<Image>("@Test.var")
// StreamingManager::AwaitLoad(x);
// StreamingManager::WaitUntilIdle();


struct File
{
	static Array<char> ReadAllBytes(const String& filename);
};