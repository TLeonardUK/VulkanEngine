#pragma once

#include "Engine/Resources/ResourceLoader.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Resources/Types/Model.h"

class Logger;
class IGraphics;
class Shader;
class IGraphicsIndexBuffer;
class IGraphicsVertexBuffer;

class ModelResourceLoader
	: public IResourceLoader
{
private:
	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<IGraphics> m_graphics;
	std::shared_ptr<Renderer> m_renderer;

	ResourcePtr<Model> m_defaultModel;

public:
	ModelResourceLoader(std::shared_ptr<Logger> logger, std::shared_ptr<IGraphics> graphics, std::shared_ptr<Renderer> renderer);

	virtual String GetTag();

	virtual void LoadDefaults(std::shared_ptr<ResourceManager> manager);

	virtual void AssignDefault(std::shared_ptr<ResourceStatus> resource);

	virtual std::shared_ptr<IResource> Load(std::shared_ptr<ResourceManager> manager, std::shared_ptr<ResourceStatus> resource, json& jsonValue);

};
