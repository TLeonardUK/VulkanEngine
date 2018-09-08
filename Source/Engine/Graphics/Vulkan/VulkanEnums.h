#pragma once
#include "Pch.h"

#include "Engine/Graphics/GraphicsEnums.h"

#include <vulkan/vulkan.h>

VkFormat GraphicsFormatToVkFormat(GraphicsFormat format);
GraphicsFormat VkFormatToGraphicsFormat(VkFormat format);
VkFormat GraphicsBindingFormatToVkFormat(GraphicsBindingFormat format);
int GraphicsFormatBytesPerPixel(GraphicsFormat format);
VkFilter GraphicsFilterToVkFilter(GraphicsFilter filter);
VkSamplerAddressMode GraphicsAddressModeToVkAddressMode(GraphicsAddressMode addressMode);
VkBorderColor GraphicsBorderColorToVkBorderColor(GraphicsBorderColor color);
VkSamplerMipmapMode GraphicsMipMapModeToVkSamplerMipMapMode(GraphicsMipMapMode mode);