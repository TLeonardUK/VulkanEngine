#pragma once

#include "Engine/Resources/ResourceLoader.h"
#include "Engine/Resources/Resource.h"
#include "Engine/Resources/ResourceManager.h"

#include "Engine/Resources/Types/Texture.h"
#include "Engine/Resources/Types/Shader.h"
#include "Engine/Resources/Types/MaterialPropertyCollection.h"

#include "Engine/Graphics/GraphicsEnums.h"

#include "Engine/Types/Math.h"
#include "Engine/Types/Array.h"
#include "Engine/Utilities/Enum.h"

class TextureResourceLoader;
class Shader;
class Binding;
class IGraphics;
class IGraphicsRenderPass;
class IGraphicsPipeline;
class Logger;
class Renderer;
struct ShaderBindingField;

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

	std::shared_ptr<IGraphicsRenderPass> m_renderPass;
	std::shared_ptr<IGraphicsPipeline> m_pipeline;

	std::shared_ptr<IGraphicsResourceSet> m_resourceSet;

	Dictionary<String, std::shared_ptr<IGraphicsUniformBuffer>> m_uniformBuffers;

	std::shared_ptr<Shader> m_lastUpdatedShader;

	bool m_dirty;

private:
	friend class Renderer;
	friend class Model;
	friend class Mesh;
	friend class MaterialResourceLoader;

	void RecreateResources();
	void UpdateBindings();

	bool FillUniformBuffer(std::shared_ptr<IGraphicsUniformBuffer> buffer, const ShaderBinding& binding);

public:
	static const char* Tag;

	Material(std::shared_ptr<IGraphics> graphics, std::shared_ptr<Renderer> renderer, std::shared_ptr<Logger> logger, const String& name, ResourcePtr<Shader> shader, MaterialPropertyCollection& properties);

	void UpdateResources();

	std::shared_ptr<IGraphicsRenderPass> GetRenderPass();
	std::shared_ptr<IGraphicsPipeline> GetPipeline();
	std::shared_ptr<IGraphicsResourceSet> GetResourceSet();

	bool GetVertexBufferFormat(VertexBufferBindingDescription& format);

	MaterialPropertyCollection& GetProperties();

	ResourcePtr<Shader> GetShader();

};
