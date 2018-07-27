#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Engine/Logging.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/GraphicsIndexBuffer.h"
#include "Engine/Graphics/Vulkan/VulkanMemoryAllocator.h"
#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Graphics/Vulkan/VulkanResource.h"

#include <vulkan/vulkan.h>

class VulkanMemoryAllocator;
class VulkanGraphics;

class VulkanIndexBuffer
	: public IGraphicsIndexBuffer
	, public IVulkanResource
{
private:
	String m_name;
	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<VulkanMemoryAllocator> m_memoryAllocator;

	int m_memorySize;
	int m_indexSize;
	int m_capacity;

	VkDevice m_device;
	VulkanAllocation m_gpuBuffer;

	Array<VulkanStagingBuffer> m_stagingBuffers;

	std::shared_ptr<VulkanGraphics> m_graphics;

private:
	friend class VulkanGraphics;
	friend class VulkanCommandBuffer;

	bool Build(int indexSize, int indexCount);

	VulkanAllocation GetGpuBuffer();
	Array<VulkanStagingBuffer> ConsumeStagingBuffers();
	int GetDataSize();
	int GetIndexSize();

public:
	VulkanIndexBuffer(
		VkDevice device,
		std::shared_ptr<Logger> logger,
		const String& name,
		std::shared_ptr<VulkanMemoryAllocator> memoryAllocator,
		std::shared_ptr<VulkanGraphics> graphics);

	virtual ~VulkanIndexBuffer();

	virtual bool Stage(void* buffer, int offset, int length);

	virtual int GetCapacity();

	virtual void FreeResources();
	virtual String GetName();

};