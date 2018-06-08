#include "Engine/Resources/Types/TextureResourceLoader.h"
#include "Engine/Resources/Types/Texture.h"
#include "Engine/Resources/Resource.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Engine/Logging.h"
#include "Engine/Graphics/Graphics.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

TextureResourceLoader::TextureResourceLoader(std::shared_ptr<Logger> logger, std::shared_ptr<IGraphics> graphics)
	: m_logger(logger)
	, m_graphics(graphics)
{
}

String TextureResourceLoader::GetTag()
{
	return Texture::Tag;
}

void TextureResourceLoader::LoadDefaults(std::shared_ptr<ResourceManager> manager)
{
	m_defaultTexture = manager->Load<Texture>("Engine/Textures/default.json");
}

void TextureResourceLoader::AssignDefault(std::shared_ptr<ResourceStatus> resource)
{
	resource->DefaultResource = m_defaultTexture.Get();
}

std::shared_ptr<IResource> TextureResourceLoader::Load(std::shared_ptr<ResourceManager> manager, std::shared_ptr<ResourceStatus> resource, json& jsonValue)
{
	if (jsonValue.count("ImagePath") == 0)
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Texture definition does not include required paramter 'ImagePath'", resource->Path.c_str());
		return nullptr;
	}

	String imagePath = jsonValue["ImagePath"];

	Array<char> buffer;
	if (!manager->ReadResourceBytes(imagePath.c_str(), buffer))
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Failed to read path", imagePath.c_str());
		return nullptr;
	}

	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(buffer.data()), buffer.size(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	if (pixels == nullptr)
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Failed to decode image, possibly corrupt or in unsupported format.", imagePath.c_str());
		return nullptr;
	}

	std::shared_ptr<IGraphicsImage> image = m_graphics->CreateImage(imagePath.c_str(), texWidth, texHeight, GraphicsFormat::UNORM_R8G8B8A8, true);
	if (image == nullptr)
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Failed to create graphics image to use for rendering.", imagePath.c_str());
		return nullptr;
	}

	image->Stage(pixels, 0, texWidth * texHeight * 4);

	std::shared_ptr<IGraphicsImageView> imageView = m_graphics->CreateImageView(StringFormat("%s View", imagePath.c_str()), image);
	if (imageView == nullptr)
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Failed to create graphics image view to use for rendering.", imagePath.c_str());
		return nullptr;
	}

	// Load sampler information.
	SamplerDescription description;

	if (jsonValue.count("MagnificationFilter"))
	{
		String value = jsonValue["MagnificationFilter"];
		if (!StringToEnum<GraphicsFilter>(value, description.MagnificationFilter))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised magnification filter.", resource->Path.c_str(), value.c_str());
			return false;
		}
	}
	if (jsonValue.count("MinificationFilter"))
	{
		String value = jsonValue["MinificationFilter"];
		if (!StringToEnum<GraphicsFilter>(value, description.MinificationFilter))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised minification filter.", resource->Path.c_str(), value.c_str());
			return false;
		}
	}
	if (jsonValue.count("AddressModeU"))
	{
		String value = jsonValue["AddressModeU"];
		if (!StringToEnum<GraphicsAddressMode>(value, description.AddressModeU))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised address mode.", resource->Path.c_str(), value.c_str());
			return false;
		}
	}
	if (jsonValue.count("AddressModeV"))
	{
		String value = jsonValue["AddressModeV"];
		if (!StringToEnum<GraphicsAddressMode>(value, description.AddressModeV))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised address mode.", resource->Path.c_str(), value.c_str());
			return false;
		}
	}
	if (jsonValue.count("AddressModeW"))
	{
		String value = jsonValue["AddressModeW"];
		if (!StringToEnum<GraphicsAddressMode>(value, description.AddressModeW))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised address mode.", resource->Path.c_str(), value.c_str());
			return false;
		}
	}
	if (jsonValue.count("MaxAnisotropy"))
	{
		description.MaxAnisotropy = jsonValue["MaxAnisotropy"];
	}
	if (jsonValue.count("MinLod"))
	{
		description.MinLod = jsonValue["MinLod"];
	}
	if (jsonValue.count("MaxLod"))
	{
		description.MaxLod = jsonValue["MaxLod"];
	}
	if (jsonValue.count("MipLodBias"))
	{
		description.MipLodBias = jsonValue["MipLodBias"];
	}
	if (jsonValue.count("MimapMode"))
	{
		String value = jsonValue["MimapMode"];
		if (!StringToEnum<GraphicsMipMapMode>(value, description.MipmapMode))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised mipmap mode mode.", resource->Path.c_str(), value.c_str());
			return false;
		}
	}

	std::shared_ptr<IGraphicsSampler> sampler = m_graphics->CreateSampler(StringFormat("%s Sampler", imagePath.c_str()), description);
	if (sampler == nullptr)
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Failed to create sampler for texture.", resource->Path.c_str());
		return false;
	}

	return std::make_shared<Texture>(image, imageView, sampler);
}