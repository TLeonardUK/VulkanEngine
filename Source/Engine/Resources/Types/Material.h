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

struct MaterialResourceSet
{
public:
	String name;
	GraphicsResourceSetDescription description;
	std::shared_ptr<IGraphicsResourceSet> set;
	Array<ShaderBinding> bindings;
	bool isGlobal;
	size_t hashCode;

public:
	void CalculateHashCode();
	void UpdateBindings(
		const std::shared_ptr<Renderer>& renderer,
		const std::shared_ptr<Logger>& logger,
		const Array<std::shared_ptr<IGraphicsUniformBuffer>>& meshUbos,
		MaterialPropertyCollection* meshPropertiesCollection, 
		MaterialPropertyCollection* materialPropertiesCollection,
		const std::shared_ptr<IGraphicsResourceSet>& updateSet
	);
};

enum class MaterialVariant
{
	Normal,
	DepthOnly,
	Count
};

class Material
	: public IResource
	, public std::enable_shared_from_this<Material>
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

	Array<MaterialResourceSet> m_resourceSets;
	Array<MaterialResourceSet> m_depthOnlyResourceSets;
	
	std::shared_ptr<Shader> m_lastUpdatedShader;

	std::mutex m_updateResourcesMutex;

	bool m_variantsCreated;
	std::weak_ptr<Material> m_variantParent;
	MaterialVariant m_variant;
	std::shared_ptr<Material> m_variants[(int)MaterialVariant::Count];

	bool m_dirty;

private:
	friend class Renderer;
	friend class Model;
	friend class Mesh;
	friend class MaterialResourceLoader;

	void RecreateResources();

	void GetResourceSetsForShader(std::shared_ptr<Shader>& shader, Array<MaterialResourceSet>& resourceSets);

public:
	static const char* Tag;

	Material(
		std::shared_ptr<IGraphics> graphics, 
		std::shared_ptr<Renderer> renderer, 
		std::shared_ptr<Logger> logger,
		const String& name, 
		ResourcePtr<Shader> shader, 
		MaterialPropertyCollection& properties, 
		std::weak_ptr<Material> variantParent,
		MaterialVariant variant);

	virtual ~Material();

	void UpdateResources();

	std::shared_ptr<IGraphicsRenderPass> GetRenderPass();
	std::shared_ptr<IGraphicsFramebuffer> GetFrameBuffer();

	std::shared_ptr<IGraphicsPipeline> GetPipeline();
	std::shared_ptr<IGraphicsPipeline> GetWireframePipeline();
	
	const Array<MaterialResourceSet>& GetResourceSets();

	std::shared_ptr<Material> GetVariant(MaterialVariant variant);

	String GetName();
	bool GetVertexBufferFormat(VertexBufferBindingDescription& format, Array<ShaderVertexStream> remapToStreams = {});

	MaterialPropertyCollection& GetProperties();

	ResourcePtr<Shader> GetShader();

};
