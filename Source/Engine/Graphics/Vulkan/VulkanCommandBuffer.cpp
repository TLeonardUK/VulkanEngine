#include "Pch.h"

#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Graphics/Vulkan/VulkanCommandBuffer.h"
#include "Engine/Graphics/Vulkan/VulkanPipeline.h"
#include "Engine/Graphics/Vulkan/VulkanRenderPass.h"
#include "Engine/Graphics/Vulkan/VulkanFramebuffer.h"
#include "Engine/Graphics/Vulkan/VulkanVertexBuffer.h"
#include "Engine/Graphics/Vulkan/VulkanIndexBuffer.h"
#include "Engine/Graphics/Vulkan/VulkanResourceSet.h"
#include "Engine/Graphics/Vulkan/VulkanImage.h"
#include "Engine/Graphics/Vulkan/VulkanImageView.h"

#include "Engine/Profiling/Profiling.h"
#include "Engine/Utilities/Statistic.h"

#include "Engine/Types/Set.h"

#include "Engine/Engine/Logging.h"

Statistic Stat_Rendering_Vulkan_CommandBufferCount("Rendering/Vulkan/Command Buffer Count", StatisticFrequency::Persistent, StatisticFormat::Integer);

VulkanCommandBuffer::VulkanCommandBuffer(
	VkDevice device,
	std::shared_ptr<Logger> logger,
	const String& name,
	VkCommandBuffer buffer,
	VkCommandPool pool,
	std::shared_ptr<VulkanGraphics> graphics,
	bool bPrimary
)
	: m_device(device)
	, m_logger(logger)
	, m_name(name)
	, m_commandBuffer(buffer)
	, m_commandPool(pool)
	, m_graphics(graphics)
	, m_primary(bPrimary)
	, m_subPassIndex(0)
	, m_activePipeline(nullptr)
	, m_activeRenderPass(nullptr)
	, m_activeFramebuffer(nullptr)
{
	Stat_Rendering_Vulkan_CommandBufferCount.Add(1);
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
	Stat_Rendering_Vulkan_CommandBufferCount.Add(-1);
	FreeResources();
}

void VulkanCommandBuffer::FreeResources()
{
	if (m_commandBuffer != nullptr)
	{
		m_graphics->QueueDisposal([m_device = m_device, m_commandPool = m_commandPool, m_commandBuffer = m_commandBuffer]() {
			vkFreeCommandBuffers(m_device, m_commandPool, 1, &m_commandBuffer);
		});
		m_commandBuffer = nullptr;
	}
}

String VulkanCommandBuffer::GetName()
{
	return m_name;
}

VkCommandBuffer VulkanCommandBuffer::GetCommandBuffer()
{
	return m_commandBuffer;
}

void VulkanCommandBuffer::Reset()
{
	CheckVkResultReturnVoidOnFail(vkResetCommandBuffer(m_commandBuffer, 0));
}

void VulkanCommandBuffer::Begin(const std::shared_ptr<IGraphicsRenderPass>& pass, const std::shared_ptr<IGraphicsFramebuffer>& framebuffer)
{
	VkCommandBufferInheritanceInfo inheritanceInfo = {};
	inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritanceInfo.pNext = nullptr;

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | (m_primary ? 0 : VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT);
	beginInfo.pInheritanceInfo = nullptr;

	if (!m_primary)
	{
		assert(pass != nullptr);
		assert(framebuffer != nullptr);

		VulkanRenderPass* vulkanPass = static_cast<VulkanRenderPass*>(&*pass);
		VulkanFramebuffer* vulkanFramebuffer = static_cast<VulkanFramebuffer*>(&*framebuffer);

		inheritanceInfo.renderPass = vulkanPass->GetRenderPass();
		inheritanceInfo.framebuffer = vulkanFramebuffer->GetFramebuffer();
		inheritanceInfo.subpass = 0;

		beginInfo.pInheritanceInfo = &inheritanceInfo;
	}

	CheckVkResultReturnVoidOnFail(vkBeginCommandBuffer(m_commandBuffer, &beginInfo));
}

