#include "Pch.h"

#include "Engine/Graphics/Vulkan/VulkanRenderPass.h"
#include "Engine/Graphics/Vulkan/VulkanEnums.h"

#include "Engine/Engine/Logging.h"

VulkanRenderPass::VulkanRenderPass(
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

VulkanRenderPass::~VulkanRenderPass()
{
	FreeResources();
}

void VulkanRenderPass::FreeResources()
{
	if (m_renderPass != nullptr)
	{
		m_graphics->QueueDisposal([m_device = m_device, m_renderPass = m_renderPass]() {
			vkDestroyRenderPass(m_device, m_renderPass, nullptr);
		});
		m_renderPass = nullptr;
	}
}

String VulkanRenderPass::GetName()
{
	return m_name;
}

VkRenderPass VulkanRenderPass::GetRenderPass()
{
	return m_renderPass;
}

GraphicsRenderPassSettings VulkanRenderPass::GetSettings()
{
	return m_settings;
}

bool VulkanRenderPass::Build(const GraphicsRenderPassSettings& settings)
{
	m_logger->WriteInfo(LogCategory::Vulkan, "Builiding new render pass: %s", m_name.c_str());

	m_settings = settings;

	Array<VkAttachmentDescription> attachmentDescriptions;
	Array<VkAttachmentReference> colorAttachmentsRefs;
	Array<VkAttachmentReference> depthStencilAttachmentsRefs;

	for (const GraphicsRenderPassAttachment& attachment : settings.attachments)
	{
		if (attachment.bIsDepthStencil)
		{
			VkAttachmentDescription depthAttachment = {};
			depthAttachment.format = GraphicsFormatToVkFormat(attachment.format); // todo: check correct format
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;// VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			attachmentDescriptions.push_back(depthAttachment);

			VkAttachmentReference depthAttachmentRef = {};
			depthAttachmentRef.attachment = static_cast<uint32_t>(attachmentDescriptions.size() - 1);
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; 
			depthStencilAttachmentsRefs.push_back(depthAttachmentRef);
		}
		else
		{
			VkAttachmentDescription colorAttachment = {};
			colorAttachment.format = GraphicsFormatToVkFormat(attachment.format); // todo: check correct format
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachment.finalLayout = settings.transitionToPresentFormat ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachmentDescriptions.push_back(colorAttachment);

			VkAttachmentReference colorAttachmentRef = {};
			colorAttachmentRef.attachment = static_cast<uint32_t>(attachmentDescriptions.size() - 1);
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachmentsRefs.push_back(colorAttachmentRef);
		}
	}

	assert(depthStencilAttachmentsRefs.size() <= 1);

	Array<VkSubpassDescription> subPasses;
	Array<VkSubpassDependency> subPassesDependencies;

	for (const GraphicsRenderPassSubPass pass : settings.subPasses)
	{
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentsRefs.size());
		subpass.pColorAttachments = colorAttachmentsRefs.data();
		if (!depthStencilAttachmentsRefs.empty())
		{
			subpass.pDepthStencilAttachment = &depthStencilAttachmentsRefs[0];
		}

		subPasses.push_back(subpass);
	}

	for (const GraphicsRenderPassSubPassDependency dep : settings.subPassDependencies)
	{
		VkSubpassDependency dependency = {};
		dependency.srcSubpass = dep.sourcePass == GraphicsExternalPassIndex ? VK_SUBPASS_EXTERNAL : (int)dep.sourcePass;
		dependency.dstSubpass = dep.destPass == GraphicsExternalPassIndex ? VK_SUBPASS_EXTERNAL : (int)dep.destPass;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		if (dep.sourceAccessMask == GraphicsAccessMask::Read || dep.sourceAccessMask == GraphicsAccessMask::ReadWrite)
		{
			dependency.srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		}
		if (dep.sourceAccessMask == GraphicsAccessMask::Write || dep.sourceAccessMask == GraphicsAccessMask::ReadWrite)
		{
			dependency.srcAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}

		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = 0;
		if (dep.destAccessMask == GraphicsAccessMask::Read || dep.destAccessMask == GraphicsAccessMask::ReadWrite)
		{
			dependency.dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		}
		if (dep.destAccessMask == GraphicsAccessMask::Write || dep.destAccessMask == GraphicsAccessMask::ReadWrite)
		{
			dependency.dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}

		subPassesDependencies.push_back(dependency);
	}

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
	renderPassInfo.pAttachments = attachmentDescriptions.data();
	renderPassInfo.subpassCount = static_cast<uint32_t>(subPasses.size());
	renderPassInfo.pSubpasses = subPasses.data();
	renderPassInfo.dependencyCount = static_cast<uint32_t>(subPassesDependencies.size());
	renderPassInfo.pDependencies = subPassesDependencies.data();

	CheckVkResultReturnOnFail(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass));

	return true;
}