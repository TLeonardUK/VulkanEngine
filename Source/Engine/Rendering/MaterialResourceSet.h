#pragma once
#include "Pch.h"

#include "Engine/Resources/Types/Shader.h"

#include "Engine/Graphics/GraphicsEnums.h"
#include "Engine/Graphics/GraphicsResourceSetPool.h"

#include "Engine/Rendering/RendererEnums.h"

class Logger;
class IGraphics;

struct MaterialResourceSet
{
public:
	String name;
	GraphicsResourceSetDescription description;
	std::shared_ptr<IGraphicsResourceSet> set;
	Array<ShaderBinding> bindings;
	GraphicsBindingFrequency frequency;
	size_t hashCode;

public:
	void CalculateHashCode();

	void UpdateBindings(
		const std::shared_ptr<Logger>& logger,
		const std::shared_ptr<IGraphics>& graphics,
		RenderPropertyCollection* collection,
		const std::shared_ptr<IGraphicsResourceSet>& updateSet
	);

};
