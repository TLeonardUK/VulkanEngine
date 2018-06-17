#pragma once

#include "Engine/Graphics/Vulkan/VulkanImage.h"
#include "Engine/Graphics/Vulkan/VulkanImageView.h"
#include "Engine/Graphics/Vulkan/VulkanEnums.h"
#include "Engine/Graphics/Vulkan/VulkanRenderPass.h"

#include "Engine/Engine/Logging.h"

#include <cassert>

VulkanImageView::VulkanImageView(
	VkDevice device,
	std::shared_ptr<Logger> logger,
	const String& name
)
	: m_device(device)
	, m_logger(logger)
	, m_name(name)
{
}

VulkanImageView::~VulkanImageView()
{
	FreeResources();
}

void VulkanImageView::FreeResources()
{
	if (m_imageView != nullptr)
	{
		vkDestroyImageView(m_device, m_imageView, nullptr);
		m_imageView = nullptr;
	}
}

String VulkanImageView::GetName()
{
	return m_name;
}

VkImageView VulkanImageView::GetImageView()
{
	return m_imageView;
}

bool VulkanImageView::Build(std::shared_ptr<IGraphicsImage> image)
{
	m_logger->WriteInfo(LogCategory::Vulkan, "Builiding new image view: %s", m_name.c_str());

	std::shared_ptr<VulkanImage> vulkanImage = std::dynamic_pointer_cast<VulkanImage>(image);

	m_vulkanImage = vulkanImage;

	bool isDepth = (vulkanImage->GetVkFormat() == VK_FORMAT_D24_UNORM_S8_UINT);

	VkImageViewCreateInfo createViewInfo = {};
	createViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createViewInfo.image = vulkanImage->GetVkImage();

	if (image->GetLayers() == 6)
	{
		createViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	}
	else if (image->GetLayers() == 1)
	{
		createViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	}
	else
	{
		assert(false);
	}

	createViewInfo.format = vulkanImage->GetVkFormat();
	createViewInfo.subresourceRange.aspectMask = isDepth ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) : VK_IMAGE_ASPECT_COLOR_BIT;
	createViewInfo.subresourceRange.baseMipLevel = 0;
	createViewInfo.subresourceRange.baseArrayLayer = 0;
	createViewInfo.subresourceRange.layerCount = vulkanImage->GetLayers();
	createViewInfo.subresourceRange.levelCount = vulkanImage->GetMipLevels();
	createViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	CheckVkResultReturnOnFail(vkCreateImageView(m_device, &createViewInfo, nullptr, &m_imageView));

	return true;
}

int VulkanImageView::GetWidth()
{
	return m_vulkanImage->GetVkExtents().width;
}

int VulkanImageView::GetHeight()
{
	return m_vulkanImage->GetVkExtents().height;
}

std::shared_ptr<IGraphicsImage> VulkanImageView::GetImage()
{
	return m_vulkanImage;
}