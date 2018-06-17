#pragma once

#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Graphics/Vulkan/VulkanResourceSetPool.h"
#include "Engine/Graphics/Vulkan/VulkanResourceSet.h"
#include "Engine/Graphics/Vulkan/VulkanUniformBuffer.h"
#include "Engine/Graphics/Vulkan/VulkanImageView.h"
#include "Engine/Graphics/Vulkan/VulkanSampler.h"

#include "Engine/Engine/Logging.h"

#include <cassert>

bool VulkanResourceSetBinding::EqualTo(const VulkanResourceSetBinding& other) const
{
	if (type != other.type ||
		location != other.location ||
		arrayIndex != other.arrayIndex)
	{
		return false;
	}

	if (type == VulkanResourceSetBindingType::Sampler)
	{
		if (sampler != other.sampler ||
			samplerImageView != other.samplerImageView)
		{
			return false;
		}
	}
	else if (type == VulkanResourceSetBindingType::UniformBuffer)
	{
		if (uniformBuffer != other.uniformBuffer)
		{
			return false;
		}
	}
	else
	{
		assert(false);
		return false;
	}

	return true;
}

bool VulkanResourceSetBinding::BindingsEqualTo(const Array<VulkanResourceSetBinding>& first, const Array<VulkanResourceSetBinding>& second)
{
	if (first.size() != second.size())
	{
		return false;
	}

	for (int i = 0; i < first.size(); i++)
	{
		if (!first[i].EqualTo(second[i]))
		{
			return false;
		}
	}

	return true;
}

VulkanResourceSetPool::VulkanResourceSetPool(
	VkDevice device,
	std::shared_ptr<Logger> logger,
	const String& name,
	std::shared_ptr<VulkanGraphics> graphics
)
	: m_device(device)
	, m_logger(logger)
	, m_name(name)
	, m_graphics(graphics)
{
}

VulkanResourceSetPool::~VulkanResourceSetPool()
{
	FreeResources();
}

void VulkanResourceSetPool::FreeResources()
{
	for (auto& layout : m_layouts)
	{
		vkDestroyDescriptorSetLayout(m_device, layout.layout, nullptr);
	}
	m_layouts.clear();

	for (auto& pool : m_pools)
	{
		vkDestroyDescriptorPool(m_device, pool, nullptr);
	}
	m_pools.clear();
}

String VulkanResourceSetPool::GetName()
{
	return m_name;
}

bool VulkanResourceSetPool::Build()
{
	m_logger->WriteInfo(LogCategory::Vulkan, "Builiding new resource set pool: %s", m_name.c_str());
	return CreateNewPool();
}

bool VulkanResourceSetPool::CreateNewPool()
{
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

	VkDescriptorPool pool;
	CheckVkResultReturnOnFail(vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &pool));

	m_pools.insert(m_pools.begin(), pool);

	return true;
}

bool VulkanResourceSetPool::AllocateSet(VkDescriptorSetLayout layout, VkDescriptorSet& resultSet, VkDescriptorPool& resultPool)
{
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &layout;

	for (VkDescriptorPool& pool : m_pools)
	{
		allocInfo.descriptorPool = pool;
		if (vkAllocateDescriptorSets(m_device, &allocInfo, &resultSet) == VK_SUCCESS)
		{
			resultPool = pool;
			return true;
		}
	}

	return false;
}

bool VulkanResourceSetPool::PruneDescriptors()
{
	for (auto iter = m_descriptors.begin(); iter != m_descriptors.end(); )
	{
		CachedDescriptors& descriptor = *iter;

		if (descriptor.lastFrameUsed > m_graphics->GetSafeRecycleFrameIndex())
		{
			vkFreeDescriptorSets(m_device, descriptor.pool, 1, &descriptor.set);
			iter = m_descriptors.erase(iter);
		}
		else
		{
			iter++;
		}
	}

	return true;
}

