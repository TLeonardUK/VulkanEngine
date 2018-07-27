#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

#include "Engine/Graphics/GraphicsEnums.h"

class IGraphicsIndexBuffer
{
protected:
	IGraphicsIndexBuffer() { };

public:
	virtual ~IGraphicsIndexBuffer() { };

	virtual bool Stage(void* buffer, int offset, int length) = 0;

	virtual int GetCapacity() = 0;

};
