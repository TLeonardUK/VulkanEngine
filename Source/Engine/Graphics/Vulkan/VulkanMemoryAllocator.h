#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Engine/Logging.h"

#include "Engine/Graphics/Vulkan/VulkanDeviceInfo.h"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

struct VulkanAllocation
{
	VkBuffer Buffer;
	VkImage Image;
	VmaAllocator Allocator;
	VmaAllocation Allocation;
	VmaAllocationInfo AllocationInfo;

public:
	VulkanAllocation()
		: Buffer(nullptr)
		, Image(nullptr)
		, Allocator(nullptr)
		, Allocation(nullptr)
	{
	}
};

class VulkanMemoryAllocator
{
private:
	std::shared_ptr<Logger> m_logger;
	
	VkDevice m_device;
	VkPhysicalDevice m_physicalDevice;

	VmaAllocator m_allocator;

private:
	friend class VulkanGraphics;

	void FreeResources();

public:
	VulkanMemoryAllocator(
		VkDevice device,
		VkPhysicalDevice physicalDevice,
		std::shared_ptr<Logger> logger);

	virtual ~VulkanMemoryAllocator();

	bool Build();

	bool CreateBuffer(int size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryFlags, VulkanAllocation& allocation);
	bool CreateImage(const VkImageCreateInfo& createInfo, VmaMemoryUsage memoryFlags, VulkanAllocation& allocation);
	void FreeBuffer(VulkanAllocation& allocation);

};