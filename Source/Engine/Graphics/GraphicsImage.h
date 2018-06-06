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

	virtual bool Stage(void* buffer, int offset, int length) = 0;

};
