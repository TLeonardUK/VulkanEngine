#pragma once

#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Graphics/Vulkan/VulkanResourceSet.h"
#include "Engine/Graphics/Vulkan/VulkanResourceSetPool.h"
#include "Engine/Graphics/Vulkan/VulkanUniformBuffer.h"
#include "Engine/Graphics/Vulkan/VulkanImageView.h"
#include "Engine/Graphics/Vulkan/VulkanSampler.h"

#include "Engine/Engine/Logging.h"

#include <cassert>
#include <algorithm>

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
	// todo: If we know we haven't been updated, keep previous descriptor set?
	// todo: If uniforms are shared, don't duplicate them.

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
	
	// Insert to keep location value in order.
	for (int i = 0; i < m_currentBindings.size(); i++)
	{
		VulkanResourceSetBinding& binding = m_currentBindings[i];
		if (newBinding.location < binding.location)
		{
			m_currentBindings.insert(m_currentBindings.begin() + i, newBinding);
			return m_currentBindings[i];
		}
	}

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

void VulkanResourceSet::GetUniformBufferOffsets(Array<uint32_t>& destination)
{
	for (auto& binding : m_currentBindings)
	{
		if (binding.type == VulkanResourceSetBindingType::UniformBuffer)
		{
			destination.push_back(binding.uniformBuffer->GetGpuBufferOffset());
		}
	}
}