#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/Vulkan/VulkanGraphics.h"

#include <vulkan/vulkan.h>

class VulkanRenderPass : public IGraphicsRenderPass
{
private:
	String m_name;
	std::shared_ptr<Logger> m_logger;

	VkDevice m_device;
	VkRenderPass m_renderPass;

private:
	friend class VulkanGraphics;
	friend class VulkanPipeline;
	friend class VulkanFramebuffer;
	friend class VulkanCommandBuffer;

	bool Build(const GraphicsRenderPassSettings& settings);
	void FreeResources();

	VkRenderPass GetRenderPass();

public:
	VulkanRenderPass(
		VkDevice device,
		std::shared_ptr<Logger> logger,
		const String& name);

	virtual ~VulkanRenderPass();

};