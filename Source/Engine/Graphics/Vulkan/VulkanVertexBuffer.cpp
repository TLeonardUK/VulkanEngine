#pragma once

#include "Engine/Graphics/Vulkan/VulkanVertexBuffer.h"
#include "Engine/Graphics/Vulkan/VulkanEnums.h"
#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Graphics/Vulkan/VulkanMemoryAllocator.h"

#include "Engine/Engine/Logging.h"

#include <cassert>
#include <vk_mem_alloc.h>

VulkanVertexBuffer::VulkanVertexBuffer(
	VkDevice device,
	std::shared_ptr<Logger> logger,
	const String& name,
	std::shared_ptr<VulkanMemoryAllocator> memoryAllocator
)
	: m_device(device)
	, m_logger(logger)
	, m_name(name)
	, m_memoryAllocator(memoryAllocator)
{
}

VulkanVertexBuffer::~VulkanVertexBuffer()
{
	FreeResources();
}

void VulkanVertexBuffer::FreeResources()
{
	if (m_stagingBuffer.Allocation != nullptr)
	{
		m_memoryAllocator->FreeBuffer(m_stagingBuffer);
		m_stagingBuffer.Allocation = nullptr;
	}
	if (m_gpuBuffer.Allocation != nullptr)
	{
		m_memoryAllocator->FreeBuffer(m_gpuBuffer);
		m_gpuBuffer.Allocation = nullptr;
	}
}

bool VulkanVertexBuffer::Build(const VertexBufferBindingDescription& binding, int vertexCount)
{
	m_logger->WriteInfo(LogCategory::Vulkan, "Builiding new vertex buffer: %s", m_name.c_str());

	Array<VkVertexInputBindingDescription> bindingDescriptions;
	Array<VkVertexInputAttributeDescription> attributeDescriptions;

	if (!GraphicsBindingDescriptionToVulkan(binding, bindingDescriptions, attributeDescriptions))
	{
		return false;
	}

	m_capacity = vertexCount;
	m_memorySize = binding.vertexSize * vertexCount;

	if (!m_memoryAllocator->CreateBuffer(
		m_memorySize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VMA_MEMORY_USAGE_CPU_TO_GPU,
		m_stagingBuffer))
	{
		return false;
	}

	if (!m_memoryAllocator->CreateBuffer(
		m_memorySize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT |VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY,
		m_gpuBuffer))
	{
		return false;
	}

	return true;
}

bool VulkanVertexBuffer::Stage(void* buffer, int offset, int length)
{
	assert(offset >= 0 && offset + length <= m_memorySize);

	void* deviceData;
	
	vmaMapMemory(m_stagingBuffer.Allocator, m_stagingBuffer.Allocation, &deviceData);

	memcpy((char*)deviceData + offset, buffer, length);
	
	vmaUnmapMemory(m_stagingBuffer.Allocator, m_stagingBuffer.Allocation);

	return true;
}

VulkanAllocation VulkanVertexBuffer::GetGpuBuffer()
{
	return m_gpuBuffer;
}

VulkanAllocation VulkanVertexBuffer::GetStagingBuffer()
{
	return m_stagingBuffer;
}

int VulkanVertexBuffer::GetDataSize()
{
	return m_memorySize;
}

bool GraphicsBindingDescriptionToVulkan(const VertexBufferBindingDescription& description, Array<VkVertexInputBindingDescription>& bindings, Array<VkVertexInputAttributeDescription>& attributes)
{
	bindings.resize(1);
	attributes.resize(description.attributes.size());

	VkVertexInputBindingDescription& bindingDescription = bindings[0];
	bindingDescription.binding = 0;
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	bindingDescription.stride = description.vertexSize;

	for (int i = 0; i < description.attributes.size(); i++)
	{
		const VertexBufferBindingAttribute& attribute = description.attributes[i];

		VkVertexInputAttributeDescription& description = attributes[i];
		description.binding = 0;
		description.location = attribute.location;
		description.format = GraphicsBindingFormatToVkFormat(attribute.format);
		description.offset = attribute.offset;
	}

	return true;
}

int VulkanVertexBuffer::GetCapacity()
{
	return m_capacity;
}