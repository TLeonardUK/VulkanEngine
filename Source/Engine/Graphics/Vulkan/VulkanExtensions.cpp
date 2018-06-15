#include "Engine/Graphics/Vulkan/VulkanGraphics.h"

#include <SDL_vulkan.h>

PFN_vkCreateDebugReportCallbackEXT fpCreateDebugReportCallbackEXT = nullptr;
VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugReportCallbackEXT(
	VkInstance instance,
	const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugReportCallbackEXT* pCallback
) {
	if (fpCreateDebugReportCallbackEXT != nullptr)
	{
		return fpCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
	}
	else
	{
		return VK_ERROR_FEATURE_NOT_PRESENT;
	}
}

PFN_vkDestroyDebugReportCallbackEXT fpDestroyDebugReportCallbackEXT = nullptr;
VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(
	VkInstance instance,
	VkDebugReportCallbackEXT callback,
	const VkAllocationCallbacks* pAllocator
) {
	if (fpDestroyDebugReportCallbackEXT != nullptr)
	{
		fpDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
	}
}

PFN_vkDebugMarkerSetObjectNameEXT fpDebugMarkerSetObjectNameEXT = nullptr;
VKAPI_ATTR VkResult VKAPI_CALL vkDebugMarkerSetObjectNameEXT(
	VkDevice device,
	const VkDebugMarkerObjectNameInfoEXT* pNameInfo
) {
	if (fpDebugMarkerSetObjectNameEXT != nullptr)
	{
		return fpDebugMarkerSetObjectNameEXT(device, pNameInfo);
	}
	else
	{
		return VK_ERROR_FEATURE_NOT_PRESENT;
	}
}

bool LoadVulkanExtensions(const VulkanGraphics& graphics)
{
	if (graphics.GetExtensionInfo().IsExtensionAvailable(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
	{
		fpCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(graphics.GetInstance(), "vkCreateDebugReportCallbackEXT"));
		fpDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(graphics.GetInstance(), "vkDestroyDebugReportCallbackEXT"));
	}

	if (graphics.GetExtensionInfo().IsExtensionAvailable(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
	{
		fpDebugMarkerSetObjectNameEXT = reinterpret_cast<PFN_vkDebugMarkerSetObjectNameEXT>(vkGetInstanceProcAddr(graphics.GetInstance(), "vkDebugMarkerSetObjectNameEXT"));
	}

	return true;
}

bool SetVulkanMarkerName(VkDevice device, VkDebugReportObjectTypeEXT objectType, uint64_t object, const String& name)
{
	VkDebugMarkerObjectNameInfoEXT nameInfo = {};
	nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
	nameInfo.object = object;
	nameInfo.objectType = objectType;
	nameInfo.pObjectName = name.c_str();
	vkDebugMarkerSetObjectNameEXT(device, &nameInfo);

	return true;
}
