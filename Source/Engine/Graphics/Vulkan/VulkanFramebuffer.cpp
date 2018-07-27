#include "Pch.h"

#include "Engine/Graphics/Vulkan/VulkanFramebuffer.h"
#include "Engine/Graphics/Vulkan/VulkanEnums.h"
#include "Engine/Graphics/Vulkan/VulkanExtensions.h"
#include "Engine/Graphics/Vulkan/VulkanRenderPass.h"
#include "Engine/Graphics/Vulkan/VulkanImageView.h"

#include "Engine/Engine/Logging.h"

VulkanFramebuffer::VulkanFramebuffer(
	std::shared_ptr<VulkanGraphics> graphics,
	VkDevice device,
	std::shared_ptr<Logger> logger,
	const String& name
)
	: m_graphics(graphics)
	, m_device(device)
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
		m_graphics->QueueDisposal([m_device = m_device, m_framebuffer = m_framebuffer]() {
			vkDestroyFramebuffer(m_device, m_framebuffer, nullptr);
		});
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

const Array<std::shared_ptr<VulkanImageView>>& VulkanFramebuffer::GetAttachments()
{
	return m_attachments;
}

bool VulkanFramebuffer::Build(const GraphicsFramebufferSettings& settings)
{
	m_logger->WriteInfo(LogCategory::Vulkan, "Builiding new framebuffer: %s", m_name.c_str());

	Array<VkImageView> attachments;
	for (std::shared_ptr<IGraphicsImageView> view : settings.attachments)
	{
		std::shared_ptr<VulkanImageView> vkView = std::static_pointer_cast<VulkanImageView>(view);
		m_attachments.push_back(vkView);

		attachments.push_back(vkView->GetImageView());
	}

	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = std::static_pointer_cast<VulkanRenderPass>(settings.renderPass)->GetRenderPass();
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