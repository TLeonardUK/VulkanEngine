#include "Engine/Graphics/Vulkan/VulkanGraphics.h"

#include <SDL_vulkan.h>

PFN_vkCreateDebugReportCallbackEXT fpCreateDebugReportCallbackEXT = nullptr;
VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugReportCallbackEXT(
	VkInstance instance,
	const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugReportCallbackEXT* pCallback
) {
	return fpCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
}

PFN_vkDestroyDebugReportCallbackEXT fpDestroyDebugReportCallbackEXT = nullptr;
VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(
	VkInstance instance,
	VkDebugReportCallbackEXT callback,
	const VkAllocationCallbacks* pAllocator
) {
	fpDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
}

bool LoadVulkanExtensions(const VulkanGraphics& graphics)
{
	if (graphics.GetExtensionInfo().IsExtensionAvailable(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
	{
		fpCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(graphics.GetInstance(), "vkCreateDebugReportCallbackEXT"));
		fpDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(graphics.GetInstance(), "vkDestroyDebugReportCallbackEXT"));
	}

	return true;
}