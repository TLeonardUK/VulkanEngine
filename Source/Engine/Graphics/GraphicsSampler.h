#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

#include <memory>

struct SamplerDescription
{
public:
	GraphicsFilter MagnificationFilter;
	GraphicsFilter MinificationFilter;

	GraphicsAddressMode AddressModeU;
	GraphicsAddressMode AddressModeV;
	GraphicsAddressMode AddressModeW;

	int MaxAnisotropy;

	SamplerDescription()
		: MagnificationFilter(GraphicsFilter::Linear)
		, MinificationFilter(GraphicsFilter::Linear)
		, AddressModeU(GraphicsAddressMode::ClampToEdge)
		, AddressModeV(GraphicsAddressMode::ClampToEdge)
		, AddressModeW(GraphicsAddressMode::ClampToEdge)
		, MaxAnisotropy(0)
	{
	}
};

class IGraphicsSampler
{
protected:
	IGraphicsSampler() { };

public:
	virtual ~IGraphicsSampler() { };

};