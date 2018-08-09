#pragma once
#include "Pch.h"

#include "Engine/Resources/Types/Texture.h"
#include "Engine/Resources/Types/Shader.h"
#include "Engine/Resources/Types/MaterialPropertyCollection.h"

#include "Engine/Graphics/GraphicsEnums.h"
#include "Engine/Graphics/GraphicsResourceSetPool.h"

#include "Engine/Rendering/RendererEnums.h"

#include "Engine/Types/Math.h"
#include "Engine/Types/Array.h"
#include "Engine/Utilities/Enum.h"

class Shader;
class Binding;
class IGraphics;
class IGraphicsRenderPass;
class IGraphicsPipeline;
class IGraphicsFramebuffer;
class Logger;
class Renderer;
struct ShaderBindingField;
class Material;

// Gets object-specific data required for rendering this material.
class MaterialRenderData
{
private:
	Array<std::shared_ptr<IGraphicsUniformBuffer>> m_uniformBuffers;
	Array<std::shared_ptr<IGraphicsResourceSet>> m_resourceSets;

	std::shared_ptr<Material> m_lastKnownMaterial = nullptr;
	int m_lastMeshPropertiesVersion = -1;
	int m_lastMaterialPropertiesVersion = -1;

	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<Renderer> m_renderer;
	std::shared_ptr<IGraphics> m_graphics;

private:
	void Recreate();
	void UpdateBindings(MaterialPropertyCollection* meshPropertiesCollection);

public:
	MaterialRenderData(
		const std::shared_ptr<Logger>& logger,
		const std::shared_ptr<Renderer>& renderer,
		const std::shared_ptr<IGraphics>& graphics);

	void Update(
		const std::shared_ptr<Material>& material,
		MaterialPropertyCollection* meshPropertiesCollection);

	const Array<std::shared_ptr<IGraphicsResourceSet>>& GetResourceSets();

};
