#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

#include "Engine/Graphics/GraphicsEnums.h"

#include <memory>

class IGraphicsIndexBuffer
{
protected:
	IGraphicsIndexBuffer() { };

public:
	virtual ~IGraphicsIndexBuffer() { };

	virtual bool Stage(void* buffer, int offset, int length) = 0;

};
