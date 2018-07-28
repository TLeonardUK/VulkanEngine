#pragma once
#include "Pch.h"

#include "Engine/Resources/Types/MaterialPropertyCollection.h"
#include "Engine/Resources/ResourceLoader.h"
#include "Engine/Resources/Resource.h"
#include "Engine/Rendering/RendererEnums.h"

#include "Engine/Graphics/GraphicsPipeline.h"

#include "Engine/Utilities/Enum.h"

struct UniformBufferLayoutField
{
public:
	String Name;
	GraphicsBindingFormat Format;
	String BindTo;
	MaterialPropertyHash BindToHash;
	int Location;

};

struct UniformBufferLayout
{
private:
	Array<char> m_dataBuffer;

public:
	size_t HashCode;

	String Name;
	GraphicsBindingFrequency Frequency;
	Array<UniformBufferLayoutField> Fields;

public:
	void CalculateHashCode();

	int GetSize() const;
	int GetFieldOffset(int fieldIndex) const;

	void FillBuffer(std::shared_ptr<Logger> logger, std::shared_ptr<IGraphicsUniformBuffer> buffer, MaterialPropertyCollection** collections, int collectionCount);

};
