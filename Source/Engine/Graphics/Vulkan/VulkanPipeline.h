#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/Vulkan/VulkanGraphics.h"

#include <vulkan/vulkan.h>

class VulkanPipeline : public IGraphicsPipeline
{
private:
	String m_name;
	std::shared_ptr<Logger> m_logger;

	VkDevice m_device;
	VkPipeline m_pipeline;
	VkPipelineLayout m_pipelineLayout;

private:
	friend class VulkanGraphics;
	friend class VulkanCommandBuffer;

	bool Build(const GraphicsPipelineSettings& settings);
	void FreeResources();

	VkPipeline GetPipeline();
	VkPipelineLayout GetPipelineLayout();

public:
	VulkanPipeline(
		VkDevice device,
		std::shared_ptr<Logger> logger,
		const String& name);

	virtual ~VulkanPipeline();

};