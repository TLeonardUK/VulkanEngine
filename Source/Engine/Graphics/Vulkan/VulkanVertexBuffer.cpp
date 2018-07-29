#include "Pch.h"

#include "Engine/Graphics/Vulkan/VulkanVertexBuffer.h"
#include "Engine/Graphics/Vulkan/VulkanEnums.h"
#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Graphics/Vulkan/VulkanMemoryAllocator.h"

#include "Engine/Engine/Logging.h"
#include "Engine/Utilities/Statistic.h"

#include <vk_mem_alloc.h>

Statistic Stat_Rendering_Vulkan_VertexBufferCount("Rendering/Vulkan/Vertex Buffer Count", StatisticFrequency::Persistent, StatisticFormat::Integer);

VulkanVertexBuffer::VulkanVertexBuffer(
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
	Stat_Rendering_Vulkan_VertexBufferCount.Add(1);
}

VulkanVertexBuffer::~VulkanVertexBuffer()
{
	Stat_Rendering_Vulkan_VertexBufferCount.Add(-1);
	FreeResources();
}

void VulkanVertexBuffer::FreeResources()
{
	for (auto& buffer : m_stagingBuffers)
	{
		m_graphics->ReleaseStagingBuffer(buffer);
	}
	m_stagingBuffers.clear();

	if (m_gpuBuffer.Allocation != nullptr)
	{
		m_graphics->QueueDisposal([m_memoryAllocator = m_memoryAllocator, m_gpuBuffer = m_gpuBuffer]() {
			m_memoryAllocator->FreeBuffer(m_gpuBuffer);
		});
		m_gpuBuffer.Allocation = nullptr;
	}
}

String VulkanVertexBuffer::GetName()
{
	return m_name;
}

bool VulkanVertexBuffer::Build(const VertexBufferBindingDescription& binding, int vertexCount)
{
	//m_logger->WriteInfo(LogCategory::Vulkan, "Builiding new vertex buffer: %s", m_name.c_str());

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
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VMA_MEMORY_USAGE_GPU_ONLY,
		0,
		m_gpuBuffer))
	{
		return false;
	}

	return true;
}

bool VulkanVertexBuffer::Stage(void* buffer, int offset, int length)
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

VulkanAllocation VulkanVertexBuffer::GetGpuBuffer()
{
	return m_gpuBuffer;
}

Array<VulkanStagingBuffer> VulkanVertexBuffer::ConsumeStagingBuffers()
{
	Array<VulkanStagingBuffer> buffers = m_stagingBuffers;
	m_stagingBuffers.clear();
	return buffers;
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