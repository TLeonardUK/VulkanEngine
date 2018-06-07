#pragma once

#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Graphics/Vulkan/VulkanResourceSetPool.h"
#include "Engine/Graphics/Vulkan/VulkanResourceSet.h"

#include "Engine/Engine/Logging.h"

#include <cassert>

VulkanResourceSetPool::VulkanResourceSetPool(
	VkDevice device,
	std::shared_ptr<Logger> logger,
	const String& name
)
	: m_device(device)
	, m_logger(logger)
	, m_name(name)
{
}

VulkanResourceSetPool::~VulkanResourceSetPool()
{
	FreeResources();
}

void VulkanResourceSetPool::FreeResources()
{
	// Free all buffers allocated by us.	
	// todo: this doesn't seem like the way to deal with this ...
	for (std::shared_ptr<VulkanResourceSet>& buffer : m_allocatedSets)
	{
		buffer->FreeResources();
	}
	m_allocatedSets.clear();

	if (m_descriptorPool != nullptr)
	{
		vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
		m_descriptorPool = nullptr;
	}
}

bool VulkanResourceSetPool::Build()
{
	m_logger->WriteInfo(LogCategory::Vulkan, "Builiding new resource set pool: %s", m_name.c_str());

	Array<VkDescriptorPoolSize> poolSizes(2);

	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = MaxAllocations;

	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = MaxAllocations;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = MaxAllocations;

	CheckVkResultReturnOnFail(vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool));

	return true;
}

std::shared_ptr<IGraphicsResourceSet> VulkanResourceSetPool::Allocate(const GraphicsResourceSetDescription& description)
{
	Array<VkDescriptorSetLayoutBinding> bindings(description.bindings.size());

	for (int i = 0; i < description.bindings.size(); i++)
	{
		const GraphicsResourceSetBinding& bindingDescription = description.bindings[i];
		VkDescriptorSetLayoutBinding& binding = bindings[i];

		binding.binding = bindingDescription.location;
		binding.descriptorCount = bindingDescription.arrayLength;
		binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
		binding.pImmutableSamplers = nullptr;

		switch (bindingDescription.type)
		{
		case GraphicsBindingType::UniformBufferObject:
			{
				binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				break;
			}
		case GraphicsBindingType::Sampler:
			{
				binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;;
				break;
			}
		default:
			{
				assert(false);
			}
		}
	}

	// Create layout.
	// todo: cache these, don't create ones for each set if same bindings.
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	VkDescriptorSetLayout descriptorSetLayout;
	CheckVkResultReturnValueOnFail(vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &descriptorSetLayout), nullptr);

	// Create actual set.
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayout;

	VkDescriptorSet descriptorSet;
	CheckVkResultReturnValueOnFail(vkAllocateDescriptorSets(m_device, &allocInfo, &descriptorSet), nullptr);

	// Done
	std::shared_ptr<VulkanResourceSet> resourceSet = std::make_shared<VulkanResourceSet>(
		m_device, 
		m_logger, 
		StringFormat("%s Set", m_name.c_str()), 
		descriptorSetLayout, 
		descriptorSet,
		m_descriptorPool);

	m_allocatedSets.push_back(resourceSet);

	return resourceSet;

}