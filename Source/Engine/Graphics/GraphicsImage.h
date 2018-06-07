#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

#include <memory>

class IGraphicsImage
{
protected:
	IGraphicsImage() { };

public:
	virtual ~IGraphicsImage() { };

	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;
	virtual int GetMipLevels() = 0;
	virtual GraphicsFormat GetFormat() = 0;
	virtual bool IsDepth() = 0;

	virtual bool Stage(void* buffer, int offset, int length) = 0;

};
