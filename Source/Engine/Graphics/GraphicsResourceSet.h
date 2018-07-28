#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

class IGraphicsUniformBuffer;
class IGraphicsSampler;
class IGraphicsImageView;
class IGraphicsResourceSetInstance;

class IGraphicsResourceSet
{
protected:
	IGraphicsResourceSet() { };

public:
	virtual ~IGraphicsResourceSet() { };

	virtual bool UpdateBinding(int location, int arrayIndex, std::shared_ptr<IGraphicsUniformBuffer> buffer) = 0;
	virtual bool UpdateBinding(int location, int arrayIndex, std::shared_ptr<IGraphicsSampler> sampler, std::shared_ptr<IGraphicsImageView> imageView) = 0;

	virtual std::shared_ptr<IGraphicsResourceSetInstance> NewInstance() = 0;
	virtual void UpdateInstance(std::shared_ptr<IGraphicsResourceSetInstance>& instance) = 0;

};
