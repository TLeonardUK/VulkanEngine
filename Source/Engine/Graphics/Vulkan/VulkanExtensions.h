#pragma once

class VulkanGraphics;

bool SetVulkanMarkerName(VkDevice device, VkDebugReportObjectTypeEXT objectType, uint64_t object, const String& name);

bool LoadVulkanExtensions(const VulkanGraphics& graphics);