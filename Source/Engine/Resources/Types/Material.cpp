#include "Pch.h"

#include "Engine/Engine/Logging.h"
#include "Engine/Graphics/Graphics.h"
#include "Engine/Rendering/RenderPropertyHeirarchy.h"
#include "Engine/Resources/Types/Material.h"
#include "Engine/Resources/Types/Shader.h"
#include "Engine/Rendering/Renderer.h"
#include "Engine/Utilities/Statistic.h"
#include "Engine/Types/Hash.h"

const char* Material::Tag = "Material";

Statistic Stat_Resources_MaterialCount("Resources/Material Count", StatisticFrequency::Persistent, StatisticFormat::Integer);

Material::Material(
	std::shared_ptr<IGraphics> graphics, 
	std::shared_ptr<Renderer> renderer, 
	std::shared_ptr<Logger> logger, 
	const String& name, 
	ResourcePtr<Shader> shader, 
	RenderPropertyCollection& properties, 
	std::weak_ptr<Material> variantParent,
	MaterialVariant variant)
	: m_graphics(graphics)
	, m_renderer(renderer)
	, m_logger(logger)
	, m_name(name)
	, m_shader(shader)
	, m_dirty(true)
	, m_properties(properties)
	, m_variantParent(variantParent)
	, m_variantsCreated(false)
	, m_variant(variant)
{
	Stat_Resources_MaterialCount.Add(1);
}

Material::~Material()
{
	Stat_Resources_MaterialCount.Add(-1);
}

std::shared_ptr<Material> Material::GetVariant(MaterialVariant variant)
{
	return m_variants[(int)variant];
}

ResourcePtr<Shader> Material::GetShader()
{
	return m_shader;
}

std::shared_ptr<IGraphicsPipeline> Material::GetPipeline()
{
	return m_pipeline;
}

std::shared_ptr<IGraphicsPipeline> Material::GetWireframePipeline()
{
	return m_wireframePipeline;
}

String Material::GetName()
{
	return m_name;
}

