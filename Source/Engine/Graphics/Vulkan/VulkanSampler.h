#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/GraphicsPipeline.h"
#include "Engine/Graphics/GraphicsSampler.h"
#include "Engine/Graphics/Vulkan/VulkanGraphics.h"

#include <vulkan/vulkan.h>

class VulkanSampler : public IGraphicsSampler
{
private:
	String m_name;
	std::shared_ptr<Logger> m_logger;

	VkDevice m_device;
	VkSampler m_sampler;

private:
	friend class VulkanGraphics;
	friend class VulkanPipeline;
	friend class VulkanResourceSet;

	bool Build(const SamplerDescription& description);
	void FreeResources();

	VkSampler GetSampler();

public:
	VulkanSampler(
		VkDevice device,
		std::shared_ptr<Logger> logger,
		const String& name);

	virtual ~VulkanSampler();

};