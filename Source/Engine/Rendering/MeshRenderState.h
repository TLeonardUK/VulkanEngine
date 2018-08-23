#pragma once
#include "Pch.h"

#include "Engine/Resources/Types/Texture.h"
#include "Engine/Resources/Types/Shader.h"
#include "Engine/Rendering/RenderPropertyCollection.h"
#include "Engine/Rendering/RenderPropertyHeirarchy.h"

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
struct RenderPropertyHeirarchy;

// Gets object-specific data required for rendering this material.
class MeshRenderState
{
private:	
	struct HeirarchyResourceSetData
	{
		std::shared_ptr<Material> lastKnownMaterial = nullptr;
		Array<std::shared_ptr<IGraphicsResourceSet>> sets;
		RenderPropertyHeirarchyVersion lastHeirarchyVersion;
	};

	Dictionary<size_t, HeirarchyResourceSetData*> m_sets;
	Mutex m_setsMutex;

	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<Renderer> m_renderer;
	std::shared_ptr<IGraphics> m_graphics;

private:
	void RecreateHeirarchyResourceSet(
		RenderPropertyHeirarchy* heirarchy,
		HeirarchyResourceSetData* resourceSet);

public:
	MeshRenderState(
		const std::shared_ptr<Logger>& logger,
		const std::shared_ptr<Renderer>& renderer,
		const std::shared_ptr<IGraphics>& graphics);

	~MeshRenderState();

	const Array<std::shared_ptr<IGraphicsResourceSet>>& UpdateAndGetResourceSets(
		const std::shared_ptr<Material>& material,
		RenderPropertyHeirarchy* heirarchy);

};
