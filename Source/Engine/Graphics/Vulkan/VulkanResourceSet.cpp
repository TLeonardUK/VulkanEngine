#pragma once

#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Graphics/Vulkan/VulkanResourceSet.h"
#include "Engine/Graphics/Vulkan/VulkanUniformBuffer.h"
#include "Engine/Graphics/Vulkan/VulkanImageView.h"
#include "Engine/Graphics/Vulkan/VulkanSampler.h"

#include "Engine/Engine/Logging.h"

#include <cassert>

VulkanResourceSet::VulkanResourceSet(
	VkDevice device,
	std::shared_ptr<Logger> logger,
	const String& name,
	VkDescriptorSetLayout layout,
	VkDescriptorSet set,
	VkDescriptorPool pool
)
	: m_device(device)
	, m_logger(logger)
	, m_name(name)
	, m_layout(layout)
	, m_set(set)
	, m_pool(pool)
{
}

VulkanResourceSet::~VulkanResourceSet()
{
	FreeResources();
}

void VulkanResourceSet::FreeResources()
{
	if (m_set != nullptr)
	{
		vkFreeDescriptorSets(m_device, m_pool, 1, &m_set);
		m_set = nullptr;
	}
	if (m_layout != nullptr)
	{
		vkDestroyDescriptorSetLayout(m_device, m_layout, nullptr);
		m_layout = nullptr;
	}
}

VkDescriptorSetLayout VulkanResourceSet::GetLayout()
{
	return m_layout;
}

VkDescriptorSet VulkanResourceSet::GetSet()
{
	return m_set;
}

bool VulkanResourceSet::UpdateBinding(int location, int arrayIndex, std::shared_ptr<IGraphicsUniformBuffer> buffer)
{
	std::shared_ptr<VulkanUniformBuffer> vulkanBuffer = std::dynamic_pointer_cast<VulkanUniformBuffer>(buffer);

	VkBuffer vkBuffer = vulkanBuffer->GetGpuBuffer().Buffer;

	VkDescriptorBufferInfo bufferInfo = { };
	bufferInfo.buffer = vkBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = vulkanBuffer->GetDataSize();

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = m_set;
	descriptorWrite.dstBinding = location;
	descriptorWrite.dstArrayElement = arrayIndex;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &bufferInfo;
	descriptorWrite.pImageInfo = nullptr; 
	descriptorWrite.pTexelBufferView = nullptr; 

	vkUpdateDescriptorSets(m_device, 1, &descriptorWrite, 0, nullptr);

	return true;
}

bool VulkanResourceSet::UpdateBinding(int location, int arrayIndex, std::shared_ptr<IGraphicsSampler> sampler, std::shared_ptr<IGraphicsImageView> imageView)
{
	std::shared_ptr<VulkanSampler> vulkanSampler = std::dynamic_pointer_cast<VulkanSampler>(sampler);
	std::shared_ptr<VulkanImageView> vulkanImageView = std::dynamic_pointer_cast<VulkanImageView>(imageView);

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = vulkanImageView->GetImageView();
	imageInfo.sampler = vulkanSampler->GetSampler();

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = m_set;
	descriptorWrite.dstBinding = location;
	descriptorWrite.dstArrayElement = arrayIndex;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = nullptr;
	descriptorWrite.pImageInfo = &imageInfo;
	descriptorWrite.pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(m_device, 1, &descriptorWrite, 0, nullptr);

	return true;
}