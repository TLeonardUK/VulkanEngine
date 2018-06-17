#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Graphics/Vulkan/VulkanResource.h"

#include <vulkan/vulkan.h>

class VulkanRenderPass 
	: public IGraphicsRenderPass
	, public IVulkanResource
{
private:
	String m_name;
	std::shared_ptr<Logger> m_logger;

	VkDevice m_device;
	VkRenderPass m_renderPass;

	GraphicsRenderPassSettings m_settings;

private:
	friend class VulkanGraphics;
	friend class VulkanPipeline;
	friend class VulkanFramebuffer;
	friend class VulkanCommandBuffer;

	bool Build(const GraphicsRenderPassSettings& settings);

	VkRenderPass GetRenderPass();
	GraphicsRenderPassSettings GetSettings();

public:
	VulkanRenderPass(
		VkDevice device,
		std::shared_ptr<Logger> logger,
		const String& name);

	virtual ~VulkanRenderPass();

	virtual void FreeResources();
	virtual String GetName();

};