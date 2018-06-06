#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Engine/Logging.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/GraphicsCommandBufferPool.h"
#include "Engine/Graphics/GraphicsCommandBuffer.h"

#include "Engine/Graphics/Vulkan/VulkanDeviceInfo.h"

#include <vulkan/vulkan.h>

class VulkanCommandBuffer;

class VulkanCommandBufferPool : public IGraphicsCommandBufferPool
{
private:
	String m_name;
	std::shared_ptr<Logger> m_logger;

	Array<std::shared_ptr<VulkanCommandBuffer>> m_allocatedBuffers;

	VkDevice m_device;
	VkCommandPool m_commandBufferPool;

	VulkanDeviceInfo m_deviceInfo;

private:
	friend class VulkanGraphics;
	friend class VulkanImageView;

	void FreeResources();
	bool Build();

	VkCommandPool GetCommandBufferPool();

public:
	VulkanCommandBufferPool(
		VkDevice device,
		const VulkanDeviceInfo& deviceInfo,
		std::shared_ptr<Logger> logger,
		const String& name);

	virtual ~VulkanCommandBufferPool();

	virtual std::shared_ptr<IGraphicsCommandBuffer> Allocate();

};