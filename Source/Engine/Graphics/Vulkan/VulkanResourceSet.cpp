#include "Pch.h"

#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Graphics/Vulkan/VulkanResourceSet.h"
#include "Engine/Graphics/Vulkan/VulkanResourceSetPool.h"
#include "Engine/Graphics/Vulkan/VulkanUniformBuffer.h"
#include "Engine/Graphics/Vulkan/VulkanImageView.h"
#include "Engine/Graphics/Vulkan/VulkanSampler.h"

#include "Engine/Engine/Logging.h"
#include "Engine/Utilities/Statistic.h"

Statistic Stat_Rendering_Vulkan_ResourceSetCount("Rendering/Vulkan/Resource Set Count", StatisticFrequency::Persistent, StatisticFormat::Integer);

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
	Stat_Rendering_Vulkan_ResourceSetCount.Add(1);
}

VulkanResourceSet::~VulkanResourceSet()
{
	Stat_Rendering_Vulkan_ResourceSetCount.Add(-1);
	FreeResources();
}

void VulkanResourceSet::FreeResources()
{
	if (m_descriptorSet != nullptr)
	{
		m_pool->FreeDescriptorSet(m_descriptorSet);
		m_descriptorSet = nullptr;
	}
}

String VulkanResourceSet::GetName()
{
	return m_name;
}

void VulkanResourceSet::UpdateDescriptorSet()
{
	ScopeLock lock(m_updateMutex);

	//printf("Updating resource set (%s): 0x%08x (thread %i)\n", m_name.c_str(), this, std::this_thread::get_id());
	if (m_descriptorSet != nullptr)
	{
		m_pool->FreeDescriptorSet(m_descriptorSet);
		m_descriptorSet = nullptr;
	}
	m_descriptorSet = m_pool->AllocDescriptorSet(m_layout, m_currentBindings);
}

void VulkanResourceSet::UpdateResources()
{
	ScopeLock lock(m_updateMutex);

	bool bUpdateRequired = false;

	// If any uniform buffers have been changed, we need to update.
	for (auto& binding : m_currentBindings)
	{
		if (binding.type == VulkanResourceSetBindingType::UniformBuffer)
		{
			if (binding.vkUniformBuffer != binding.uniformBuffer->GetGpuBuffer())
			{
				binding.vkUniformBuffer = binding.uniformBuffer->GetGpuBuffer();
				bUpdateRequired = true;
			}
		}
	}

	if (bUpdateRequired || m_descriptorSet == nullptr)
	{
		UpdateDescriptorSet();
	}
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
	std::shared_ptr<VulkanUniformBuffer> vkBuffer = std::static_pointer_cast<VulkanUniformBuffer>(buffer);

	VulkanResourceSetBinding& binding = GetBinding(location, arrayIndex);
	if (binding.location != location ||
		binding.arrayIndex != arrayIndex ||
		binding.type != VulkanResourceSetBindingType::UniformBuffer ||
		binding.uniformBuffer != vkBuffer)
	{
		binding.location = location;
		binding.arrayIndex = arrayIndex;
		binding.type = VulkanResourceSetBindingType::UniformBuffer;
		binding.uniformBuffer = vkBuffer;
		binding.vkUniformBuffer = vkBuffer->GetGpuBuffer();

		UpdateDescriptorSet();
	}

	return true;
}

bool VulkanResourceSet::UpdateBinding(int location, int arrayIndex, std::shared_ptr<IGraphicsSampler> sampler, std::shared_ptr<IGraphicsImageView> imageView)
{
	std::shared_ptr<VulkanSampler> vkSampler = std::static_pointer_cast<VulkanSampler>(sampler);
	std::shared_ptr<VulkanImageView> vkImageView = std::static_pointer_cast<VulkanImageView>(imageView);

	VulkanResourceSetBinding& binding = GetBinding(location, arrayIndex);
	if (binding.location != location ||
		binding.arrayIndex != arrayIndex ||
		binding.type != VulkanResourceSetBindingType::Sampler ||
		binding.sampler != vkSampler ||
		binding.samplerImageView != vkImageView)
	{
		binding.location = location;
		binding.arrayIndex = arrayIndex;
		binding.type = VulkanResourceSetBindingType::Sampler;
		binding.sampler = vkSampler;
		binding.samplerImageView = vkImageView;

		UpdateDescriptorSet();
	}

	return true;
}

void VulkanResourceSet::GetUniformBufferOffsets(uint32_t* destination, int* count)
{
	for (auto& binding : m_currentBindings)
	{
		if (binding.type == VulkanResourceSetBindingType::UniformBuffer)
		{
			assert(*count < VulkanGraphics::MAX_BOUND_UBO);
			destination[(*count)++] = binding.uniformBuffer->GetGpuBufferOffset();
		}
	}
}

void VulkanResourceSet::GetDescriptorSets(VkDescriptorSet* destination, int* count)
{
	assert(*count < VulkanGraphics::MAX_BOUND_DESCRIPTOR_SETS);
	destination[(*count)++] = m_descriptorSet->set;
}
