#pragma once

#include "Engine/Types/Array.h"

#include <vulkan/vulkan.h>
#include <SDL_vulkan.h>

#include "Engine/Graphics/Vulkan/VulkanExtensionInfo.h"

class Logger;

struct VulkanDeviceInfo
{
private:
	std::shared_ptr<Logger> m_logger;

public:
	VkPhysicalDevice Device;

	VkPhysicalDeviceProperties Properties;
	VkPhysicalDeviceFeatures Features;
	VkPhysicalDeviceMemoryProperties Memory;
	VkSurfaceCapabilitiesKHR SurfaceCapabilities;

	Array<VkSurfaceFormatKHR> SupportedSurfaceFormats;
	Array<VkPresentModeKHR> SupportedPresentModes;

	Array<VkQueueFamilyProperties> QueueFamilyProperties;

	VulkanExtensionInfo Extensions;

	int CopyQueueFamilyIndex;
	int GraphicsQueueFamilyIndex;
	int ComputeQueueFamilyIndex;
	int PresentQueueFamilyIndex;

	VulkanDeviceInfo(std::shared_ptr<Logger> logger);

	bool Populate(VkPhysicalDevice device, VkSurfaceKHR surface);

	bool GetPhysicalDeviceSuitability(int* deviceScore);

	VkDeviceSize GetTotalDeviceLocalMemory();

};