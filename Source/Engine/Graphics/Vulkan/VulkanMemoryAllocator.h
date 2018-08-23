#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Types/Mutex.h"
#include "Engine/Engine/Logging.h"

#include "Engine/Graphics/Vulkan/VulkanDeviceInfo.h"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

class VulkanGraphics;

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

struct VulkanUniformBufferPool
{
	VulkanAllocation Buffer;
	void* MappedData;
	uint32_t ChunkSize;
	uint32_t BufferSize;
	Array<uint32_t> FreeChunkOffsets;
};

struct VulkanUniformBufferAllocation
{
	VkBuffer Buffer;
	void* MappedData;
	uint32_t Offset;
	std::shared_ptr<VulkanUniformBufferPool> Pool;
};

class VulkanMemoryAllocator
{
private:
	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<VulkanGraphics> m_graphics;
	
	VkDevice m_device;
	VkPhysicalDevice m_physicalDevice;

	VulkanDeviceInfo m_deviceInfo;

	VmaAllocator m_allocator;

	Array<std::shared_ptr<VulkanUniformBufferPool>> m_uniformBufferPools;

	const uint32_t MaxPoolBufferSize = 8 * 1024 * 1024;

	std::atomic<int> m_allocationsActive;

	Mutex m_allocationMutex;

private:
	friend class VulkanGraphics;

	void FreeResources();

	void ReleaseUniformBufferInternal(VulkanUniformBufferAllocation allocation);
	std::shared_ptr<VulkanUniformBufferPool> GetUniformBufferPoolForSize(int size);

public:
	VulkanMemoryAllocator(
		VkDevice device,
		VkPhysicalDevice physicalDevice,
		std::shared_ptr<Logger> logger,
		std::shared_ptr<VulkanGraphics> graphics,
		const VulkanDeviceInfo& deviceInfo);

	virtual ~VulkanMemoryAllocator();

	bool Build();

	void UpdateStatistics();

	bool CreateBuffer(int size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryFlags, VmaAllocationCreateFlags allocFlags, VulkanAllocation& allocation);
	bool CreateImage(const VkImageCreateInfo& createInfo, VmaMemoryUsage memoryFlags, VmaAllocationCreateFlags allocFlags, VulkanAllocation& allocation);
	void FreeBuffer(const VulkanAllocation& allocation);
	void FreeImage(const VulkanAllocation& allocation);

	VulkanUniformBufferAllocation AllocateUniformBuffer(int size);
	void ReleaseUniformBuffer(VulkanUniformBufferAllocation allocation);

};