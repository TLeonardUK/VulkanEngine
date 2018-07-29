#include "Pch.h"

#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Graphics/Vulkan/VulkanCommandBufferPool.h"
#include "Engine/Graphics/Vulkan/VulkanCommandBuffer.h"

#include "Engine/Engine/Logging.h"
#include "Engine/Utilities/Statistic.h"

Statistic Stat_Rendering_Vulkan_CommandBufferPoolCount("Rendering/Vulkan/Command Buffer Pool Count", StatisticFrequency::Persistent, StatisticFormat::Integer);

VulkanCommandBufferPool::VulkanCommandBufferPool(
	VkDevice device,
	const VulkanDeviceInfo& deviceInfo,
	std::shared_ptr<Logger> logger,
	const String& name,
	std::shared_ptr<VulkanGraphics> graphics
)
	: m_device(device)
	, m_deviceInfo(deviceInfo)
	, m_logger(logger)
	, m_name(name)
	, m_graphics(graphics)
{
	Stat_Rendering_Vulkan_CommandBufferPoolCount.Add(1);
}

VulkanCommandBufferPool::~VulkanCommandBufferPool()
{
	Stat_Rendering_Vulkan_CommandBufferPoolCount.Add(-1);
	FreeResources();
}

String VulkanCommandBufferPool::GetName()
{
	return m_name;
}

void VulkanCommandBufferPool::FreeResources()
{
	if (m_commandBufferPool != nullptr)
	{
		m_graphics->QueueDisposal([m_device = m_device, m_commandBufferPool = m_commandBufferPool]() {
			vkDestroyCommandPool(m_device, m_commandBufferPool, nullptr);
		});
		m_commandBufferPool = nullptr;
	}
}

VkCommandPool VulkanCommandBufferPool::GetCommandBufferPool()
{
	return m_commandBufferPool;
}

bool VulkanCommandBufferPool::Build()
{
	//m_logger->WriteInfo(LogCategory::Vulkan, "Builiding new command buffer pool: %s", m_name.c_str());

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = m_deviceInfo.GraphicsQueueFamilyIndex;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	CheckVkResultReturnOnFail(vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandBufferPool));

	return true;
}

std::shared_ptr<IGraphicsCommandBuffer> VulkanCommandBufferPool::Allocate(bool bPrimary)
{
	VkCommandBuffer buffer;

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_commandBufferPool;
	allocInfo.level = bPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	allocInfo.commandBufferCount = 1;

	CheckVkResultReturnValueOnFail(vkAllocateCommandBuffers(m_device, &allocInfo, &buffer), nullptr);

	std::shared_ptr<VulkanCommandBuffer> output = std::make_shared<VulkanCommandBuffer>(m_device, m_logger, StringFormat("%s Buffer", m_name.c_str()), buffer, m_commandBufferPool, m_graphics, bPrimary);
	
	return output;
}