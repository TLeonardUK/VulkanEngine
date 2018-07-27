#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Graphics/Vulkan/VulkanResource.h"

#include <vulkan/vulkan.h>

class VulkanPipeline 
	: public IGraphicsPipeline
	, public IVulkanResource
{
private:
	String m_name;
	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<VulkanGraphics> m_graphics;

	VkDevice m_device;
	VkPipeline m_pipeline;
	VkPipelineLayout m_pipelineLayout;

private:
	friend class VulkanGraphics;
	friend class VulkanCommandBuffer;

	bool Build(const GraphicsPipelineSettings& settings);

	VkPipeline GetPipeline();
	VkPipelineLayout GetPipelineLayout();

public:
	VulkanPipeline(
		std::shared_ptr<VulkanGraphics> graphics,
		VkDevice device,
		std::shared_ptr<Logger> logger,
		const String& name);

	virtual ~VulkanPipeline();

	virtual void FreeResources();
	virtual String GetName();

};