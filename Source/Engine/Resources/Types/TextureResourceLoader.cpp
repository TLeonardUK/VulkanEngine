#include "Pch.h"

#include "Engine/Resources/Types/TextureResourceLoader.h"
#include "Engine/Resources/Types/Texture.h"
#include "Engine/Resources/Resource.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Rendering/Renderer.h"
#include "Engine/Engine/Logging.h"
#include "Engine/Graphics/Graphics.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

TextureResourceLoader::TextureResourceLoader(std::shared_ptr<Logger> logger, std::shared_ptr<IGraphics> graphics, std::shared_ptr<Renderer> renderer)
	: m_logger(logger)
	, m_graphics(graphics)
	, m_renderer(renderer)
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

bool TextureResourceLoader::LoadInternal(
	std::shared_ptr<ResourceManager> manager,
	std::shared_ptr<ResourceStatus> resource,
	json& jsonValue,
	Array<String>& imagePaths,
	std::shared_ptr<IGraphicsSampler>& sampler,
	std::shared_ptr<IGraphicsImageView>& imageView,
	std::shared_ptr<IGraphicsImage>& image)
{
	int finalTexWidth = -1;
	int finalTexHeight = -1;
	int finalTexChannels = -1;

	Array<stbi_uc*> buffers;
	buffers.resize(imagePaths.size());

	for (int i = 0; i < imagePaths.size(); i++)
	{
		String& imagePath = imagePaths[i];

		Array<char> data;
		if (!manager->ReadResourceBytes(imagePath.c_str(), data))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] Failed to read path", imagePath.c_str());
			return false;
		}

		int texWidth, texHeight, texChannels;
		buffers[i] = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(data.data()), (int)data.size(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		if (buffers[i] == nullptr)
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] Failed to decode image, possibly corrupt or in unsupported format.", imagePath.c_str());
			return false;
		}

		if (i == 0)
		{
			finalTexWidth = texWidth;
			finalTexHeight = texHeight;
			finalTexChannels = texChannels;
		}
		else if (texWidth != finalTexWidth || texHeight != finalTexHeight || texChannels != finalTexChannels)
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] Images contained in texture definition are not all of the same width, height and format.", resource->Path.c_str());
			return false;
		}
	}

	// Create image.
	image = m_graphics->CreateImage(resource->Path.c_str(), finalTexWidth, finalTexHeight, (int)imagePaths.size(), GraphicsFormat::UNORM_R8G8B8A8, true, GraphicsUsage::None);
	if (image == nullptr)
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Failed to create graphics image to use for rendering.", resource->Path.c_str());
		return false;
	}

	for (int i = 0; i < imagePaths.size(); i++)
	{
		image->Stage(i, buffers[i], 0, finalTexWidth * finalTexHeight * 4);
		stbi_image_free(buffers[i]);
	}

	// Create view of image.
	imageView = m_graphics->CreateImageView(StringFormat("%s View", resource->Path.c_str()), image);
	if (imageView == nullptr)
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Failed to create graphics image view to use for rendering.", resource->Path.c_str());
		return false;
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

	if (description.MaxLod == -1)
	{
		description.MaxLod = (float)image->GetMipLevels();
	}

	sampler = m_graphics->CreateSampler(StringFormat("%s Sampler", resource->Path.c_str()), description);
	if (sampler == nullptr)
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Failed to create sampler for texture.", resource->Path.c_str());
		return false;
	}

	return true;
}

std::shared_ptr<IResource> TextureResourceLoader::Load(std::shared_ptr<ResourceManager> manager, std::shared_ptr<ResourceStatus> resource, json& jsonValue)
{
	Array<String> imagePaths;

	if (jsonValue.count("ImagePath") == 0)
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Texture definition does not include required paramter 'ImagePath'", resource->Path.c_str());
		return nullptr;
	}

	imagePaths.push_back(jsonValue["ImagePath"].get<std::string>());
	
	std::shared_ptr<IGraphicsSampler> sampler;
	std::shared_ptr<IGraphicsImageView> imageView;
	std::shared_ptr<IGraphicsImage> image;
	if (!LoadInternal(manager, resource, jsonValue, imagePaths, sampler, imageView, image))
	{
		return nullptr;
	}
	
	std::shared_ptr<Texture> texture = std::make_shared<Texture>(m_renderer, image, imageView, sampler);
	m_renderer->QueueRenderCommand(RenderCommandStage::PreRender, [=](std::shared_ptr<IGraphicsCommandBuffer> buffer) {
		texture->UpdateResources();
	});

	return texture;
}

std::shared_ptr<Texture> TextureResourceLoader::CreateTextureFromBytes(const String& name, const Array<char>& data, int width, int height, const SamplerDescription& samplerDescription, bool bGenerateMips)
{
	assert(data.size() == (width * height * 4));

	std::shared_ptr<IGraphicsImage> image = m_graphics->CreateImage(name, width, height, 1, GraphicsFormat::UNORM_R8G8B8A8, bGenerateMips, GraphicsUsage::None);
	if (image == nullptr)
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Failed to create graphics image to use for rendering.", name.c_str());
		return nullptr;
	}

	image->Stage(0, (void*)data.data(), 0, width * height * 4);

	std::shared_ptr<IGraphicsImageView> imageView = m_graphics->CreateImageView(StringFormat("%s View", name.c_str()), image);
	if (imageView == nullptr)
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Failed to create graphics image view to use for rendering.", name.c_str());
		return nullptr;
	}

	SamplerDescription description = samplerDescription;
	if (description.MaxLod == -1)
	{
		description.MaxLod = (float)image->GetMipLevels();
	}

	std::shared_ptr<IGraphicsSampler> sampler = m_graphics->CreateSampler(StringFormat("%s Sampler", name.c_str()), description);
	if (sampler == nullptr)
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Failed to create sampler for texture.", name.c_str());
		return false;
	}

	std::shared_ptr<Texture> texture = std::make_shared<Texture>(m_renderer, image, imageView, sampler);

	m_renderer->QueueRenderCommand(RenderCommandStage::PreRender, [=](std::shared_ptr<IGraphicsCommandBuffer> buffer) {
		texture->UpdateResources();
	});

	return texture;
}