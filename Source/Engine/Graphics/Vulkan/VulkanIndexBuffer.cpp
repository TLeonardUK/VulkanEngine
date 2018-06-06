#pragma once

#include "Engine/Graphics/Vulkan/VulkanIndexBuffer.h"
#include "Engine/Graphics/Vulkan/VulkanEnums.h"
#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Graphics/Vulkan/VulkanMemoryAllocator.h"

#include "Engine/Engine/Logging.h"

#include <cassert>
#include <vk_mem_alloc.h>

VulkanIndexBuffer::VulkanIndexBuffer(
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

VulkanIndexBuffer::~VulkanIndexBuffer()
{
	FreeResources();
}

void VulkanIndexBuffer::FreeResources()
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

bool VulkanIndexBuffer::Build(int indexSize, int vertexCount)
{
	m_logger->WriteInfo(LogCategory::Vulkan, "Builiding new index buffer: %s", m_name.c_str());

	assert(indexSize == 2 || indexSize == 4);

	m_memorySize = indexSize * vertexCount;
	m_indexSize = indexSize;

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
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY,
		m_gpuBuffer))
	{
		return false;
	}

	return true;
}

bool VulkanIndexBuffer::Stage(void* buffer, int offset, int length)
{
	assert(offset >= 0 && offset + length <= m_memorySize);

	void* deviceData;

	vmaMapMemory(m_stagingBuffer.Allocator, m_stagingBuffer.Allocation, &deviceData);

	memcpy((char*)deviceData + offset, buffer, length);

	vmaUnmapMemory(m_stagingBuffer.Allocator, m_stagingBuffer.Allocation);

	return true;
}

VulkanAllocation VulkanIndexBuffer::GetGpuBuffer()
{
	return m_gpuBuffer;
}

VulkanAllocation VulkanIndexBuffer::GetStagingBuffer()
{
	return m_stagingBuffer;
}

int VulkanIndexBuffer::GetDataSize()
{
	return m_memorySize;
}

int VulkanIndexBuffer::GetIndexSize()
{
	return m_indexSize;
}
