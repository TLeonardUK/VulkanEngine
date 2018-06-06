#pragma once

#include "Engine/Graphics/Vulkan/VulkanImage.h"
#include "Engine/Graphics/Vulkan/VulkanEnums.h"
#include "Engine/Graphics/Vulkan/VulkanRenderPass.h"

#include "Engine/Engine/Logging.h"

#include <cassert>

VulkanImage::VulkanImage(
	VkDevice device,
	std::shared_ptr<Logger> logger,
	const String& name,
	VkImage image,
	VkFormat format,
	VkExtent3D extents,
	bool isOwner
)
	: m_device(device)
	, m_logger(logger)
	, m_name(name)
	, m_isOwner(isOwner)
	, m_image(image)
	, m_format(format)
	, m_extents(extents)
	, m_isDepth(false)
{
}

VulkanImage::VulkanImage(
	VkDevice device,
	std::shared_ptr<Logger> logger,
	const String& name,
	std::shared_ptr<VulkanMemoryAllocator> allocator
)
	: m_device(device)
	, m_logger(logger)
	, m_name(name)
	, m_isOwner(true)
	, m_image(nullptr)
	, m_memoryAllocator(allocator)
	, m_isDepth(false)
{
}

VulkanImage::~VulkanImage()
{
	FreeResources();
}

void VulkanImage::FreeResources()
{
	if (m_stagingBuffer.Allocation != nullptr)
	{
		m_memoryAllocator->FreeBuffer(m_stagingBuffer);
		m_stagingBuffer.Allocation = nullptr;
	}
	else if (m_image != nullptr)
	{
		if (m_isOwner)
		{
			vkDestroyImage(m_device, m_image, nullptr);
		}
		m_image = nullptr;
	}
}

bool VulkanImage::Build(int width, int height, int depth, GraphicsFormat format)
{
	m_extents.width = width;
	m_extents.height = height;
	m_extents.depth = depth;
	m_format = GraphicsFormatToVkFormat(format);
	m_memorySize = width * height * depth * GraphicsFormatBytesPerPixel(format);
	m_isDepth = (format == GraphicsFormat::UNORM_D24_UINT_S8);

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = depth == 1 ? VK_IMAGE_TYPE_2D : VK_IMAGE_TYPE_3D;
	imageInfo.extent = m_extents;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = m_format;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = m_isDepth ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0;

	if (!m_isDepth)
	{
		if (!m_memoryAllocator->CreateBuffer(
			m_memorySize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_CPU_TO_GPU,
			m_stagingBuffer))
		{
			return false;
		}
	}

	if (!m_memoryAllocator->CreateImage(
		imageInfo,
		VMA_MEMORY_USAGE_GPU_ONLY,
		m_mainImage))
	{
		return false;
	}

	m_image = m_mainImage.Image;

	return true;
}

VkBuffer VulkanImage::GetStagingBuffer()
{
	return m_stagingBuffer.Buffer;
}

VkImage VulkanImage::GetImage()
{
	return m_image;
}

VkFormat VulkanImage::GetFormat()
{
	return m_format;
}

VkExtent3D VulkanImage::GetExtents()
{
	return m_extents;
}

bool VulkanImage::IsDepth()
{
	return m_isDepth;
}

bool VulkanImage::Stage(void* buffer, int offset, int length)
{
	assert(!m_isDepth);
	assert(offset >= 0 && offset + length <= m_memorySize);

	void* deviceData;

	vmaMapMemory(m_stagingBuffer.Allocator, m_stagingBuffer.Allocation, &deviceData);

	memcpy((char*)deviceData + offset, buffer, length);

	vmaUnmapMemory(m_stagingBuffer.Allocator, m_stagingBuffer.Allocation);

	return true;
}