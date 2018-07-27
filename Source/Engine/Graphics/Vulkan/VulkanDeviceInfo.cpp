#include "Pch.h"

#include "Engine/Graphics/Vulkan/VulkanDeviceInfo.h"
#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Engine/Logging.h"

VulkanDeviceInfo::VulkanDeviceInfo(std::shared_ptr<Logger> logger)
	: Extensions(logger)
	, m_logger(logger)
{
}

bool VulkanDeviceInfo::Populate(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	Device = device;

	// Get general device properties.
	vkGetPhysicalDeviceProperties(device, &Properties);
	vkGetPhysicalDeviceFeatures(device, &Features);
	vkGetPhysicalDeviceMemoryProperties(device, &Memory);

	// Get supported device queues.
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	QueueFamilyProperties.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, QueueFamilyProperties.data());

	GraphicsQueueFamilyIndex = -1;
	ComputeQueueFamilyIndex = -1;
	CopyQueueFamilyIndex = -1;
	PresentQueueFamilyIndex = -1;

	for (int i = 0; i < QueueFamilyProperties.size(); i++)
	{
		VkQueueFamilyProperties& properties = QueueFamilyProperties[i];
		if (properties.queueCount > 0)
		{
			if ((properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
			{
				GraphicsQueueFamilyIndex = i;

				VkBool32 presentSupported = false;
				CheckVkResultReturnOnFail(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupported));

				if (presentSupported)
				{
					PresentQueueFamilyIndex = i;
				}
			}
			if ((properties.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0)
			{
				ComputeQueueFamilyIndex = i;
			}
			if ((properties.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0)
			{
				CopyQueueFamilyIndex = i;
			}
		}
	}

	// Get supported surface formats.
	CheckVkResultReturnOnFail(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &SurfaceCapabilities));

	uint32_t formatCount;
	CheckVkResultReturnOnFail(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr));

	if (formatCount != 0) 
	{
		SupportedSurfaceFormats.resize(formatCount);
		CheckVkResultReturnOnFail(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, SupportedSurfaceFormats.data()));
	}

	// Get supported present modes
	uint32_t presentModeCount;
	CheckVkResultReturnOnFail(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr));

	if (presentModeCount != 0)
	{
		SupportedPresentModes.resize(presentModeCount);
		CheckVkResultReturnOnFail(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, SupportedPresentModes.data()));
	}

	if (!Extensions.Populate(device))
	{
		return false;
	}

	return true;
}

bool VulkanDeviceInfo::GetPhysicalDeviceSuitability(int* deviceScore)
{
	// Calculate a preference score for device.
	int score = 0;

	if (Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		score += 100000;
	}
	else if (Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
	{
		score += 50000;
	}

	score += ((int)GetTotalDeviceLocalMemory() / 1024) / 1024;

	*deviceScore = score;

	// Require certain features.
	if (!Features.samplerAnisotropy)
	{
		return false;
	}

	// Check we have all required queues.
	if (GraphicsQueueFamilyIndex < 0 || 
		ComputeQueueFamilyIndex < 0 ||
		CopyQueueFamilyIndex < 0 ||
		PresentQueueFamilyIndex < 0)
	{
		return false;
	}

	// Check we have valid swap chain support.
	if (SupportedSurfaceFormats.empty() ||
		SupportedPresentModes.empty())
	{
		return false;
	}

	return true;
}

VkDeviceSize VulkanDeviceInfo::GetTotalDeviceLocalMemory()
{
	VkDeviceSize size = 0;

	for (uint32_t i = 0; i < Memory.memoryHeapCount; i++)
	{
		if ((Memory.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0)
		{
			size += Memory.memoryHeaps[i].size;
		}
	}

	return size;
}

