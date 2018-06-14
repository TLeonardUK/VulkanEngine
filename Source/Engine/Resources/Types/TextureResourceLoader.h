#pragma once

#include "Engine/Resources/ResourceLoader.h"
#include "Engine/Resources/ResourceManager.h"

class Logger;
class IGraphics;
class Texture;
class Renderer;
struct SamplerDescription;

class TextureResourceLoader
	: public IResourceLoader
{
private:
	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<IGraphics> m_graphics;
	std::shared_ptr<Renderer> m_renderer;

	ResourcePtr<Texture> m_defaultTexture;

public:
	TextureResourceLoader(std::shared_ptr<Logger> logger, std::shared_ptr<IGraphics> graphics, std::shared_ptr<Renderer> renderer);

	virtual String GetTag();

	virtual void LoadDefaults(std::shared_ptr<ResourceManager> manager);

	virtual void AssignDefault(std::shared_ptr<ResourceStatus> resource);

	virtual std::shared_ptr<IResource> Load(std::shared_ptr<ResourceManager> manager, std::shared_ptr<ResourceStatus> resource, json& jsonValue);

	std::shared_ptr<Texture> CreateTextureFromBytes(const String& name, const Array<char>& data, int width, int height, const SamplerDescription& samplerDescription, bool bGenerateMips);

};
