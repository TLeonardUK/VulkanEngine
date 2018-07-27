#pragma once
#include "Pch.h"

#include "Engine/Resources/ResourceLoader.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Resources/Types/Shader.h"

class Logger;
class IGraphics;
class Shader;

class ShaderResourceLoader
	: public IResourceLoader
{
private:
	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<IGraphics> m_graphics;

	ResourcePtr<Shader> m_defaultShader;

private:
	bool LoadBindings(Array<ShaderBinding>& bindings, json& bindingsJson, std::shared_ptr<ResourceStatus> resource);
	bool LoadStageStreams(ShaderStage& stage, const String& stageName, json& bindingsJson, std::shared_ptr<ResourceStatus> resource);

public:
	ShaderResourceLoader(std::shared_ptr<Logger> logger, std::shared_ptr<IGraphics> graphics);

	virtual String GetTag();

	virtual void LoadDefaults(std::shared_ptr<ResourceManager> manager);

	virtual void AssignDefault(std::shared_ptr<ResourceStatus> resource);

	virtual std::shared_ptr<IResource> Load(std::shared_ptr<ResourceManager> manager, std::shared_ptr<ResourceStatus> resource, json& jsonValue);

};
