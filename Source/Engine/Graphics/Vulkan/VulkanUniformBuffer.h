#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Engine/Logging.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/GraphicsUniformBuffer.h"
#include "Engine/Graphics/Vulkan/VulkanMemoryAllocator.h"
#include "Engine/Graphics/Vulkan/VulkanResource.h"

#include <vulkan/vulkan.h>

class VulkanMemoryAllocator;

class VulkanUniformBuffer 
	: public IGraphicsUniformBuffer
	, public IVulkanResource
{
private:
	String m_name;
	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<VulkanMemoryAllocator> m_memoryAllocator;

	int m_memorySize;

	VkDevice m_device;

	VulkanUniformBufferAllocation m_buffer;

private:
	friend class VulkanGraphics;
	friend class VulkanCommandBuffer;
	friend class VulkanResourceSet;
	friend class VulkanResourceSetPool;
	friend struct VulkanResourceSetBinding;

	bool Build(int bufferSize);

	VkBuffer GetGpuBuffer();
	uint32_t GetGpuBufferOffset();
	int GetDataSize();

public:
	VulkanUniformBuffer(
		VkDevice device,
		std::shared_ptr<Logger> logger,
		const String& name,
		std::shared_ptr<VulkanMemoryAllocator> memoryAllocator);

	virtual ~VulkanUniformBuffer();

	virtual bool Upload(void* buffer, int offset, int length);

	virtual void FreeResources();
	virtual String GetName();

};