bool VulkanResourceSetPool::WriteDescriptorSet(VkDescriptorSet set, const Array<VulkanResourceSetBinding>& bindings)
{
	Array<VkDescriptorBufferInfo> bufferInfo(bindings.size());
	Array<VkDescriptorImageInfo> imageInfo(bindings.size());
	Array<VkWriteDescriptorSet> writes(bindings.size());

	int bufferInfoIndex = 0;
	int imageInfoIndex = 0;

	for (int i = 0; i < bindings.size(); i++)
	{
		const VulkanResourceSetBinding& binding = bindings[i];

		VkWriteDescriptorSet& write = writes[i];
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = set;
		write.dstBinding = binding.location;
		write.dstArrayElement = binding.arrayIndex;

		switch (binding.type)
		{
		case VulkanResourceSetBindingType::Sampler:
			{
				// todo: this may not be right layout?
				imageInfo[imageInfoIndex].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo[imageInfoIndex].imageView = binding.samplerImageView->GetImageView();
				imageInfo[imageInfoIndex].sampler = binding.sampler->GetSampler();

				write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				write.descriptorCount = 1;
				write.pBufferInfo = nullptr;
				write.pImageInfo = &imageInfo[imageInfoIndex];
				write.pTexelBufferView = nullptr;

				imageInfoIndex++;

				break;
			}
		case VulkanResourceSetBindingType::UniformBuffer:
			{
				bufferInfo[bufferInfoIndex].buffer = binding.uniformBuffer->GetGpuBuffer().Buffer;
				bufferInfo[bufferInfoIndex].offset = 0;
				bufferInfo[bufferInfoIndex].range = binding.uniformBuffer->GetDataSize();

				write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				write.descriptorCount = 1;
				write.pBufferInfo = &bufferInfo[bufferInfoIndex];
				write.pImageInfo = nullptr;
				write.pTexelBufferView = nullptr;

				bufferInfoIndex++;
				break;
			}
		}
	}

	vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

	return true;
}

VkDescriptorSet VulkanResourceSetPool::RequestDescriptorSetForThisFrame(VkDescriptorSetLayout layout, const Array<VulkanResourceSetBinding>& bindings)
{
	// Find one that matches our layout and binding exactly, we can just return
	// it as-is regardless of which frame it was last sent to the gpu as we don't need to update it.
	for (auto& descriptor : m_descriptors)
	{
		if (descriptor.layout != layout)
		{
			continue;
		}

		if (!VulkanResourceSetBinding::BindingsEqualTo(descriptor.currentBindings, bindings))
		{
			continue;
		}
		
		descriptor.lastFrameUsed = m_graphics->GetFrameIndex();
		return descriptor.set;
	}

	// Find one using the same layout, but that hasn't been used for enough frames to ensure
	// we don't trample on it while the gpu is reading, update this descriptor and return it.
	for (auto& descriptor : m_descriptors)
	{
		if (descriptor.layout != layout)
		{
			continue;
		}

		if (descriptor.lastFrameUsed > m_graphics->GetSafeRecycleFrameIndex())
		{
			continue;
		}

		WriteDescriptorSet(descriptor.set, bindings);

		descriptor.currentBindings = bindings;
		descriptor.lastFrameUsed = m_graphics->GetFrameIndex();
		return descriptor.set;
	}

	// If neither of the above worked, its time to create a new descriptor.
	// Create actual set.		
	CachedDescriptors descriptor;
	descriptor.currentBindings = bindings;
	descriptor.lastFrameUsed = m_graphics->GetFrameIndex();
	descriptor.layout = layout;

	if (!AllocateSet(layout, descriptor.set, descriptor.pool))
	{
		PruneDescriptors();

		if (!AllocateSet(layout, descriptor.set, descriptor.pool))
		{
			if (!CreateNewPool())
			{
				m_logger->WriteError(LogCategory::Vulkan, "Failed to allocate new descriptor pool when new descriptor was required.");
				return nullptr;
			}

			if (!AllocateSet(layout, descriptor.set, descriptor.pool))
			{
				m_logger->WriteError(LogCategory::Vulkan, "Failed to allocate new descriptor after pruning and attempting to create new pools.");
				return nullptr;
			}
		}
	}

	WriteDescriptorSet(descriptor.set, descriptor.currentBindings);

	m_descriptors.push_back(descriptor);

	//m_logger->WriteInfo(LogCategory::Vulkan, "Allocated new descriptor: %i", m_descriptors.size());

	return descriptor.set;
}

VkDescriptorSetLayout VulkanResourceSetPool::RequestLayout(const GraphicsResourceSetDescription& description)
{
	// See if we have a cached layout.
	for (auto& layout : m_layouts)
	{
		if (layout.description.EqualTo(description))
		{
			return layout.layout;
		}
	}

	// Nope, create a new one, generate binding data.
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
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	VkDescriptorSetLayout descriptorSetLayout;
	CheckVkResultReturnValueOnFail(vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &descriptorSetLayout), nullptr);

	CachedLayout layout;
	layout.description = description;
	layout.layout = descriptorSetLayout;
	m_layouts.push_back(layout);

	m_logger->WriteInfo(LogCategory::Vulkan, "Allocated new layout.");

	return descriptorSetLayout;
}

std::shared_ptr<IGraphicsResourceSet> VulkanResourceSetPool::Allocate(const GraphicsResourceSetDescription& description)
{
	VkDescriptorSetLayout layout = RequestLayout(description);
	if (layout == nullptr)
	{
		return nullptr;
	}

	return std::make_shared<VulkanResourceSet>(
		m_device,
		m_logger,
		StringFormat("%s Set", m_name.c_str()),
		shared_from_this(),
		layout);
}