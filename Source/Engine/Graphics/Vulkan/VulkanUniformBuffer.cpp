#include "Pch.h"

#include "Engine/Graphics/Vulkan/VulkanUniformBuffer.h"
#include "Engine/Graphics/Vulkan/VulkanEnums.h"
#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Graphics/Vulkan/VulkanMemoryAllocator.h"

#include "Engine/Engine/Logging.h"
#include "Engine/Utilities/Statistic.h"

#include <vk_mem_alloc.h>

Statistic Stat_Rendering_Vulkan_UniformBufferCount("Rendering/Vulkan/Uniform Buffer Count", StatisticFrequency::Persistent, StatisticFormat::Integer);

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
	Stat_Rendering_Vulkan_UniformBufferCount.Add(1);
}

VulkanUniformBuffer::~VulkanUniformBuffer()
{
	Stat_Rendering_Vulkan_UniformBufferCount.Add(-1);
	FreeResources();
}

void VulkanUniformBuffer::FreeResources()
{
	if (m_buffer.Buffer != nullptr)
	{
		m_memoryAllocator->ReleaseUniformBuffer(m_buffer);
		m_buffer.Buffer = nullptr;
	}
}

String VulkanUniformBuffer::GetName()
{
	return m_name;
}

bool VulkanUniformBuffer::Build(int bufferSize)
{
	//m_logger->WriteInfo(LogCategory::Vulkan, "Builiding new uniform buffer: %s", m_name.c_str());
	m_memorySize = bufferSize;
	m_buffer.Buffer = nullptr;
	
	return true;
}

VkBuffer VulkanUniformBuffer::GetGpuBuffer()
{
	return m_buffer.Buffer;
}

uint32_t VulkanUniformBuffer::GetGpuBufferOffset()
{
	return m_buffer.Offset;
}

int VulkanUniformBuffer::GetDataSize()
{
	return m_memorySize;
}

bool VulkanUniformBuffer::Upload(void* buffer, int offset, int length)
{
	assert(offset >= 0 && offset + length <= m_memorySize);

	if (m_buffer.Buffer != nullptr)
	{
		m_memoryAllocator->ReleaseUniformBuffer(m_buffer);
	}
	m_buffer = m_memoryAllocator->AllocateUniformBuffer(m_memorySize);
	
	memcpy((char*)m_buffer.MappedData + m_buffer.Offset + offset, buffer, length);

	return true;
}