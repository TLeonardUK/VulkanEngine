#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Graphics/Vulkan/VulkanResource.h"

#include <vulkan/vulkan.h>

class VulkanFramebuffer 
	: public IGraphicsFramebuffer
	, public IVulkanResource
{
private:
	String m_name;
	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<VulkanGraphics> m_graphics;

	VkDevice m_device;
	VkFramebuffer m_framebuffer;

	Array<std::shared_ptr<VulkanImageView>> m_attachments;

	int m_width;
	int m_height;

private:
	friend class VulkanGraphics;
	friend class VulkanCommandBuffer;

	bool Build(const GraphicsFramebufferSettings& settings);

	int GetWidth();
	int GetHeight();

	const Array<std::shared_ptr<VulkanImageView>>& GetAttachments();
	VkFramebuffer GetFramebuffer();

public:
	VulkanFramebuffer(
		std::shared_ptr<VulkanGraphics> graphics,
		VkDevice device,
		std::shared_ptr<Logger> logger,
		const String& name);

	virtual ~VulkanFramebuffer();

	virtual void FreeResources();
	virtual String GetName();

};