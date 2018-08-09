#include "Pch.h"

#include "Engine/Graphics/Vulkan/VulkanMemoryAllocator.h"
#include "Engine/Graphics/Vulkan/VulkanGraphics.h"

#include "Engine/Types/Math.h"
#include "Engine/Utilities/Statistic.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

VulkanMemoryAllocator::VulkanMemoryAllocator(
	VkDevice device,
	VkPhysicalDevice physicalDevice,
	std::shared_ptr<Logger> logger,
	std::shared_ptr<VulkanGraphics> graphics,
	const VulkanDeviceInfo& deviceInfo
)
	: m_device(device)
	, m_physicalDevice(physicalDevice)
	, m_logger(logger)
	, m_graphics(graphics)
	, m_deviceInfo(deviceInfo)
	, m_allocationsActive(0)
{
}

VulkanMemoryAllocator::~VulkanMemoryAllocator()
{
	FreeResources();
}

void VulkanMemoryAllocator::FreeResources()
{	
	for (auto& pool : m_uniformBufferPools)
	{
		FreeBuffer(pool->Buffer);
	}
	m_uniformBufferPools.clear();

	if (m_allocator != nullptr)
	{
		vmaDestroyAllocator(m_allocator);
		m_allocator = nullptr;
	}
}

bool VulkanMemoryAllocator::CreateBuffer(int size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryFlags, VmaAllocationCreateFlags allocFlags, VulkanAllocation& allocResult)
{
	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = size;
	bufferInfo.usage = bufferUsage;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = memoryFlags;
	allocInfo.flags = allocFlags;

	CheckVkResultReturnOnFail(vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo, &allocResult.Buffer, &allocResult.Allocation, &allocResult.AllocationInfo));

	allocResult.Allocator = m_allocator;

	m_allocationsActive++;

	return true;
}

bool VulkanMemoryAllocator::CreateImage(const VkImageCreateInfo& createInfo, VmaMemoryUsage memoryFlags, VmaAllocationCreateFlags allocFlags, VulkanAllocation& allocResult)
{
	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = memoryFlags;
	allocInfo.flags = allocFlags;

	CheckVkResultReturnOnFail(vmaCreateImage(m_allocator, &createInfo, &allocInfo, &allocResult.Image, &allocResult.Allocation, &allocResult.AllocationInfo));

	allocResult.Allocator = m_allocator;

	m_allocationsActive++;

	return true;
}

void VulkanMemoryAllocator::FreeBuffer(const VulkanAllocation& allocation)
{
	vmaDestroyBuffer(m_allocator, allocation.Buffer, allocation.Allocation);
	m_allocationsActive--;
	//allocation.Buffer = nullptr;
	//allocation.Allocation = nullptr;
}

void VulkanMemoryAllocator::FreeImage(const VulkanAllocation& allocation)
{
	vmaDestroyImage(m_allocator, allocation.Image, allocation.Allocation);
	m_allocationsActive--;
	//allocation.Buffer = nullptr;
	//allocation.Allocation = nullptr;
}

bool VulkanMemoryAllocator::Build()
{
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = m_physicalDevice;
	allocatorInfo.device = m_device;
	//allocatorInfo.preferredLargeHeapBlockSize = 1024 * 16;
	
	CheckVkResultReturnOnFail(vmaCreateAllocator(&allocatorInfo, &m_allocator));

	return true;
}

std::shared_ptr<VulkanUniformBufferPool> VulkanMemoryAllocator::GetUniformBufferPoolForSize(int size)
{
	for (auto& pool : m_uniformBufferPools)
	{
		if (pool->FreeChunkOffsets.size() > 0 && pool->ChunkSize >= (uint32_t)size)
		{
			return pool;
		}
	}

	// Time to create a new pool.
	std::shared_ptr<VulkanUniformBufferPool> pool = std::make_shared<VulkanUniformBufferPool>();
	pool->ChunkSize = std::max(Math::RoundUpToPowerOfTwo(size), (uint32_t)m_deviceInfo.Properties.limits.minUniformBufferOffsetAlignment);
	pool->BufferSize = std::min(MaxPoolBufferSize, (uint32_t)m_deviceInfo.Properties.limits.maxUniformBufferRange);
		
	if (!CreateBuffer(
		pool->BufferSize,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VMA_MEMORY_USAGE_CPU_TO_GPU,
		VMA_ALLOCATION_CREATE_MAPPED_BIT,
		pool->Buffer))
	{
		m_logger->WriteError(LogCategory::Vulkan, "Failed to create uniform buffer pool buffer (size %i).", pool->BufferSize);
		return nullptr;
	}

	for (uint32_t offset = 0; offset < pool->BufferSize; offset += pool->ChunkSize)
	{
		pool->FreeChunkOffsets.push_back(offset);
	}

	pool->MappedData = pool->Buffer.AllocationInfo.pMappedData;

	memset((char*)pool->MappedData, 0xAA, pool->BufferSize);

	m_logger->WriteWarning(LogCategory::Vulkan, "Allocated new uniform buffer pool (Chunk Size=%i, Chunks=%i).", pool->ChunkSize, pool->FreeChunkOffsets.size());

	// Insert buffer into pool by size.
	for (int i = 0; i < m_uniformBufferPools.size(); i++)
	{
		std::shared_ptr<VulkanUniformBufferPool> otherPool = m_uniformBufferPools[i];
		if (pool->ChunkSize < otherPool->ChunkSize)
		{
			m_uniformBufferPools.insert(m_uniformBufferPools.begin() + i, pool);
			return pool;
		}
	}

	m_uniformBufferPools.push_back(pool);
	return pool;
}

VulkanUniformBufferAllocation VulkanMemoryAllocator::AllocateUniformBuffer(int size)
{
	std::lock_guard<std::mutex> mutex(m_allocationMutex);

	std::shared_ptr<VulkanUniformBufferPool> pool = GetUniformBufferPoolForSize(size);

	VulkanUniformBufferAllocation allocation;
	allocation.Buffer = pool->Buffer.Buffer;
	allocation.MappedData = pool->MappedData;
	allocation.Offset = pool->FreeChunkOffsets.back();
	allocation.Pool = pool;

	pool->FreeChunkOffsets.pop_back();

	return allocation;
}

void VulkanMemoryAllocator::ReleaseUniformBufferInternal(VulkanUniformBufferAllocation allocation)
{
	std::lock_guard<std::mutex> mutex(m_allocationMutex);
	allocation.Pool->FreeChunkOffsets.push_back(allocation.Offset);
}

void VulkanMemoryAllocator::ReleaseUniformBuffer(VulkanUniformBufferAllocation allocation)
{
	m_graphics->QueueDisposal([this, allocation]() mutable {
		ReleaseUniformBufferInternal(allocation);
	});
}

extern Statistic Stat_Rendering_Memory_PoolBlockCount;
extern Statistic Stat_Rendering_Memory_PoolAllocationCount;
extern Statistic Stat_Rendering_Memory_PoolUsedBytes;
extern Statistic Stat_Rendering_Memory_PoolUnusedBytes;

void VulkanMemoryAllocator::UpdateStatistics()
{
	VmaStats stats;
	vmaCalculateStats(m_allocator, &stats);

	Stat_Rendering_Memory_PoolBlockCount.Set(stats.total.blockCount);
	Stat_Rendering_Memory_PoolAllocationCount.Set(stats.total.allocationCount);
	Stat_Rendering_Memory_PoolUsedBytes.Set((double)stats.total.usedBytes);
	Stat_Rendering_Memory_PoolUnusedBytes.Set((double)stats.total.unusedBytes);
}