#include "Pch.h"

#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Graphics/Vulkan/VulkanResourceSet.h"
#include "Engine/Graphics/Vulkan/VulkanResourceSetInstance.h"
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
}

String VulkanResourceSet::GetName()
{
	return m_name;
}

VkDescriptorSet VulkanResourceSet::ConsumeSet()
{
	VkDescriptorSet output;
	if (m_pool->RequestDescriptorSetForThisFrame(m_layout, m_currentBindings, output))
	{
		return output;
	}

	return nullptr;
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
	binding.uniformBuffer = std::static_pointer_cast<VulkanUniformBuffer>(buffer);

	return true;
}

bool VulkanResourceSet::UpdateBinding(int location, int arrayIndex, std::shared_ptr<IGraphicsSampler> sampler, std::shared_ptr<IGraphicsImageView> imageView)
{
	VulkanResourceSetBinding& binding = GetBinding(location, arrayIndex);
	binding.location = location;
	binding.arrayIndex = arrayIndex;
	binding.type = VulkanResourceSetBindingType::Sampler;
	binding.sampler = std::static_pointer_cast<VulkanSampler>(sampler);
	binding.samplerImageView = std::static_pointer_cast<VulkanImageView>(imageView);

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

std::shared_ptr<IGraphicsResourceSetInstance> VulkanResourceSet::NewInstance()
{
	std::shared_ptr<IGraphicsResourceSetInstance> instance = std::make_shared<VulkanResourceSetInstance>();
	UpdateInstance(instance);
	return instance;
}

void VulkanResourceSet::UpdateInstance(std::shared_ptr<IGraphicsResourceSetInstance>& instance)
{
	VulkanResourceSetInstance* vkInstance = static_cast<VulkanResourceSetInstance*>(&*instance);
	vkInstance->m_sets[0] = ConsumeSet();
	vkInstance->m_setCount = 1;
	vkInstance->m_uboCount = 0;

	GetUniformBufferOffsets(vkInstance->m_uniformBufferOffsets, &vkInstance->m_uboCount);
}
