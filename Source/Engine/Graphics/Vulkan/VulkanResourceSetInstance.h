#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

#include "Engine/Graphics/GraphicsResourceSetInstance.h"
#include "Engine/Graphics/Vulkan/VulkanGraphics.h"

class VulkanResourceSetInstance 
	: public IGraphicsResourceSetInstance
{
private:
	friend class VulkanCommandBuffer;
	friend class VulkanResourceSet;

	VkDescriptorSet m_sets[VulkanGraphics::MAX_BOUND_DESCRIPTOR_SETS];
	int m_setCount;

	uint32_t m_uniformBufferOffsets[VulkanGraphics::MAX_BOUND_UBO];
	int m_uboCount;

public:
	VulkanResourceSetInstance();

};
