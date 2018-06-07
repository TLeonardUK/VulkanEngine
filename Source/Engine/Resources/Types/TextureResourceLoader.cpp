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
	if (jsonValue.count("imagePath") == 0)
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Texture definition does not include required paramter 'imagePath'", resource->Path.c_str());
		return nullptr;
	}

	String imagePath = jsonValue["imagePath"];

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

	return std::make_shared<Texture>(image, imageView);
}