#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/GraphicsPipeline.h"
#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Graphics/Vulkan/VulkanResource.h"

#include <vulkan/vulkan.h>

class VulkanShader 
	: public IGraphicsShader
	, public IVulkanResource
{
private:
	String m_name;
	String m_entryPoint;
	GraphicsPipelineStage m_stage;
	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<VulkanGraphics> m_graphics;

	VkDevice m_device;
	VkShaderModule m_module;

private:
	friend class VulkanGraphics;
	friend class VulkanPipeline;

	bool LoadFromArray(const Array<char>& data);

	VkShaderModule GetModule();

public:
	VulkanShader(
		std::shared_ptr<VulkanGraphics> graphics,
		VkDevice device, 
		std::shared_ptr<Logger> logger, 
		const String& name, 
		const String& entryPoint,
		GraphicsPipelineStage stage);

	virtual ~VulkanShader();

	virtual void FreeResources();
	virtual String GetName();

};