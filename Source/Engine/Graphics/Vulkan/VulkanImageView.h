#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Engine/Logging.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/GraphicsImage.h"
#include "Engine/Graphics/GraphicsImageView.h"
#include "Engine/Graphics/Vulkan/VulkanResource.h"

#include <vulkan/vulkan.h>

class VulkanImageView
	: public IGraphicsImageView
	, public IVulkanResource
{
private:
	String m_name;
	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<VulkanGraphics> m_graphics;

	VkDevice m_device;
	VkImageView m_imageView;

	std::shared_ptr<VulkanImage> m_vulkanImage;

private:
	friend class VulkanGraphics;
	friend class VulkanFramebuffer;
	friend class VulkanResourceSet;
	friend class VulkanResourceSetPool;
	friend struct VulkanResourceSetBinding;
	friend class VulkanCommandBuffer;

	bool Build(std::shared_ptr<IGraphicsImage> image, int baseLayer, int layerCount);

	VkImageView GetImageView()
	{
		return m_imageView;
	}

	std::shared_ptr<VulkanImage> GetVkImage()
	{
		return m_vulkanImage;
	}

public:
	VulkanImageView(
		std::shared_ptr<VulkanGraphics> graphics,
		VkDevice device,
		std::shared_ptr<Logger> logger,
		const String& name);

	virtual ~VulkanImageView();

	virtual int GetWidth();
	virtual int GetHeight();
	virtual std::shared_ptr<IGraphicsImage> GetImage();

	virtual void FreeResources();
	virtual String GetName();

};