bool Material::GetVertexBufferFormat(VertexBufferBindingDescription& vertexBufferFormat, Array<ShaderVertexStream> remapToStreams)
{
	std::shared_ptr<Shader> shader = m_shader.Get();

	// Build description of vertex data.
	vertexBufferFormat = {};

	const ShaderStage* vertexStage;
	if (!shader->GetStage(GraphicsPipelineStage::Vertex, &vertexStage))
	{
		m_logger->WriteWarning(LogCategory::Resources, "[%-30s] Could not calculate vertex buffer format. Shader does not have a vertex pipeline stage defined.", m_name.c_str());
		return false;
	}

	// If we are a variant, get format from parent, so we can re-use index/vert buffers created
	// for the base material.
	std::shared_ptr<Material> parent = m_variantParent.lock();
	if (parent != nullptr)
	{
		return parent->GetVertexBufferFormat(vertexBufferFormat, vertexStage->Streams);
	}

	// Figure out vertex size.
	int vertexSize = 0;
	for (auto& stream : vertexStage->Streams)
	{
		switch (stream.BindTo)
		{
		case ShaderVertexStreamBinding::Position:	vertexSize += sizeof(Vector3); break;
		case ShaderVertexStreamBinding::Color:		vertexSize += sizeof(Vector4); break;
		case ShaderVertexStreamBinding::Normal:		vertexSize += sizeof(Vector3); break;
		case ShaderVertexStreamBinding::TexCoord1:	vertexSize += sizeof(Vector2); break;
		case ShaderVertexStreamBinding::TexCoord2:	vertexSize += sizeof(Vector2); break;
		case ShaderVertexStreamBinding::TexCoord3:	vertexSize += sizeof(Vector2); break;
		case ShaderVertexStreamBinding::TexCoord4:	vertexSize += sizeof(Vector2); break;
		case ShaderVertexStreamBinding::Internal:	vertexSize += GetByteSizeForGraphicsBindingFormat(stream.Format); break;
		}
	}
	vertexBufferFormat.SetVertexSize(vertexSize);

	int remappedStreamCount = 0;

	// Build interleaved vertex data.
	int bindingOffset = 0;
	for (auto& stream : vertexStage->Streams)
	{
		GraphicsBindingFormat streamFormat;
		int streamSize = 0;
		bool dataExists = false;

		void* sourceData = nullptr;

		switch (stream.BindTo)
		{
		case ShaderVertexStreamBinding::Position:
		{
			streamFormat = GraphicsBindingFormat::Float3;
			streamSize = sizeof(Vector3);
			break;
		}
		case ShaderVertexStreamBinding::Color:
		{
			streamFormat = GraphicsBindingFormat::Float4;
			streamSize = sizeof(Vector4);
			break;
		}
		case ShaderVertexStreamBinding::Normal:
		{
			streamFormat = GraphicsBindingFormat::Float3;
			streamSize = sizeof(Vector3);
			break;
		}
		case ShaderVertexStreamBinding::TexCoord1:
		{
			streamFormat = GraphicsBindingFormat::Float2;
			streamSize = sizeof(Vector2);
			break;
		}
		case ShaderVertexStreamBinding::TexCoord2:
		{
			streamFormat = GraphicsBindingFormat::Float2;
			streamSize = sizeof(Vector2);
			break;
		}
		case ShaderVertexStreamBinding::TexCoord3:
		{
			streamFormat = GraphicsBindingFormat::Float2;
			streamSize = sizeof(Vector2);
			break;
		}
		case ShaderVertexStreamBinding::TexCoord4:
		{
			streamFormat = GraphicsBindingFormat::Float2;
			streamSize = sizeof(Vector2);
			break;
		}
		case ShaderVertexStreamBinding::Internal:
		{
			streamFormat = stream.Format;
			streamSize = GetByteSizeForGraphicsBindingFormat(stream.Format);
			break;
		}
		}

		if (remapToStreams.size() > 0)
		{
			for (auto& remapStream : remapToStreams)
			{
				if (remapStream.BindTo == stream.BindTo)
				{
					vertexBufferFormat.AddAttribute(remapStream.Name, remapStream.Location, streamFormat, vertexSize, bindingOffset);
					remappedStreamCount++;
				}
			}
		}
		else
		{
			vertexBufferFormat.AddAttribute(stream.Name, stream.Location, streamFormat, vertexSize, bindingOffset);
		}
		bindingOffset += streamSize;
	}

	if (remapToStreams.size() > 0 && remappedStreamCount != remapToStreams.size())
	{
		m_logger->WriteWarning(LogCategory::Resources, "[0x%08x %-30s] variant %i of 0x%08x Parent of variant material does not contain vertex streams that variant wants to bind. Material variant may not behave correctly.", this, m_name.c_str(), m_variant, this);
		return false;
	}

	return true;
}

const Array<MaterialResourceSet>& Material::GetResourceSets()
{
	return m_resourceSets;
}

void Material::UpdateResources()
{
	std::shared_ptr<Shader> shader = m_shader.Get();
	if (shader != m_lastUpdatedShader)
	{
		//printf("[0x%08x %-30s], shader changed from 0x%08x to 0x%08x\n",
		//	this, GetName().c_str(), &*m_lastUpdatedShader, &*shader);
		m_lastUpdatedShader = shader;
		m_dirty = true;
	}

	if (m_dirty)
	{
		ScopeLock lock(m_updateResourcesMutex);
		if (!m_dirty)
		{
			return;
		}
		m_dirty = false;

		//printf("[0x%08x %-30s] Recreated due to dirty state\n", this, GetName().c_str());
		RecreateResources();
	}

	// Create variants if not already made.
	if (!m_variantsCreated && m_variantParent.lock() == nullptr)
	{
		m_variants[(int)MaterialVariant::Normal] = shared_from_this();
		m_variants[(int)MaterialVariant::DepthOnly] = std::make_shared<Material>(
			m_graphics,
			m_renderer,
			m_logger,
			m_name + " (Depth Only)",
			m_renderer->GetDepthOnlyShader(),
			m_properties,
			shared_from_this(),
			MaterialVariant::DepthOnly
		);
		m_variantsCreated = true;
	}

	// Update variants.
	for (int i = 0; i < (int)MaterialVariant::Count; i++)
	{
		if (static_cast<MaterialVariant>(i) != MaterialVariant::Normal && m_variants[i] != nullptr)
		{
			m_variants[i]->UpdateResources();
		}
	}
}

