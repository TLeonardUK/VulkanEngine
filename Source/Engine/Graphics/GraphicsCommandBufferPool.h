#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

#include <memory>

class IGraphicsCommandBuffer;

class IGraphicsCommandBufferPool
{
protected:
	IGraphicsCommandBufferPool() { };

public:
	virtual ~IGraphicsCommandBufferPool() { };

	virtual std::shared_ptr<IGraphicsCommandBuffer> Allocate() = 0;

};
