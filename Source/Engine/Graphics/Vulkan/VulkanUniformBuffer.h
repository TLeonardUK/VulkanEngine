#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Engine/Logging.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/GraphicsUniformBuffer.h"
#include "Engine/Graphics/Vulkan/VulkanMemoryAllocator.h"

#include <vulkan/vulkan.h>

class VulkanMemoryAllocator;

class VulkanUniformBuffer : public IGraphicsUniformBuffer
{
private:
	String m_name;
	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<VulkanMemoryAllocator> m_memoryAllocator;

	int m_memorySize;

	VkDevice m_device;
	VulkanAllocation m_gpuBuffer;

private:
	friend class VulkanGraphics;
	friend class VulkanCommandBuffer;
	friend class VulkanResourceSet;

	bool Build(int bufferSize);
	void FreeResources();

	VulkanAllocation GetGpuBuffer();
	int GetDataSize();

public:
	VulkanUniformBuffer(
		VkDevice device,
		std::shared_ptr<Logger> logger,
		const String& name,
		std::shared_ptr<VulkanMemoryAllocator> memoryAllocator);

	virtual ~VulkanUniformBuffer();

	virtual bool Upload(void* buffer, int offset, int length);

};