void Material::GetResourceSetsForShader(std::shared_ptr<Shader>& shader, Array<MaterialResourceSet>& resourceSets)
{
	const Array<ShaderBinding>& bindings = shader->GetBindings();
	resourceSets.clear();

	// Count all resource sets in use.
	int totalSets = 0;
	for (const ShaderBinding& binding : bindings)
	{
		if (binding.Set + 1 > totalSets)
		{
			totalSets = binding.Set + 1;
		}
	}
	for (int i = 0; i < totalSets; i++)
	{
		MaterialResourceSet set;
		set.description = {};
		set.frequency = GraphicsBindingFrequency::COUNT;
		set.set = nullptr;
		set.hashCode = 1;
		set.name = m_name;

		resourceSets.push_back(set);
	}

	// Generate descriptors set descriptions..
	for (const ShaderBinding& binding : bindings)
	{
		resourceSets[binding.Set].description.name = StringFormat("%s %s", m_name.c_str(), binding.Name.c_str());
		resourceSets[binding.Set].description.AddBinding(binding.Name, binding.Binding, binding.Type, binding.ArrayLength);
		resourceSets[binding.Set].bindings.push_back(binding);

		// If ubo is a global, make sure to register it with the renderer to be filled.
		assert(
			resourceSets[binding.Set].frequency == GraphicsBindingFrequency::COUNT ||
			resourceSets[binding.Set].frequency == binding.Frequency);

		resourceSets[binding.Set].frequency = binding.Frequency;
	}

	// Allocate/Register each resource-set.
	for (auto& set : m_resourceSets)
	{
		set.CalculateHashCode();
		set.set = m_renderer->AllocateResourceSet(set.description);
	}
}

void Material::RecreateResources()
{
	std::shared_ptr<Shader> shader = m_shader.Get();

	VertexBufferBindingDescription vertexBufferFormat;

	//printf("[0x%08x %-30s] Recreating resources for shader variant %i\n", this, GetName().c_str(), m_variant);

	if (!GetVertexBufferFormat(vertexBufferFormat, {}))
	{
		return;
	}

	GetResourceSetsForShader(shader, m_resourceSets);

	// Create pipeline.
	GraphicsPipelineSettings pipelineSettings;
	{
		pipelineSettings = shader->GetPipelineDescription();
		pipelineSettings.RenderPass = GetRenderPass();
		pipelineSettings.VertexFormatDescription = vertexBufferFormat;
		pipelineSettings.HasVertexFormatDescription = true;
		for (auto& set : m_resourceSets)
		{
			pipelineSettings.ResourceSets.push_back(set.set);
		}

		m_pipeline = m_graphics->CreatePipeline(StringFormat("%s Pipeline", m_name.c_str()), pipelineSettings);
	}

	// Create wireframe pipeline.
	{
		pipelineSettings.PolygonMode = GraphicsPolygonMode::Line;
		m_wireframePipeline = m_graphics->CreatePipeline(StringFormat("%s Pipeline (Wireframe)", m_name.c_str()), pipelineSettings);
	}
}

RenderPropertyCollection& Material::GetProperties()
{
	return m_properties;
}

std::shared_ptr<IGraphicsRenderPass> Material::GetRenderPass()
{
	return  m_renderer->GetRenderPassForTarget(m_shader.Get()->GetTarget());
}

std::shared_ptr<IGraphicsFramebuffer> Material::GetFrameBuffer()
{
	return  m_renderer->GetFramebufferForTarget(m_shader.Get()->GetTarget());
}
