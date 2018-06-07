#include "Engine/Graphics/Vulkan/VulkanEnums.h"

#include <cassert>

VkFormat GraphicsFormatToVkFormat(GraphicsFormat format)
{
	switch (format)
	{
	case GraphicsFormat::UNORM_R8:				return VK_FORMAT_R8G8_UNORM;
	case GraphicsFormat::UNORM_R8G8B8:			return VK_FORMAT_R8G8B8_UNORM;
	case GraphicsFormat::UNORM_R8G8B8A8:		return VK_FORMAT_R8G8B8A8_UNORM;
	case GraphicsFormat::UNORM_B8G8R8:			return VK_FORMAT_B8G8R8_UINT;
	case GraphicsFormat::UNORM_B8G8R8A8:		return VK_FORMAT_B8G8R8A8_UNORM;
	case GraphicsFormat::UNORM_R16G16B16:		return VK_FORMAT_R16G16B16_UNORM;
	case GraphicsFormat::UNORM_R16G16B16A16:	return VK_FORMAT_R16G16B16A16_UNORM;
	case GraphicsFormat::UNORM_D24_UINT_S8:		return VK_FORMAT_D24_UNORM_S8_UINT;
	}

	assert(false);
	return VK_FORMAT_UNDEFINED;
}

int GraphicsFormatBytesPerPixel(GraphicsFormat format)
{
	switch (format)
	{
	case GraphicsFormat::UNORM_R8:				return 1;
	case GraphicsFormat::UNORM_R8G8B8:			return 3;
	case GraphicsFormat::UNORM_R8G8B8A8:		return 4;
	case GraphicsFormat::UNORM_B8G8R8:			return 3;
	case GraphicsFormat::UNORM_B8G8R8A8:		return 4;
	case GraphicsFormat::UNORM_R16G16B16:		return 6;
	case GraphicsFormat::UNORM_R16G16B16A16:	return 8;
	case GraphicsFormat::UNORM_D24_UINT_S8:		return 4;
	}

	assert(false);
	return 0;
}

GraphicsFormat VkFormatToGraphicsFormat(VkFormat format)
{
	switch (format)
	{
	case VK_FORMAT_R8G8_UNORM:			return GraphicsFormat::UNORM_R8;
	case VK_FORMAT_R8G8B8_UNORM:		return GraphicsFormat::UNORM_R8G8B8;
	case VK_FORMAT_R8G8B8A8_UNORM:		return GraphicsFormat::UNORM_R8G8B8A8;
	case VK_FORMAT_B8G8R8_UINT:			return GraphicsFormat::UNORM_B8G8R8;
	case VK_FORMAT_B8G8R8A8_UNORM:		return GraphicsFormat::UNORM_B8G8R8A8;
	case VK_FORMAT_R16G16B16_UNORM:		return GraphicsFormat::UNORM_R16G16B16;
	case VK_FORMAT_R16G16B16A16_UNORM:	return GraphicsFormat::UNORM_R16G16B16A16;
	case VK_FORMAT_D24_UNORM_S8_UINT:	return GraphicsFormat::UNORM_D24_UINT_S8;
	}

	assert(false);
	return GraphicsFormat::Undefined;
}

VkFormat GraphicsBindingFormatToVkFormat(GraphicsBindingFormat format)
{
	switch (format)
	{
		case GraphicsBindingFormat::Bool1:		return VK_FORMAT_R8_SINT;
		case GraphicsBindingFormat::Bool2:		return VK_FORMAT_R8G8_SINT;
		case GraphicsBindingFormat::Bool3:		return VK_FORMAT_R8G8B8_SINT;
		case GraphicsBindingFormat::Bool4:		return VK_FORMAT_R8G8B8A8_SINT;
		case GraphicsBindingFormat::Int1:		return VK_FORMAT_R32_SINT;
		case GraphicsBindingFormat::Int2:		return VK_FORMAT_R32G32_SINT;
		case GraphicsBindingFormat::Int3:		return VK_FORMAT_R32G32B32_SINT;
		case GraphicsBindingFormat::Int4:		return VK_FORMAT_R32G32B32A32_SINT;
		case GraphicsBindingFormat::UInt1:		return VK_FORMAT_R32_UINT;
		case GraphicsBindingFormat::UInt2:		return VK_FORMAT_R32G32_UINT;
		case GraphicsBindingFormat::UInt3:		return VK_FORMAT_R32G32B32_UINT;
		case GraphicsBindingFormat::UInt4:		return VK_FORMAT_R32G32B32A32_UINT;
		case GraphicsBindingFormat::Float:		return VK_FORMAT_R32_SFLOAT;
		case GraphicsBindingFormat::Float2:		return VK_FORMAT_R32G32_SFLOAT;
		case GraphicsBindingFormat::Float3:		return VK_FORMAT_R32G32B32_SFLOAT;
		case GraphicsBindingFormat::Float4:		return VK_FORMAT_R32G32B32A32_SFLOAT;
		case GraphicsBindingFormat::Double1:	return VK_FORMAT_R64_SFLOAT;
		case GraphicsBindingFormat::Double2:	return VK_FORMAT_R64G64_SFLOAT;
		case GraphicsBindingFormat::Double3:	return VK_FORMAT_R64G64B64_SFLOAT;
		case GraphicsBindingFormat::Double4:	return VK_FORMAT_R64G64B64A64_SFLOAT;
	}

	assert(false);
	return VK_FORMAT_UNDEFINED;
}

VkFilter GraphicsFilterToVkFilter(GraphicsFilter filter)
{
	switch (filter)
	{
	case GraphicsFilter::Linear:			return VK_FILTER_LINEAR;
	case GraphicsFilter::NearestNeighbour:	return VK_FILTER_NEAREST;
	}

	assert(false);
	return VK_FILTER_MAX_ENUM;
}

VkSamplerAddressMode GraphicsAddressModeToVkAddressMode(GraphicsAddressMode addressMode)
{
	switch (addressMode)
	{
	case GraphicsAddressMode::Repeat:			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	case GraphicsAddressMode::MirroredRepeat:	return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	case GraphicsAddressMode::ClampToEdge:		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	case GraphicsAddressMode::ClampToBorder:	return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	}

	assert(false);
	return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
}

VkSamplerMipmapMode GraphicsMipMapModeToVkSamplerMipMapMode(GraphicsMipMapMode mode)
{
	switch (mode)
	{
	case GraphicsMipMapMode::Linear:			return VK_SAMPLER_MIPMAP_MODE_LINEAR;
	case GraphicsMipMapMode::NearestNeighbour:	return VK_SAMPLER_MIPMAP_MODE_NEAREST;
	}

	assert(false);
	return VK_SAMPLER_MIPMAP_MODE_MAX_ENUM;
}