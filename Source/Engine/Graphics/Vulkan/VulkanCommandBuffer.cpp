#pragma once

#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Graphics/Vulkan/VulkanCommandBuffer.h"
#include "Engine/Graphics/Vulkan/VulkanPipeline.h"
#include "Engine/Graphics/Vulkan/VulkanRenderPass.h"
#include "Engine/Graphics/Vulkan/VulkanFramebuffer.h"
#include "Engine/Graphics/Vulkan/VulkanVertexBuffer.h"
#include "Engine/Graphics/Vulkan/VulkanIndexBuffer.h"
#include "Engine/Graphics/Vulkan/VulkanResourceSet.h"
#include "Engine/Graphics/Vulkan/VulkanImage.h"

#include "Engine/Engine/Logging.h"

#include <cassert>

VulkanCommandBuffer::VulkanCommandBuffer(
	VkDevice device,
	std::shared_ptr<Logger> logger,
	const String& name,
	VkCommandBuffer buffer,
	VkCommandPool pool
)
	: m_device(device)
	, m_logger(logger)
	, m_name(name)
	, m_commandBuffer(buffer)
	, m_commandPool(pool)
{
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
	FreeResources();
}

void VulkanCommandBuffer::FreeResources()
{
	if (m_commandBuffer != nullptr)
	{
		vkFreeCommandBuffers(m_device, m_commandPool, 1, &m_commandBuffer);
		m_commandBuffer = nullptr;
	}
}

VkCommandBuffer VulkanCommandBuffer::GetCommandBuffer()
{
	return m_commandBuffer;
}

void VulkanCommandBuffer::Reset()
{
	CheckVkResultReturnVoidOnFail(vkResetCommandBuffer(m_commandBuffer, 0));
}

void VulkanCommandBuffer::Begin()
{
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	CheckVkResultReturnVoidOnFail(vkBeginCommandBuffer(m_commandBuffer, &beginInfo));
}

void VulkanCommandBuffer::End()
{
	CheckVkResultReturnVoidOnFail(vkEndCommandBuffer(m_commandBuffer));
	m_activePipeline = nullptr;
}

void VulkanCommandBuffer::BeginPass(std::shared_ptr<IGraphicsRenderPass> pass, std::shared_ptr<IGraphicsFramebuffer> framebuffer)
{
	std::shared_ptr<VulkanRenderPass> vulkanPass = std::dynamic_pointer_cast<VulkanRenderPass>(pass);
	std::shared_ptr<VulkanFramebuffer> vulkanFramebuffer = std::dynamic_pointer_cast<VulkanFramebuffer>(framebuffer);

	VkRect2D renderArea = {};
	renderArea.offset = { 0, 0 };
	renderArea.extent = { (uint32_t)vulkanFramebuffer->GetWidth(), (uint32_t)vulkanFramebuffer->GetHeight() };

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = vulkanPass->GetRenderPass();
	renderPassInfo.framebuffer = vulkanFramebuffer->GetFramebuffer();
	renderPassInfo.renderArea = renderArea;

	vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	m_subPassIndex = 0;
}

void VulkanCommandBuffer::EndPass()
{
	vkCmdEndRenderPass(m_commandBuffer);
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

void VulkanCommandBuffer::Clear(std::shared_ptr<IGraphicsImage> image, Color color, float depth, float stencil)
{
	std::shared_ptr<VulkanImage> vulkanImage = std::dynamic_pointer_cast<VulkanImage>(image);
	
	VkClearColorValue clearColor = { color.R, color.G, color.B, color.A };
	
	VkClearValue clearValue = {};
	clearValue.color = clearColor;
	clearValue.depthStencil.depth = depth;
	clearValue.depthStencil.stencil = stencil;

	VkImageSubresourceRange imageRange = {};
	imageRange.aspectMask = vulkanImage->IsDepth() ? (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) : VK_IMAGE_ASPECT_COLOR_BIT;
	imageRange.levelCount = 1;
	imageRange.layerCount = 1;

	if (vulkanImage->IsDepth())
	{
		vkCmdClearDepthStencilImage(
			m_commandBuffer,
			vulkanImage->GetImage(),
			VK_IMAGE_LAYOUT_GENERAL,
			&clearValue.depthStencil,
			1,
			&imageRange
		);
	}
	else
	{
		vkCmdClearColorImage(
			m_commandBuffer,
			vulkanImage->GetImage(),
			VK_IMAGE_LAYOUT_GENERAL,
			&clearColor,
			1,
			&imageRange
		);
	}
}

void VulkanCommandBuffer::SetPipeline(std::shared_ptr<IGraphicsPipeline> pipeline)
{
	std::shared_ptr<VulkanPipeline> vulkanPipeline = std::dynamic_pointer_cast<VulkanPipeline>(pipeline);

	vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipeline->GetPipeline());

	m_activePipeline = vulkanPipeline;
}

