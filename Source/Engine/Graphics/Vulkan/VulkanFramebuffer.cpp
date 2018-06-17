#pragma once

#include "Engine/Graphics/Vulkan/VulkanFramebuffer.h"
#include "Engine/Graphics/Vulkan/VulkanEnums.h"
#include "Engine/Graphics/Vulkan/VulkanExtensions.h"
#include "Engine/Graphics/Vulkan/VulkanRenderPass.h"
#include "Engine/Graphics/Vulkan/VulkanImageView.h"

#include "Engine/Engine/Logging.h"

#include <cassert>

VulkanFramebuffer::VulkanFramebuffer(
	VkDevice device,
	std::shared_ptr<Logger> logger,
	const String& name
)
	: m_device(device)
	, m_logger(logger)
	, m_name(name)
{
}

VulkanFramebuffer::~VulkanFramebuffer()
{
	FreeResources();
}

void VulkanFramebuffer::FreeResources()
{
	if (m_framebuffer != nullptr)
	{
		vkDestroyFramebuffer(m_device, m_framebuffer, nullptr);
		m_framebuffer = nullptr;
	}
}

String VulkanFramebuffer::GetName()
{
	return m_name;
}

VkFramebuffer VulkanFramebuffer::GetFramebuffer()
{
	return m_framebuffer;
}

int VulkanFramebuffer::GetWidth()
{
	return m_width;
}

int VulkanFramebuffer::GetHeight()
{
	return m_height;
}

bool VulkanFramebuffer::Build(const GraphicsFramebufferSettings& settings)
{
	m_logger->WriteInfo(LogCategory::Vulkan, "Builiding new framebuffer: %s", m_name.c_str());

	Array<VkImageView> attachments;
	for (std::shared_ptr<IGraphicsImageView> view : settings.attachments)
	{
		attachments.push_back(std::dynamic_pointer_cast<VulkanImageView>(view)->GetImageView());
	}

	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = std::dynamic_pointer_cast<VulkanRenderPass>(settings.renderPass)->GetRenderPass();
	framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	framebufferInfo.pAttachments = attachments.data();
	framebufferInfo.width = settings.width;
	framebufferInfo.height = settings.height;
	framebufferInfo.layers = 1;

	m_width = settings.width;
	m_height = settings.height;

	CheckVkResultReturnOnFail(vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_framebuffer));

	SetVulkanMarkerName(m_device, VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, (uint64_t)m_framebuffer, m_name);

	return true;
}