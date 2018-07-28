#pragma once
#include "Pch.h"

#include "Engine/Resources/ResourceLoader.h"
#include "Engine/Resources/Resource.h"
#include "Engine/Resources/ResourceManager.h"

#include "Engine/Resources/Types/Texture.h"
#include "Engine/Resources/Types/Shader.h"
#include "Engine/Resources/Types/MaterialPropertyCollection.h"

#include "Engine/Graphics/GraphicsEnums.h"
#include "Engine/Graphics/GraphicsResourceSetPool.h"

#include "Engine/Rendering/RendererEnums.h"

#include "Engine/Types/Math.h"
#include "Engine/Types/Array.h"
#include "Engine/Utilities/Enum.h"

class TextureResourceLoader;
class Shader;
class Binding;
class IGraphics;
class IGraphicsRenderPass;
class IGraphicsPipeline;
class IGraphicsFramebuffer;
class Logger;
class Renderer;
struct ShaderBindingField;

// Gets object-specific data required for rendering this material.
class MaterialRenderData
{
private:
	Array<std::shared_ptr<IGraphicsUniformBuffer>> m_uniformBuffers;
	std::shared_ptr<IGraphicsResourceSet> m_resourceSet;
	std::shared_ptr<IGraphicsResourceSetInstance> m_resourceSetInstance;

	std::shared_ptr<Material> m_lastKnownMaterial = nullptr;
	int m_lastMeshPropertiesVersion = -1;
	int m_lastMaterialPropertiesVersion = -1;

	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<Renderer> m_renderer;
	std::shared_ptr<IGraphics> m_graphics;

private:
	void Recreate();
	void UpdateBindings(MaterialPropertyCollection* meshPropertiesCollection);
	MaterialProperty* GetMaterialPropertyFromCollections(MaterialPropertyHash hash, MaterialPropertyCollection** collections, int collectionCount);

public:
	MaterialRenderData(
		const std::shared_ptr<Logger>& logger,
		const std::shared_ptr<Renderer>& renderer,
		const std::shared_ptr<IGraphics>& graphics);

	void Update(
		const std::shared_ptr<Material>& material,
		MaterialPropertyCollection* meshPropertiesCollection);

	const std::shared_ptr<IGraphicsResourceSet>& GetResourceSet();
	const std::shared_ptr<IGraphicsResourceSetInstance>& GetResourceSetInstance();

};

class Material
	: public IResource
{
private:
	std::shared_ptr<Logger> m_logger;

	ResourcePtr<Shader> m_shader;
	MaterialPropertyCollection m_properties;

	std::shared_ptr<IGraphics> m_graphics;
	std::shared_ptr<Renderer> m_renderer;
	String m_name;

	std::shared_ptr<IGraphicsPipeline> m_pipeline;
	std::shared_ptr<IGraphicsPipeline> m_wireframePipeline;
	
	GraphicsResourceSetDescription m_resourceSetDescription;
	std::shared_ptr<IGraphicsResourceSet> m_resourceSet;

	std::shared_ptr<Shader> m_lastUpdatedShader;

	bool m_dirty;

private:
	friend class Renderer;
	friend class Model;
	friend class Mesh;
	friend class MaterialResourceLoader;

	void RecreateResources();

public:
	static const char* Tag;

	Material(std::shared_ptr<IGraphics> graphics, std::shared_ptr<Renderer> renderer, std::shared_ptr<Logger> logger, const String& name, ResourcePtr<Shader> shader, MaterialPropertyCollection& properties);

	void UpdateResources();

	std::shared_ptr<IGraphicsRenderPass> GetRenderPass();
	std::shared_ptr<IGraphicsFramebuffer> GetFrameBuffer();

	std::shared_ptr<IGraphicsPipeline> GetPipeline();
	std::shared_ptr<IGraphicsPipeline> GetWireframePipeline();
	GraphicsResourceSetDescription GetResourceSetDescription();

	String GetName();
	bool GetVertexBufferFormat(VertexBufferBindingDescription& format);

	MaterialPropertyCollection& GetProperties();

	ResourcePtr<Shader> GetShader();

};