void VulkanCommandBuffer::SetVertexBuffer(std::shared_ptr<IGraphicsVertexBuffer> buffer)
{
	std::shared_ptr<VulkanVertexBuffer> vulkanBuffer = std::dynamic_pointer_cast<VulkanVertexBuffer>(buffer);

	VkBuffer buffers[] = { vulkanBuffer->GetGpuBuffer().Buffer };
	VkDeviceSize offsets[] = { 0 };

	vkCmdBindVertexBuffers(m_commandBuffer, 0, 1, buffers, offsets);
}

void VulkanCommandBuffer::SetIndexBuffer(std::shared_ptr<IGraphicsIndexBuffer> buffer)
{
	std::shared_ptr<VulkanIndexBuffer> vulkanBuffer = std::dynamic_pointer_cast<VulkanIndexBuffer>(buffer);
	vkCmdBindIndexBuffer(
		m_commandBuffer,
		vulkanBuffer->GetGpuBuffer().Buffer,
		0,
		vulkanBuffer->GetIndexSize() == 4 ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
}

void VulkanCommandBuffer::SetResourceSets(Array<std::shared_ptr<IGraphicsResourceSet>> resourceSets)
{
	assert(m_activePipeline != nullptr);

	Array<VkDescriptorSet> sets(resourceSets.size());
	for (int i = 0; i < resourceSets.size(); i++)
	{
		std::shared_ptr<VulkanResourceSet> vulkanBuffer = std::dynamic_pointer_cast<VulkanResourceSet>(resourceSets[i]);
		sets[i] = vulkanBuffer->GetSet();
	}

	vkCmdBindDescriptorSets(
		m_commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_activePipeline->GetPipelineLayout(),
		0,
		sets.size(),
		sets.data(),
		0,
		nullptr
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

void VulkanCommandBuffer::Upload(std::shared_ptr<IGraphicsVertexBuffer> buffer)
{
	std::shared_ptr<VulkanVertexBuffer> vulkanBuffer = std::dynamic_pointer_cast<VulkanVertexBuffer>(buffer);

	VulkanAllocation gpuBuffer = vulkanBuffer->GetGpuBuffer();
	VulkanAllocation stagingBuffer = vulkanBuffer->GetStagingBuffer();
	
	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = vulkanBuffer->GetDataSize();

	vkCmdCopyBuffer(m_commandBuffer, stagingBuffer.Buffer, gpuBuffer.Buffer, 1, &copyRegion);
}

void VulkanCommandBuffer::Upload(std::shared_ptr<IGraphicsIndexBuffer> buffer)
{
	std::shared_ptr<VulkanIndexBuffer> vulkanBuffer = std::dynamic_pointer_cast<VulkanIndexBuffer>(buffer);

	VulkanAllocation gpuBuffer = vulkanBuffer->GetGpuBuffer();
	VulkanAllocation stagingBuffer = vulkanBuffer->GetStagingBuffer();

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = vulkanBuffer->GetDataSize();

	vkCmdCopyBuffer(m_commandBuffer, stagingBuffer.Buffer, gpuBuffer.Buffer, 1, &copyRegion);
}

void VulkanCommandBuffer::TransitionImage(VkImage image, VkFormat format, VkImageLayout srcLayout, VkImageLayout dstLayout)
{
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = srcLayout;
	barrier.newLayout = dstLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (dstLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	else 
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	if (srcLayout == VK_IMAGE_LAYOUT_UNDEFINED && dstLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (srcLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && dstLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (srcLayout == VK_IMAGE_LAYOUT_UNDEFINED && dstLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
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

void VulkanCommandBuffer::Upload(std::shared_ptr<IGraphicsImage> buffer)
{
	std::shared_ptr<VulkanImage> vulkanImage = std::dynamic_pointer_cast<VulkanImage>(buffer);

	VkImage image = vulkanImage->GetImage();
	VkBuffer stagingBuffer = vulkanImage->GetStagingBuffer();

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = vulkanImage->GetExtents();

	TransitionImage(image, vulkanImage->GetFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	vkCmdCopyBufferToImage(
		m_commandBuffer,
		stagingBuffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	TransitionImage(image, vulkanImage->GetFormat(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}