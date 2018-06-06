#pragma once

#include "Engine/Graphics/Vulkan/VulkanUniformBuffer.h"
#include "Engine/Graphics/Vulkan/VulkanEnums.h"
#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Graphics/Vulkan/VulkanMemoryAllocator.h"

#include "Engine/Engine/Logging.h"

#include <cassert>
#include <vk_mem_alloc.h>

VulkanUniformBuffer::VulkanUniformBuffer(
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

VulkanUniformBuffer::~VulkanUniformBuffer()
{
	FreeResources();
}

void VulkanUniformBuffer::FreeResources()
{
	if (m_gpuBuffer.Allocation != nullptr)
	{
		m_memoryAllocator->FreeBuffer(m_gpuBuffer);
		m_gpuBuffer.Allocation = nullptr;
	}
}

bool VulkanUniformBuffer::Build(int bufferSize)
{
	m_logger->WriteInfo(LogCategory::Vulkan, "Builiding new uniform buffer: %s", m_name.c_str());

	m_memorySize = bufferSize;

	if (!m_memoryAllocator->CreateBuffer(
		m_memorySize,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VMA_MEMORY_USAGE_CPU_TO_GPU,
		m_gpuBuffer))
	{
		return false;
	}

	return true;
}

VulkanAllocation VulkanUniformBuffer::GetGpuBuffer()
{
	return m_gpuBuffer;
}

int VulkanUniformBuffer::GetDataSize()
{
	return m_memorySize;
}

bool VulkanUniformBuffer::Upload(void* buffer, int offset, int length)
{
	assert(offset >= 0 && offset + length <= m_memorySize);

	void* deviceData;

	vmaMapMemory(m_gpuBuffer.Allocator, m_gpuBuffer.Allocation, &deviceData);

	memcpy((char*)deviceData + offset, buffer, length);

	vmaUnmapMemory(m_gpuBuffer.Allocator, m_gpuBuffer.Allocation);

	return true;
}