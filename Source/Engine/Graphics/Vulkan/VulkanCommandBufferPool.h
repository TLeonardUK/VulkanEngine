#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Engine/Logging.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/GraphicsCommandBufferPool.h"
#include "Engine/Graphics/GraphicsCommandBuffer.h"

#include "Engine/Graphics/Vulkan/VulkanDeviceInfo.h"
#include "Engine/Graphics/Vulkan/VulkanResource.h"

#include <vulkan/vulkan.h>

class VulkanGraphics;
class VulkanCommandBuffer;

class VulkanCommandBufferPool 
	: public IGraphicsCommandBufferPool
	, public IVulkanResource
{
private:
	String m_name;
	std::shared_ptr<Logger> m_logger;

	Array<std::shared_ptr<VulkanCommandBuffer>> m_allocatedBuffers;

	VkDevice m_device;
	VkCommandPool m_commandBufferPool;

	std::shared_ptr<VulkanGraphics> m_graphics;

	VulkanDeviceInfo m_deviceInfo;

private:
	friend class VulkanGraphics;
	friend class VulkanImageView;

	bool Build();

	VkCommandPool GetCommandBufferPool();

public:
	VulkanCommandBufferPool(
		VkDevice device,
		const VulkanDeviceInfo& deviceInfo,
		std::shared_ptr<Logger> logger,
		const String& name,
		std::shared_ptr<VulkanGraphics> graphics);

	virtual ~VulkanCommandBufferPool();

	virtual std::shared_ptr<IGraphicsCommandBuffer> Allocate();

	virtual void FreeResources();
	virtual String GetName();

};