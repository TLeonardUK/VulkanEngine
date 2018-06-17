#pragma once

#include "Engine/Graphics/Vulkan/VulkanImage.h"
#include "Engine/Graphics/Vulkan/VulkanEnums.h"
#include "Engine/Graphics/Vulkan/VulkanRenderPass.h"

#include "Engine/Engine/Logging.h"

#include <cassert>
#include <algorithm>
#include <math.h>

VulkanImage::VulkanImage(
	VkDevice device,
	std::shared_ptr<Logger> logger,
	const String& name,
	VkImage image,
	VkFormat format,
	VkExtent3D extents,
	bool isOwner,
	std::shared_ptr<VulkanGraphics> graphics
)
	: m_device(device)
	, m_logger(logger)
	, m_name(name)
	, m_isOwner(isOwner)
	, m_image(image)
	, m_format(format)
	, m_extents(extents)
	, m_isDepth(false)
	, m_mipLevels(1)
	, m_layers(1)
	, m_graphics(graphics)
{
}

VulkanImage::VulkanImage(
	VkDevice device,
	std::shared_ptr<Logger> logger,
	const String& name,
	std::shared_ptr<VulkanMemoryAllocator> allocator,
	std::shared_ptr<VulkanGraphics> graphics
)
	: m_device(device)
	, m_logger(logger)
	, m_name(name)
	, m_isOwner(true)
	, m_image(nullptr)
	, m_memoryAllocator(allocator)
	, m_isDepth(false)
	, m_mipLevels(1)
	, m_layers(1)
	, m_graphics(graphics)
{
}

VulkanImage::~VulkanImage()
{
	FreeResources();
}

void VulkanImage::FreeResources()
{
	for (auto& buffer : m_stagingBuffers)
	{
		m_graphics->ReleaseStagingBuffer(buffer);
	}
	m_stagingBuffers.clear();

	if (m_image != nullptr)
	{
		if (m_isOwner)
		{
			vkDestroyImage(m_device, m_image, nullptr);
		}
		m_image = nullptr;
	}
}

String VulkanImage::GetName()
{
	return m_name;
}

bool VulkanImage::Build(int width, int height, int layers, GraphicsFormat format, bool generateMips)
{
	m_logger->WriteInfo(LogCategory::Vulkan, "Builiding new image (%i x %i, %i layers): %s", width, height, layers, m_name.c_str());

	m_mipLevels = 1;
	if (generateMips)
	{
		m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
	}

	m_extents.width = width;
	m_extents.height = height;
	m_extents.depth = 1;
	m_layers = layers;
	m_format = GraphicsFormatToVkFormat(format);
	m_layerSize = width * height * GraphicsFormatBytesPerPixel(format);
	m_memorySize = m_layerSize * layers;
	m_isDepth = (format == GraphicsFormat::UNORM_D24_UINT_S8);

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent = m_extents;
	imageInfo.mipLevels = m_mipLevels;
	imageInfo.arrayLayers = layers;
	imageInfo.format = m_format;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	if (m_isDepth)
	{
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}
	else
	{
		imageInfo.usage = (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	}
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0;

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

Array<VulkanStagingBuffer> VulkanImage::ConsumeStagingBuffers()
{
	Array<VulkanStagingBuffer> buffers = m_stagingBuffers;
	m_stagingBuffers.clear();
	return buffers;
}

VkImage VulkanImage::GetVkImage()
{
	return m_image;
}

VkFormat VulkanImage::GetVkFormat()
{
	return m_format;
}

VkExtent3D VulkanImage::GetVkExtents()
{
	return m_extents;
}

bool VulkanImage::IsDepth()
{
	return m_isDepth;
}

int VulkanImage::GetWidth()
{
	return m_extents.width;
}

int VulkanImage::GetHeight()
{
	return m_extents.height;
}

int VulkanImage::GetMipLevels()
{
	return m_mipLevels;
}

int VulkanImage::GetLayers()
{
	return m_layers;
}

GraphicsFormat VulkanImage::GetFormat()
{
	return VkFormatToGraphicsFormat(m_format);
}

int VulkanImage::GetLayerSize()
{
	return m_layerSize;
}

bool VulkanImage::Stage(int layer, void* buffer, int offset, int length)
{
	assert(!m_isDepth);
	assert(offset >= 0 && offset + length <= m_memorySize);
	
	VulkanStagingBuffer stagingBuffer;
	if (m_stagingBuffers.size() == 0)
	{
		if (!m_graphics->AllocateStagingBuffer(m_memorySize, stagingBuffer))
		{
			return false;
		}

		m_stagingBuffers.push_back(stagingBuffer);
	}
	else
	{
		stagingBuffer = m_stagingBuffers[0];
	}

	memcpy((char*)stagingBuffer.MappedData + offset + (layer * m_layerSize), (char*)buffer, length);

	return true;
/*
	assert(!m_isDepth);
	assert(offset >= 0 && offset + length <= m_memorySize);

	void* deviceData;

	vmaMapMemory(m_stagingBuffer.Allocator, m_stagingBuffer.Allocation, &deviceData);

	memcpy((char*)deviceData + offset + (layer * m_layerSize), (char*)buffer, length);

	vmaUnmapMemory(m_stagingBuffer.Allocator, m_stagingBuffer.Allocation);

	return true;
*/
}