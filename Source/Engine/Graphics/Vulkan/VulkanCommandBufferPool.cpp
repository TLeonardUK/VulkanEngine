#pragma once

#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Graphics/Vulkan/VulkanCommandBufferPool.h"
#include "Engine/Graphics/Vulkan/VulkanCommandBuffer.h"

#include "Engine/Engine/Logging.h"

#include <cassert>

VulkanCommandBufferPool::VulkanCommandBufferPool(
	VkDevice device,
	const VulkanDeviceInfo& deviceInfo,
	std::shared_ptr<Logger> logger,
	const String& name
)
	: m_device(device)
	, m_deviceInfo(deviceInfo)
	, m_logger(logger)
	, m_name(name)
{
}

VulkanCommandBufferPool::~VulkanCommandBufferPool()
{
	FreeResources();
}

void VulkanCommandBufferPool::FreeResources()
{
	// Free all buffers allocated by us.	
	// todo: this doesn't seem like the way to deal with this ...
	for (std::shared_ptr<VulkanCommandBuffer>& buffer : m_allocatedBuffers)
	{
		buffer->FreeResources();
	}
	m_allocatedBuffers.clear();

	if (m_commandBufferPool != nullptr)
	{
		vkDestroyCommandPool(m_device, m_commandBufferPool, nullptr);
		m_commandBufferPool = nullptr;
	}
}

VkCommandPool VulkanCommandBufferPool::GetCommandBufferPool()
{
	return m_commandBufferPool;
}

bool VulkanCommandBufferPool::Build()
{
	m_logger->WriteInfo(LogCategory::Vulkan, "Builiding new command buffer pool: %s", m_name.c_str());

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = m_deviceInfo.GraphicsQueueFamilyIndex;
	poolInfo.flags = 0;

	CheckVkResultReturnOnFail(vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandBufferPool));

	return true;
}

std::shared_ptr<IGraphicsCommandBuffer> VulkanCommandBufferPool::Allocate()
{
	VkCommandBuffer buffer;

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_commandBufferPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	CheckVkResultReturnValueOnFail(vkAllocateCommandBuffers(m_device, &allocInfo, &buffer), nullptr);

	std::shared_ptr<VulkanCommandBuffer> output = std::make_shared<VulkanCommandBuffer>(m_device, m_logger, String::Format("%s Buffer", m_name.c_str()), buffer, m_commandBufferPool);
	m_allocatedBuffers.push_back(output);

	return output;
}