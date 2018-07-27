#pragma once
#include "Pch.h"

#include "Engine/Types/Array.h"

#include <vulkan/vulkan.h>
#include <SDL_vulkan.h>

class Logger;

struct VulkanExtensionInfo
{
private:
	std::shared_ptr<Logger> m_logger;
	bool m_populatedFromDevice;

public:
	Array<VkExtensionProperties> Extensions;
	Array<VkLayerProperties> ValidationLayers;

public:
	VulkanExtensionInfo(std::shared_ptr<Logger> logger);

	bool Populate();
	bool Populate(VkPhysicalDevice device);

	void Print();

	void RemoveUnavailableLayers(Array<const char*>& input) const;
	void RemoveUnavailableExtensions(Array<const char*>& input) const;

	bool IsExtensionAvailable(const char* name) const;
	bool IsValidationLayerAvailable(const char* name) const;

};