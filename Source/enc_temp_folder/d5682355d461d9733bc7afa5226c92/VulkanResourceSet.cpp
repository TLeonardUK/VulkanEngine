#pragma once

#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Graphics/Vulkan/VulkanResourceSet.h"
#include "Engine/Graphics/Vulkan/VulkanResourceSetPool.h"
#include "Engine/Graphics/Vulkan/VulkanUniformBuffer.h"
#include "Engine/Graphics/Vulkan/VulkanImageView.h"
#include "Engine/Graphics/Vulkan/VulkanSampler.h"

#include "Engine/Engine/Logging.h"

#include <cassert>

VulkanResourceSet::VulkanResourceSet(
	VkDevice device,
	std::shared_ptr<Logger> logger,
	const String& name, 
	std::shared_ptr<VulkanResourceSetPool> pool,
	VkDescriptorSetLayout layout
)
	: m_device(device)
	, m_logger(logger)
	, m_name(name)
	, m_pool(pool)
	, m_layout(layout)
{
}

VulkanResourceSet::~VulkanResourceSet()
{
	FreeResources();
}

void VulkanResourceSet::FreeResources()
{
}

String VulkanResourceSet::GetName()
{
	return m_name;
}

VkDescriptorSetLayout VulkanResourceSet::GetLayout()
{
	return m_layout;
}

VkDescriptorSet VulkanResourceSet::ConsumeSet()
{
	return m_pool->RequestDescriptorSetForThisFrame(m_layout, m_currentBindings);
}

VulkanResourceSetBinding& VulkanResourceSet::GetBinding(int location, int arrayIndex)
{
	for (auto& binding : m_currentBindings)
	{
		if (binding.location == location && binding.arrayIndex == arrayIndex)
		{
			return binding;
		}
	}

	VulkanResourceSetBinding newBinding;
	newBinding.arrayIndex = arrayIndex;
	newBinding.location = location;
	
	m_currentBindings.push_back(newBinding);

	return m_currentBindings[m_currentBindings.size() - 1];
}

bool VulkanResourceSet::UpdateBinding(int location, int arrayIndex, std::shared_ptr<IGraphicsUniformBuffer> buffer)
{
	VulkanResourceSetBinding& binding = GetBinding(location, arrayIndex);
	binding.location = location;
	binding.arrayIndex = arrayIndex;
	binding.type = VulkanResourceSetBindingType::UniformBuffer;
	binding.uniformBuffer = std::dynamic_pointer_cast<VulkanUniformBuffer>(buffer);

	return true;
}

bool VulkanResourceSet::UpdateBinding(int location, int arrayIndex, std::shared_ptr<IGraphicsSampler> sampler, std::shared_ptr<IGraphicsImageView> imageView)
{
	VulkanResourceSetBinding& binding = GetBinding(location, arrayIndex);
	binding.location = location;
	binding.arrayIndex = arrayIndex;
	binding.type = VulkanResourceSetBindingType::Sampler;
	binding.sampler = std::dynamic_pointer_cast<VulkanSampler>(sampler);
	binding.samplerImageView = std::dynamic_pointer_cast<VulkanImageView>(imageView);

	return true;
}