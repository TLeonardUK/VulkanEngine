#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

#include "Engine/Graphics/GraphicsEnums.h"

class IGraphicsUniformBuffer
{
protected:
	IGraphicsUniformBuffer() { };

public:
	virtual ~IGraphicsUniformBuffer() { };

	virtual bool Upload(void* buffer, int offset, int length) = 0;
};
