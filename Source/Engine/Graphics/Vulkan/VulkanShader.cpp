#include "Pch.h"

#include "Engine/Graphics/Vulkan/VulkanShader.h"

#include "Engine/Engine/Logging.h"
#include "Engine/Utilities/Statistic.h"

Statistic Stat_Rendering_Vulkan_ShaderCount("Rendering/Vulkan/Shader Count", StatisticFrequency::Persistent, StatisticFormat::Integer);

VulkanShader::VulkanShader(
	std::shared_ptr<VulkanGraphics> graphics,
	VkDevice device, 
	std::shared_ptr<Logger> logger, 
	const String& name, 
	const String& entryPoint, 
	GraphicsPipelineStage stage
)
	: m_graphics(graphics)
	, m_device(device)
	, m_logger(logger)
	, m_name(name)
	, m_entryPoint(entryPoint)
	, m_stage(stage)
{
	Stat_Rendering_Vulkan_ShaderCount.Add(1);
}

VulkanShader::~VulkanShader()
{
	Stat_Rendering_Vulkan_ShaderCount.Add(-1);
	FreeResources();
}

void VulkanShader::FreeResources()
{
	if (m_module != nullptr)
	{
		m_graphics->QueueDisposal([m_device = m_device, m_module = m_module]() {
			vkDestroyShaderModule(m_device, m_module, nullptr);
		});
		m_module = nullptr;
	}
}

String VulkanShader::GetName()
{
	return m_name;
}

VkShaderModule VulkanShader::GetModule()
{
	return m_module;
}

bool VulkanShader::LoadFromArray(const Array<char>& data)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = data.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(data.data());

	m_logger->WriteInfo(LogCategory::Vulkan, "Loading shader '%s'", m_name.c_str());

	CheckVkResultReturnOnFail(vkCreateShaderModule(m_device, &createInfo, nullptr, &m_module));

	m_logger->WriteSuccess(LogCategory::Vulkan, "Loaded shader '%s' successfully.", m_name.c_str());

	return true;
}