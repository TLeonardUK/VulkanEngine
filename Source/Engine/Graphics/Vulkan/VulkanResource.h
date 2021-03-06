#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"

class IVulkanResource
{
public:
	virtual void FreeResources() = 0;
	virtual String GetName() = 0;

};