#pragma once
#include "Pch.h"

#include "Engine/Resources/ResourceLoader.h"
#include "Engine/Resources/ResourceManager.h"

#include "Engine/Resources/Types/TextureResourceLoader.h"

class Logger;
class IGraphics;
class Texture;
class Renderer;
struct SamplerDescription;

class TextureCubeResourceLoader
	: public TextureResourceLoader
{
private:	
	ResourcePtr<Texture> m_defaultCubeTexture;

public:
	TextureCubeResourceLoader(std::shared_ptr<Logger> logger, std::shared_ptr<IGraphics> graphics, std::shared_ptr<Renderer> renderer);

	virtual String GetTag();

	virtual void LoadDefaults(std::shared_ptr<ResourceManager> manager);

	virtual void AssignDefault(std::shared_ptr<ResourceStatus> resource);

	virtual std::shared_ptr<IResource> Load(std::shared_ptr<ResourceManager> manager, std::shared_ptr<ResourceStatus> resource, json& jsonValue);

};
