#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Engine/Logging.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/GraphicsVertexBuffer.h"
#include "Engine/Graphics/Vulkan/VulkanMemoryAllocator.h"

#include <vulkan/vulkan.h>

class VulkanMemoryAllocator;

bool GraphicsBindingDescriptionToVulkan(const VertexBufferBindingDescription& description, Array<VkVertexInputBindingDescription>& bindings, Array<VkVertexInputAttributeDescription>& attributes);

class VulkanVertexBuffer : public IGraphicsVertexBuffer
{
private:
	String m_name;
	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<VulkanMemoryAllocator> m_memoryAllocator;

	int m_memorySize;
	int m_capacity;

	VkDevice m_device;
	VulkanAllocation m_stagingBuffer;
	VulkanAllocation m_gpuBuffer;

private:
	friend class VulkanGraphics;
	friend class VulkanCommandBuffer;

	bool Build(const VertexBufferBindingDescription& binding, int vertexCount);
	void FreeResources();

	VulkanAllocation GetGpuBuffer();
	VulkanAllocation GetStagingBuffer();
	int GetDataSize();

public:
	VulkanVertexBuffer(
		VkDevice device,
		std::shared_ptr<Logger> logger,
		const String& name,
		std::shared_ptr<VulkanMemoryAllocator> memoryAllocator);

	virtual ~VulkanVertexBuffer();

	virtual bool Stage(void* buffer, int offset, int length);

	virtual int GetCapacity();

};