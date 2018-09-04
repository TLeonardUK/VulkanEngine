#pragma once
#include "Pch.h"

#include "Engine/Resources/ResourceLoader.h"
#include "Engine/Resources/Resource.h"

#include "Engine/Rendering/RenderProperty.h"
#include "Engine/Rendering/RendererEnums.h"

#include "Engine/Graphics/GraphicsPipeline.h"

#include "Engine/Utilities/Enum.h"

struct RenderPropertyCollection;

struct UniformBufferLayoutField
{
public:
	String Name;
	GraphicsBindingFormat Format;
	String BindTo;
	RenderPropertyHash BindToHash;

	int ArrayLength;

};

struct UniformBufferLayout
{
private:
	Array<char> m_dataBuffer;

public:
	size_t HashCode;

	String Name;
	Array<UniformBufferLayoutField> Fields;

public:
	void CalculateHashCode();

	int GetSize() const;
	int GetFieldOffset(int fieldIndex, int elementIndex) const;

	void FillBuffer(std::shared_ptr<Logger> logger, std::shared_ptr<IGraphicsUniformBuffer> buffer, RenderPropertyCollection** collections, int collectionCount);

};
