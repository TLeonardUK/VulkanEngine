#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

class IGraphicsImage
{
protected:
	IGraphicsImage() { };

public:
	virtual ~IGraphicsImage() { };

	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;
	virtual int GetMipLevels() = 0;
	virtual int GetLayers() = 0;
	virtual GraphicsFormat GetFormat() = 0;
	virtual bool IsDepth() = 0;
	virtual bool IsStencil() = 0;
	virtual String GetImageName() = 0;

	virtual bool Stage(int layer, void* buffer, int offset, int length) = 0;

};
