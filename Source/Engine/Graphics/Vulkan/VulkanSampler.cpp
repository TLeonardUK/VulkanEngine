#pragma once

#include "Engine/Graphics/Vulkan/VulkanSampler.h"
#include "Engine/Graphics/Vulkan/VulkanEnums.h"
#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Graphics/Vulkan/VulkanMemoryAllocator.h"

#include "Engine/Engine/Logging.h"

#include <cassert>
#include <vk_mem_alloc.h>

VulkanSampler::VulkanSampler(
	VkDevice device,
	std::shared_ptr<Logger> logger,
	const String& name
)
	: m_device(device)
	, m_logger(logger)
	, m_name(name)
	, m_sampler(nullptr)
{
}

VulkanSampler::~VulkanSampler()
{
	FreeResources();
}

VkSampler VulkanSampler::GetSampler()
{
	return m_sampler;
}

void VulkanSampler::FreeResources()
{
	if (m_sampler != nullptr)
	{
		vkDestroySampler(m_device, m_sampler, nullptr);
		m_sampler = nullptr;
	}
}

bool VulkanSampler::Build(const SamplerDescription& description)
{
	m_logger->WriteInfo(LogCategory::Vulkan, "Builiding new sampler: %s", m_name.c_str());

	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = GraphicsFilterToVkFilter(description.MagnificationFilter);
	samplerInfo.minFilter = GraphicsFilterToVkFilter(description.MinificationFilter);
	samplerInfo.addressModeU = GraphicsAddressModeToVkAddressMode(description.AddressModeU);
	samplerInfo.addressModeV = GraphicsAddressModeToVkAddressMode(description.AddressModeV);
	samplerInfo.addressModeW = GraphicsAddressModeToVkAddressMode(description.AddressModeW);
	samplerInfo.anisotropyEnable = (description.MaxAnisotropy > 0);
	samplerInfo.maxAnisotropy = static_cast<float>(description.MaxAnisotropy);
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = GraphicsMipMapModeToVkSamplerMipMapMode(description.MipmapMode);
	samplerInfo.mipLodBias = description.MipLodBias;
	samplerInfo.minLod = description.MinLod;
	samplerInfo.maxLod = description.MaxLod;

	CheckVkResultReturnOnFail(vkCreateSampler(m_device, &samplerInfo, nullptr, &m_sampler));

	return true;
}
