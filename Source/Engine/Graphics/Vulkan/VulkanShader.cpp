#pragma once

#include "Engine/Graphics/Vulkan/VulkanShader.h"

#include "Engine/Engine/Logging.h"

VulkanShader::VulkanShader(
	VkDevice device, 
	std::shared_ptr<Logger> logger, 
	const String& name, 
	const String& entryPoint, 
	GraphicsPipelineStage stage
)
	: m_device(device)
	, m_logger(logger)
	, m_name(name)
	, m_entryPoint(entryPoint)
	, m_stage(stage)
{
}

VulkanShader::~VulkanShader()
{
	FreeResources();
}

void VulkanShader::FreeResources()
{
	if (m_module != nullptr)
	{
		vkDestroyShaderModule(m_device, m_module, nullptr);
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