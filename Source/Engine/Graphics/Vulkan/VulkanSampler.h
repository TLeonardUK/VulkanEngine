#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/GraphicsPipeline.h"
#include "Engine/Graphics/GraphicsSampler.h"
#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Graphics/Vulkan/VulkanResource.h"

#include <vulkan/vulkan.h>

class VulkanSampler 
	: public IGraphicsSampler
	, public IVulkanResource
{
private:
	String m_name;
	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<VulkanGraphics> m_graphics;

	VkDevice m_device;
	VkSampler m_sampler;

private:
	friend class VulkanGraphics;
	friend class VulkanPipeline;
	friend class VulkanResourceSet;
	friend class VulkanResourceSetPool;
	friend struct VulkanResourceSetBinding;

	bool Build(const SamplerDescription& description);

	VkSampler GetSampler();

public:
	VulkanSampler(
		std::shared_ptr<VulkanGraphics> graphics,
		VkDevice device,
		std::shared_ptr<Logger> logger,
		const String& name);

	virtual ~VulkanSampler();

	virtual void FreeResources();
	virtual String GetName();

};