void VulkanCommandBuffer::End()
{
	CheckVkResultReturnVoidOnFail(vkEndCommandBuffer(m_commandBuffer));
	m_activePipeline = nullptr;
}

void VulkanCommandBuffer::BeginPass(const std::shared_ptr<IGraphicsRenderPass>& pass, const std::shared_ptr<IGraphicsFramebuffer>& framebuffer, bool bInline)
{
	VulkanRenderPass* vulkanPass = static_cast<VulkanRenderPass*>(&*pass);
	VulkanFramebuffer* vulkanFramebuffer = static_cast<VulkanFramebuffer*>(&*framebuffer);

	// Transition framebuffer images to initial layouts.
	const Array<std::shared_ptr<VulkanImageView>>& attachments = vulkanFramebuffer->GetAttachments();
	for (auto& imageView : attachments)
	{
		VulkanImage* image = static_cast<VulkanImage*>(&*imageView->GetImage());
		TransitionImage(image, image->IsDepth() ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	}

	VkRect2D renderArea = {};
	renderArea.offset = { 0, 0 };
	renderArea.extent = { (uint32_t)vulkanFramebuffer->GetWidth(), (uint32_t)vulkanFramebuffer->GetHeight() };

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = vulkanPass->GetRenderPass();
	renderPassInfo.framebuffer = vulkanFramebuffer->GetFramebuffer();
	renderPassInfo.renderArea = renderArea;

	vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, bInline ? VK_SUBPASS_CONTENTS_INLINE : VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

	m_subPassIndex = 0;	
	m_activeRenderPass = vulkanPass;
	m_activeFramebuffer = vulkanFramebuffer;
}

void VulkanCommandBuffer::EndPass()
{
	vkCmdEndRenderPass(m_commandBuffer);

	if (m_activeRenderPass->GetSettings().transitionToPresentFormat)
	{
		Array<std::shared_ptr<VulkanImageView>> imageViews = m_activeFramebuffer->GetAttachments();
		for (auto& view : imageViews)
		{
			std::shared_ptr<VulkanImage> image = view->GetVkImage();
			if (!image->IsDepth())
			{
				image->SetVkLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
			}
		}
	}

	m_activeRenderPass = nullptr;
	m_activeFramebuffer = nullptr;
}

void VulkanCommandBuffer::BeginSubPass()
{
	if (m_subPassIndex > 0)
	{
		vkCmdNextSubpass(m_commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
	}
	m_subPassIndex++;
}

void VulkanCommandBuffer::EndSubPass()
{
}

void VulkanCommandBuffer::SetViewport(int x, int y, int width, int height)
{
	VkViewport viewport = {};
	viewport.x = (float)x;
	viewport.y = (float)y;
	viewport.width = (float)width;
	viewport.height = (float)height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);
}

void VulkanCommandBuffer::SetScissor(int x, int y, int width, int height)
{
	VkRect2D scissor = {};
	scissor.offset = { x, y };
	scissor.extent = { (uint32_t)width, (uint32_t)height };

	vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
}

void VulkanCommandBuffer::Clear(const std::shared_ptr<IGraphicsImage>& image, Color color, float depth, float stencil)
{
	VulkanImage* vulkanImage = static_cast<VulkanImage*>(&*image);
	
	VkClearColorValue clearColor = { color.r, color.g, color.b, color.a };
	
	VkClearValue clearValue = {};
	clearValue.color = clearColor;
	clearValue.depthStencil.depth = depth;
	clearValue.depthStencil.stencil = static_cast<uint32_t>(stencil);

	VkImageAspectFlags aspectFlags = 0;
	if (vulkanImage->IsDepth())
	{
		aspectFlags |= VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	if (vulkanImage->IsStencil())
	{
		aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	if (!vulkanImage->IsDepth() && !vulkanImage->IsStencil())
	{
		aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	VkImageSubresourceRange imageRange = {};
	imageRange.aspectMask = aspectFlags;
	imageRange.levelCount = 1;
	imageRange.layerCount = 1;

	TransitionImage(vulkanImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	
	if (vulkanImage->IsDepth())
	{
		vkCmdClearDepthStencilImage(
			m_commandBuffer,
			vulkanImage->GetVkImage(),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			&clearValue.depthStencil,
			1,
			&imageRange
		);
	}
	else
	{
		vkCmdClearColorImage(
			m_commandBuffer,
			vulkanImage->GetVkImage(),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			&clearColor,
			1,
			&imageRange
		);
	}
}

void VulkanCommandBuffer::SetPipeline(const std::shared_ptr<IGraphicsPipeline>& pipeline)
{
	VulkanPipeline* vulkanPipeline = static_cast<VulkanPipeline*>(&*pipeline);

	vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->GetPipeline());

	m_activePipeline = vulkanPipeline;
}

void VulkanCommandBuffer::SetVertexBuffer(const std::shared_ptr<IGraphicsVertexBuffer>& buffer)
{
	VulkanVertexBuffer* vulkanBuffer = static_cast<VulkanVertexBuffer*>(&*buffer);

	VkBuffer buffers[] = { vulkanBuffer->GetGpuBuffer().Buffer };
	VkDeviceSize offsets[] = { 0 };

	vkCmdBindVertexBuffers(m_commandBuffer, 0, 1, buffers, offsets);
}

void VulkanCommandBuffer::SetIndexBuffer(const std::shared_ptr<IGraphicsIndexBuffer>& buffer)
{
	VulkanIndexBuffer* vulkanBuffer = static_cast<VulkanIndexBuffer*>(&*buffer);
	vkCmdBindIndexBuffer(
		m_commandBuffer,
		vulkanBuffer->GetGpuBuffer().Buffer,
		0,
		vulkanBuffer->GetIndexSize() == 4 ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
}

void VulkanCommandBuffer::TransitionResourceSets(const std::shared_ptr<IGraphicsResourceSet>* values, int count)
{
	for (int i = 0; i < count; i++)
	{
		const VulkanResourceSet* vulkanBuffer = static_cast<const VulkanResourceSet*>(&*values[i]);
		
		const Array<VulkanResourceSetBinding>& bindings = vulkanBuffer->GetBindings();
		for (const VulkanResourceSetBinding& binding : bindings)
		{
			if (binding.type == VulkanResourceSetBindingType::Sampler)
			{
				VulkanImage* vkImage = &*binding.samplerImageView->GetVkImage();
				TransitionImage(vkImage, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
		}
	}
}

void VulkanCommandBuffer::TransitionResourceSets(const Array<std::shared_ptr<IGraphicsResourceSet>*>& values)
{
	for (int i = 0; i < values.size(); i++)
	{
		const VulkanResourceSet* vulkanBuffer = static_cast<const VulkanResourceSet*>(&**values[i]);

		const Array<VulkanResourceSetBinding>& bindings = vulkanBuffer->GetBindings();
		for (const VulkanResourceSetBinding& binding : bindings)
		{
			if (binding.type == VulkanResourceSetBindingType::Sampler)
			{
				VulkanImage* image = &*binding.samplerImageView->GetVkImage();
				if (image->m_layout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
				{
					TransitionImage(image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				}
			}
		}
	}
}

void VulkanCommandBuffer::SetResourceSets(const std::shared_ptr<IGraphicsResourceSet>* values, int count)
{
	if (count == 0)
	{
		return;
	}

	assert(m_activePipeline != nullptr);

	int descriptorSetCount = 0;
	int uboOffsetCount = 0;

	for (int i = 0; i < count; i++)
	{
		std::shared_ptr<VulkanResourceSet> vulkanBuffer = std::static_pointer_cast<VulkanResourceSet>(values[i]);
		vulkanBuffer->UpdateResources();
		vulkanBuffer->GetUniformBufferOffsets(m_uniformBufferOffsetBuffer, &uboOffsetCount);
		vulkanBuffer->GetDescriptorSets(m_descriptorSetBuffer, &descriptorSetCount);
	}

	vkCmdBindDescriptorSets(
		m_commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_activePipeline->GetPipelineLayout(),
		0,
		descriptorSetCount,
		m_descriptorSetBuffer,
		uboOffsetCount,
		m_uniformBufferOffsetBuffer
	);
}

void VulkanCommandBuffer::DrawElements(int vertexCount, int instanceCount, int vertexOffset, int instanceOffset)
{
	vkCmdDraw(m_commandBuffer, vertexCount, instanceCount, vertexOffset, instanceOffset);
}

void VulkanCommandBuffer::DrawIndexedElements(int indexCount, int instanceCount, int indexOffset, int vertexOffset, int instanceOffset)
{
	vkCmdDrawIndexed(m_commandBuffer, indexCount, instanceCount, indexOffset, vertexOffset, instanceOffset);
}

void VulkanCommandBuffer::Dispatch(const std::shared_ptr<IGraphicsCommandBuffer>& buffer)
{
	VulkanCommandBuffer* vulkanBuffer = static_cast<VulkanCommandBuffer*>(&*buffer);
	vkCmdExecuteCommands(m_commandBuffer, 1, &vulkanBuffer->m_commandBuffer);
}

void VulkanCommandBuffer::Upload(const std::shared_ptr<IGraphicsVertexBuffer>& buffer)
{
	VulkanVertexBuffer* vulkanBuffer = static_cast<VulkanVertexBuffer*>(&*buffer);
	UploadStagingBuffers(vulkanBuffer->ConsumeStagingBuffers());
}

void VulkanCommandBuffer::Upload(const std::shared_ptr<IGraphicsIndexBuffer>& buffer)
{
	VulkanIndexBuffer* vulkanBuffer = static_cast<VulkanIndexBuffer*>(&*buffer);
	UploadStagingBuffers(vulkanBuffer->ConsumeStagingBuffers());
}

void VulkanCommandBuffer::UploadStagingBuffers(Array<VulkanStagingBuffer> buffers)
{		
	for (int i = 0; i < buffers.size(); i++)
	{
		VulkanStagingBuffer& stagingBuffer = buffers[i];

		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = stagingBuffer.SourceOffset;
		copyRegion.dstOffset = stagingBuffer.DestinationOffset;
		copyRegion.size = stagingBuffer.Length;

		vkCmdCopyBuffer(m_commandBuffer, stagingBuffer.Source.Buffer, stagingBuffer.Destination.Buffer, 1, &copyRegion);

		m_graphics->ReleaseStagingBuffer(stagingBuffer);
	}
}

void VulkanCommandBuffer::TransitionImage(VulkanImage* image, VkImageLayout dstLayout)
{
	while (image->m_layout != dstLayout)
	{
		VkImageLayout srcLayout = image->m_layout;

		if (image->m_layout.compare_exchange_strong(srcLayout, dstLayout))
		{
			TransitionImage(
				image->m_image,
				image->m_mipLevels,
				srcLayout,
				dstLayout,
				image->m_format,
				image->m_layers);
		}
	}
}

void VulkanCommandBuffer::TransitionImage(VkImage image, int mipLevels, VkImageLayout srcLayout, VkImageLayout dstLayout, VkFormat format, int layerCount)
{
	if (srcLayout == dstLayout)
	{
		return;
	}

	VkImageMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = nullptr;
	barrier.oldLayout = srcLayout;
	barrier.newLayout = dstLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.layerCount = layerCount;
	barrier.subresourceRange.baseArrayLayer = 0;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	VkImageAspectFlags aspectFlags = 0;

	bool isDepth = (format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D16_UNORM);
	bool isStencil = (format == VK_FORMAT_D24_UNORM_S8_UINT);

	if (isDepth)
	{
		aspectFlags |= VK_IMAGE_ASPECT_DEPTH_BIT;
	}

	if (isStencil)
	{
		aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	if (!isDepth && !isStencil)
	{
		aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	barrier.subresourceRange.aspectMask = aspectFlags;
	
	if (srcLayout == VK_IMAGE_LAYOUT_UNDEFINED)
	{
		if (dstLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (dstLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else if (dstLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
	}
	else if (srcLayout == VK_IMAGE_LAYOUT_PREINITIALIZED)
	{
		if (dstLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (dstLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else if (dstLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
	}
	else if (srcLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		if (dstLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (dstLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else if (dstLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
	}
	else if (srcLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		if (dstLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
	}
	else if (srcLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
	{
		if (dstLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else if (dstLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
	}
	else if (srcLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		if (dstLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
	}
	else if (srcLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		if (dstLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
	}
	else 
	{
		assert(false);
	}

	vkCmdPipelineBarrier(
		m_commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
}

void VulkanCommandBuffer::Upload(const std::shared_ptr<IGraphicsImage>& buffer)
{
	VulkanImage* vulkanImage = static_cast<VulkanImage*>(&*buffer);

	int mipLevels = vulkanImage->GetMipLevels();
	int layerCount = vulkanImage->GetLayers();

	VkImage image = vulkanImage->GetVkImage();

	Array<VulkanStagingBuffer> buffers = vulkanImage->ConsumeStagingBuffers();
	if (buffers.size() == 0)
	{
		return;
	}

	VulkanStagingBuffer& stagingBuffer = buffers[0];

	// Copy base data into image.
	Array<VkBufferImageCopy> regions;

	int offset = 0;
	for (int i = 0; i < layerCount; i++)
	{
		VkBufferImageCopy region = {};
		region.bufferOffset = stagingBuffer.SourceOffset + offset;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = i;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = vulkanImage->GetVkExtents();

		regions.push_back(region);

		offset += vulkanImage->GetLayerSize();
	}

	TransitionImage(vulkanImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	//TransitionImage(image, vulkanImage->GetVkFormat(), mipLevels, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	vkCmdCopyBufferToImage(
		m_commandBuffer,
		stagingBuffer.Source.Buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		static_cast<uint32_t>(regions.size()),
		regions.data()
	);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = buffer->GetLayers();
	barrier.subresourceRange.levelCount = 1;

	// Blit down to each mip level.
	int32_t mipWidth = vulkanImage->GetVkExtents().width;
	int32_t mipHeight = vulkanImage->GetVkExtents().height;

	// Generate the mip chain.
	for (int i = 1; i < mipLevels; i++)
	{
		// Transition previous mip level to src-optimal.
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(
			m_commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrier);

		// Blit to this level.
		for (int layer = 0; layer < layerCount; layer++)
		{
			VkImageBlit blit = {};

			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = layer;
			blit.srcSubresource.layerCount = 1;

			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth / 2, mipHeight / 2, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = layer;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(
				m_commandBuffer,
				image,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&blit,
				VK_FILTER_LINEAR);
		}

		// Transition level to optimal layout for reading.
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(
			m_commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrier);

		if (mipWidth > 1)
		{
			mipWidth /= 2;
		}
		if (mipHeight > 1)
		{
			mipHeight /= 2;
		}
	}

	// Transition top level mip to optimal layout for reading.
	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(
		m_commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, 
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 
		0,
		0, 
		nullptr,
		0, 
		nullptr,
		1, 
		&barrier);

	vulkanImage->SetVkLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	// Release staging buffers.
	m_graphics->ReleaseStagingBuffer(stagingBuffer);
}