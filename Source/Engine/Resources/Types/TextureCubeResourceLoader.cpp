#include "Pch.h"

#include "Engine/Resources/Types/TextureCubeResourceLoader.h"
#include "Engine/Resources/Types/TextureCube.h"
#include "Engine/Resources/Resource.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Rendering/Renderer.h"
#include "Engine/Engine/Logging.h"
#include "Engine/Graphics/Graphics.h"

TextureCubeResourceLoader::TextureCubeResourceLoader(std::shared_ptr<Logger> logger, std::shared_ptr<IGraphics> graphics, std::shared_ptr<Renderer> renderer)
	: TextureResourceLoader(logger, graphics, renderer)
{
}

String TextureCubeResourceLoader::GetTag()
{
	return TextureCube::Tag;
}

void TextureCubeResourceLoader::LoadDefaults(std::shared_ptr<ResourceManager> manager)
{
	m_defaultCubeTexture = manager->Load<Texture>("Engine/Textures/default_cube.json");
}

void TextureCubeResourceLoader::AssignDefault(std::shared_ptr<ResourceStatus> resource)
{
	resource->DefaultResource = m_defaultCubeTexture.Get();
}

std::shared_ptr<IResource> TextureCubeResourceLoader::Load(std::shared_ptr<ResourceManager> manager, std::shared_ptr<ResourceStatus> resource, json& jsonValue)
{
	Array<String> imagePaths;

	if (jsonValue.count("TopImagePath") == 0)
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Texture definition does not include required paramter 'TopImagePath'", resource->Path.c_str());
		return nullptr;
	}
	if (jsonValue.count("BottomImagePath") == 0)
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Texture definition does not include required paramter 'BottomImagePath'", resource->Path.c_str());
		return nullptr;
	}
	if (jsonValue.count("FrontImagePath") == 0)
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Texture definition does not include required paramter 'FrontImagePath'", resource->Path.c_str());
		return nullptr;
	}
	if (jsonValue.count("BackImagePath") == 0)
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Texture definition does not include required paramter 'BackImagePath'", resource->Path.c_str());
		return nullptr;
	}
	if (jsonValue.count("LeftImagePath") == 0)
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Texture definition does not include required paramter 'LeftImagePath'", resource->Path.c_str());
		return nullptr;
	}
	if (jsonValue.count("RightImagePath") == 0)
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Texture definition does not include required paramter 'RightImagePath'", resource->Path.c_str());
		return nullptr;
	}

	imagePaths.push_back(jsonValue["RightImagePath"].get<std::string>());
	imagePaths.push_back(jsonValue["LeftImagePath"].get<std::string>());
	imagePaths.push_back(jsonValue["TopImagePath"].get<std::string>());
	imagePaths.push_back(jsonValue["BottomImagePath"].get<std::string>());
	imagePaths.push_back(jsonValue["FrontImagePath"].get<std::string>());
	imagePaths.push_back(jsonValue["BackImagePath"].get<std::string>());
	
	std::shared_ptr<IGraphicsSampler> sampler;
	std::shared_ptr<IGraphicsImageView> imageView;
	std::shared_ptr<IGraphicsImage> image;
	if (!LoadInternal(manager, resource, jsonValue, imagePaths, sampler, imageView, image))
	{
		return nullptr;
	}

	std::shared_ptr<TextureCube> texture = std::make_shared<TextureCube>(m_renderer, image, imageView, sampler);
	m_renderer->QueueRenderCommand(RenderCommandStage::Global_PreRender, [=](std::shared_ptr<IGraphicsCommandBuffer> buffer) {
		texture->UpdateResources();
	});

	return texture;
}
