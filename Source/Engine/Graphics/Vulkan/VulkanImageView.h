#pragma once

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

	VkDevice m_device;
	VkImageView m_imageView;

	std::shared_ptr<VulkanImage> m_vulkanImage;

private:
	friend class VulkanGraphics;
	friend class VulkanFramebuffer;
	friend class VulkanResourceSet;
	friend class VulkanResourceSetPool;
	friend struct VulkanResourceSetBinding;

	bool Build(std::shared_ptr<IGraphicsImage> image);

	VkImageView GetImageView();

public:
	VulkanImageView(
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