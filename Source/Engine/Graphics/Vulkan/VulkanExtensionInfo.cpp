#include "Engine/Graphics/Vulkan/VulkanExtensionInfo.h"
#include "Engine/Graphics/Vulkan/VulkanGraphics.h"
#include "Engine/Engine/Logging.h"

VulkanExtensionInfo::VulkanExtensionInfo(std::shared_ptr<Logger> logger)
	: m_logger(logger)
	, m_populatedFromDevice(false)
{
}

bool VulkanExtensionInfo::Populate()
{
	m_populatedFromDevice = false;

	uint32_t extensionCount = 0;
	CheckVkResultReturnOnFail(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));

	Extensions.resize(extensionCount);
	CheckVkResultReturnOnFail(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, Extensions.data()));

	uint32_t layerCount = 0;
	CheckVkResultReturnOnFail(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));

	ValidationLayers.resize(layerCount);
	CheckVkResultReturnOnFail(vkEnumerateInstanceLayerProperties(&layerCount, ValidationLayers.data()));

	return true;
}

bool VulkanExtensionInfo::Populate(VkPhysicalDevice device)
{
	m_populatedFromDevice = true;

	uint32_t extensionCount = 0;
	CheckVkResultReturnOnFail(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr));

	Extensions.resize(extensionCount);
	CheckVkResultReturnOnFail(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, Extensions.data()));

	uint32_t layerCount = 0;
	CheckVkResultReturnOnFail(vkEnumerateDeviceLayerProperties(device, &layerCount, nullptr));

	ValidationLayers.resize(layerCount);
	CheckVkResultReturnOnFail(vkEnumerateDeviceLayerProperties(device, &layerCount, ValidationLayers.data()));

	return true;
}

void VulkanExtensionInfo::Print()
{
	m_logger->WriteInfo(LogCategory::Vulkan, "Available %s Extensions:", (m_populatedFromDevice ? "Device" : "Instance"));
	for (const VkExtensionProperties& extension : Extensions)
	{
		m_logger->WriteInfo(LogCategory::Vulkan, "\t%s (version %i)", extension.extensionName, extension.specVersion);
	}

	m_logger->WriteInfo(LogCategory::Vulkan, "Available %s Validation Layers:", (m_populatedFromDevice ? "Device" : "Instance"));
	for (const VkLayerProperties& layer : ValidationLayers)
	{
		m_logger->WriteInfo(LogCategory::Vulkan, "\t%s - %s (version %i)", layer.layerName, layer.description, layer.specVersion);
	}
}

void VulkanExtensionInfo::RemoveUnavailableLayers(Array<const char*>& input) const
{
	for (auto iter = input.begin(); iter != input.end(); iter++)
	{
		const char* layer = *iter;
		if (!IsValidationLayerAvailable(layer))
		{
			iter = input.erase(iter, iter);
		}
	}
}

void VulkanExtensionInfo::RemoveUnavailableExtensions(Array<const char*>& input) const
{
	for (auto iter = input.begin(); iter != input.end(); iter++)
	{
		const char* layer = *iter;
		if (!IsExtensionAvailable(layer))
		{
			iter = input.erase(iter, iter);
		}
	}
}

bool VulkanExtensionInfo::IsExtensionAvailable(const char* name) const
{
	for (VkExtensionProperties extProperties : Extensions)
	{
		if (strcmp(extProperties.extensionName, name) == 0)
		{
			return true;
		}
	}
	return false;
}

bool VulkanExtensionInfo::IsValidationLayerAvailable(const char* name) const
{
	for (VkLayerProperties layerProperties : ValidationLayers)
	{
		if (strcmp(layerProperties.layerName, name) == 0)
		{
			return true;
		}
	}
	return false;
}