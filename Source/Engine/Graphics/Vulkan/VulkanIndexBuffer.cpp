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
	std::shared_ptr<VulkanMemoryAllocator> memoryAllocator,
	std::shared_ptr<VulkanGraphics> graphics
)
	: m_device(device)
	, m_logger(logger)
	, m_name(name)
	, m_memoryAllocator(memoryAllocator)
	, m_graphics(graphics)
{
}

VulkanIndexBuffer::~VulkanIndexBuffer()
{
	FreeResources();
}

void VulkanIndexBuffer::FreeResources()
{
	for (auto& buffer : m_stagingBuffers)
	{
		m_graphics->ReleaseStagingBuffer(buffer);
	}
	m_stagingBuffers.clear();

	if (m_gpuBuffer.Allocation != nullptr)
	{
		m_memoryAllocator->FreeBuffer(m_gpuBuffer);
		m_gpuBuffer.Allocation = nullptr;
	}
}

String VulkanIndexBuffer::GetName()
{
	return m_name;
}

bool VulkanIndexBuffer::Build(int indexSize, int vertexCount)
{
	m_logger->WriteInfo(LogCategory::Vulkan, "Builiding new index buffer: %s", m_name.c_str());

	assert(indexSize == 2 || indexSize == 4);

	m_capacity = vertexCount;
	m_memorySize = indexSize * vertexCount;
	m_indexSize = indexSize;

	if (!m_memoryAllocator->CreateBuffer(
		m_memorySize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY,
		0,
		m_gpuBuffer))
	{
		return false;
	}

	return true;
}

bool VulkanIndexBuffer::Stage(void* buffer, int offset, int length)
{
	assert(offset >= 0 && offset + length <= m_memorySize);

	VulkanStagingBuffer stagingBuffer;
	if (!m_graphics->AllocateStagingBuffer(m_gpuBuffer, offset, length, stagingBuffer))
	{
		return false;
	}

	memcpy((char*)stagingBuffer.MappedData, buffer, length);

	m_stagingBuffers.push_back(stagingBuffer);

	return true;
}

VulkanAllocation VulkanIndexBuffer::GetGpuBuffer()
{
	return m_gpuBuffer;
}

Array<VulkanStagingBuffer> VulkanIndexBuffer::ConsumeStagingBuffers()
{
	Array<VulkanStagingBuffer> buffers = m_stagingBuffers;
	m_stagingBuffers.clear();
	return buffers;
}

int VulkanIndexBuffer::GetDataSize()
{
	return m_memorySize;
}

int VulkanIndexBuffer::GetIndexSize()
{
	return m_indexSize;
}

int VulkanIndexBuffer::GetCapacity()
{
	return m_capacity;
}