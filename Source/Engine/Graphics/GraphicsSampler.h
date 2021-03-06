#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

struct SamplerDescription
{
public:
	GraphicsFilter MagnificationFilter;
	GraphicsFilter MinificationFilter;

	GraphicsAddressMode AddressModeU;
	GraphicsAddressMode AddressModeV;
	GraphicsAddressMode AddressModeW;

	GraphicsBorderColor BorderColor;

	GraphicsMipMapMode MipmapMode;

	float MinLod;
	float MaxLod;
	float MipLodBias;

	int MaxAnisotropy;

	SamplerDescription()
		: MagnificationFilter(GraphicsFilter::Linear)
		, MinificationFilter(GraphicsFilter::Linear)
		, AddressModeU(GraphicsAddressMode::ClampToEdge)
		, AddressModeV(GraphicsAddressMode::ClampToEdge)
		, AddressModeW(GraphicsAddressMode::ClampToEdge)
		, MaxAnisotropy(0)
		, MinLod(0.0f)
		, MaxLod(-1.0f)
		, MipLodBias(0.0f)
		, MipmapMode(GraphicsMipMapMode::Linear)
		, BorderColor(GraphicsBorderColor::OpaqueBlack)
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