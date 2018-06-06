#include "Engine/Graphics/Vulkan/VulkanMemoryAllocator.h"
#include "Engine/Graphics/Vulkan/VulkanGraphics.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

VulkanMemoryAllocator::VulkanMemoryAllocator(
	VkDevice device,
	VkPhysicalDevice physicalDevice,
	std::shared_ptr<Logger> logger
)
	: m_device(device)
	, m_physicalDevice(physicalDevice)
	, m_logger(logger)
{
}

VulkanMemoryAllocator::~VulkanMemoryAllocator()
{
	FreeResources();
}

void VulkanMemoryAllocator::FreeResources()
{
	if (m_allocator != nullptr)
	{
		vmaDestroyAllocator(m_allocator);
		m_allocator = nullptr;
	}
}

bool VulkanMemoryAllocator::CreateBuffer(int size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryFlags, VulkanAllocation& allocResult)
{
	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = size;
	bufferInfo.usage = bufferUsage;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = memoryFlags;
	//allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

	CheckVkResultReturnOnFail(vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo, &allocResult.Buffer, &allocResult.Allocation, &allocResult.AllocationInfo));

	allocResult.Allocator = m_allocator;

	return true;
}

bool VulkanMemoryAllocator::CreateImage(const VkImageCreateInfo& createInfo, VmaMemoryUsage memoryFlags, VulkanAllocation& allocResult)
{
	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = memoryFlags;
	//allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

	CheckVkResultReturnOnFail(vmaCreateImage(m_allocator, &createInfo, &allocInfo, &allocResult.Image, &allocResult.Allocation, &allocResult.AllocationInfo));

	allocResult.Allocator = m_allocator;

	return true;
}

void VulkanMemoryAllocator::FreeBuffer(VulkanAllocation& allocation)
{
	m_logger->WriteInfo(LogCategory::Vulkan, "Freed block of device memory.");

	vmaDestroyBuffer(m_allocator, allocation.Buffer, allocation.Allocation);
	allocation.Buffer = nullptr;
	allocation.Allocation = nullptr;
}

bool VulkanMemoryAllocator::Build()
{
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = m_physicalDevice;
	allocatorInfo.device = m_device;
	
	CheckVkResultReturnOnFail(vmaCreateAllocator(&allocatorInfo, &m_allocator));

	return true;
}