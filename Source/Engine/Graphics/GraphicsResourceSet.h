#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

#include <memory>

class IGraphicsUniformBuffer;
class IGraphicsSampler;
class IGraphicsImageView;

class IGraphicsResourceSet
{
protected:
	IGraphicsResourceSet() { };

public:
	virtual ~IGraphicsResourceSet() { };

	virtual bool UpdateBinding(int location, int arrayIndex, std::shared_ptr<IGraphicsUniformBuffer> buffer) = 0;
	virtual bool UpdateBinding(int location, int arrayIndex, std::shared_ptr<IGraphicsSampler> sampler, std::shared_ptr<IGraphicsImageView> imageView) = 0;

};
