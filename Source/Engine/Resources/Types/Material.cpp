#include "Pch.h"

#include "Engine/Engine/Logging.h"
#include "Engine/Graphics/Graphics.h"
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
	MaterialPropertyCollection& properties, 
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
		std::lock_guard<std::mutex> lock(m_updateResourcesMutex);
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
		set.isGlobal = true;
		set.set = nullptr;
		set.hashCode = 1;
		set.name = m_name;

		resourceSets.push_back(set);
	}

	// Generate descriptors set descriptions..
	for (const ShaderBinding& binding : bindings)
	{
		resourceSets[binding.Set].description.AddBinding(binding.Name, binding.Binding, binding.Type);
		resourceSets[binding.Set].bindings.push_back(binding);

		// If ubo is a global, make sure to register it with the renderer to be filled.
		if (binding.Type == GraphicsBindingType::UniformBufferObject &&
			binding.UniformBufferLayout.Frequency == GraphicsBindingFrequency::Global)
		{
			m_renderer->RegisterGlobalUniformBuffer(binding.UniformBufferLayout);
		}
		else
		{
			resourceSets[binding.Set].isGlobal = false;
		}
	}

	// Allocate/Register each resource-set.
	for (auto& set : m_resourceSets)
	{
		set.CalculateHashCode();

		set.set = m_renderer->AllocateResourceSet(set.description);

		if (set.isGlobal)
		{
			set.set = m_renderer->RegisterGlobalResourceSet(set);
		}
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

MaterialPropertyCollection& Material::GetProperties()
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

MaterialProperty* GetMaterialPropertyFromCollections(MaterialPropertyHash hash, MaterialPropertyCollection** collections, int collectionCount)
{
	MaterialProperty* prop = nullptr;

	for (int i = 0; i < collectionCount; i++)
	{
		if (collections[i] != nullptr &&
			collections[i]->Get(hash, &prop))
		{
			return prop;
		}
	}

	return nullptr;
}

void MaterialResourceSet::CalculateHashCode()
{
	hashCode = 1;

	CombineHash(hashCode, bindings.size());

	for (ShaderBinding& binding : bindings)
	{
		CombineHash(hashCode, binding.Binding);
		CombineHash(hashCode, binding.Set);
		CombineHash(hashCode, binding.BindTo);
		CombineHash(hashCode, binding.Name);
		CombineHash(hashCode, binding.Type);
		CombineHash(hashCode, binding.UniformBufferLayout.HashCode);
	}
}

void MaterialResourceSet::UpdateBindings(
	const std::shared_ptr<Renderer>& renderer,
	const std::shared_ptr<Logger>& logger,
	const Array<std::shared_ptr<IGraphicsUniformBuffer>>& meshUbos,
	MaterialPropertyCollection* meshPropertiesCollection,
	MaterialPropertyCollection* materialPropertiesCollection,
	const std::shared_ptr<IGraphicsResourceSet>& updateSet
)
{
	MaterialPropertyCollection* meshFrequencyPropertyCollections[2] = {
		meshPropertiesCollection,
		materialPropertiesCollection,
	};

	int meshUboIndex = 0;

	for (ShaderBinding& binding : bindings)
	{
		switch (binding.Type)
		{
		case GraphicsBindingType::UniformBufferObject:
			{
				std::shared_ptr<IGraphicsUniformBuffer> buffer = nullptr;

				if (binding.UniformBufferLayout.Frequency == GraphicsBindingFrequency::Global)
				{
					buffer = renderer->GetGlobalUniformBuffer(binding.UniformBufferLayout.HashCode);
				}
				else if (binding.UniformBufferLayout.Frequency == GraphicsBindingFrequency::Mesh)
				{
					buffer = meshUbos[meshUboIndex++];
					binding.UniformBufferLayout.FillBuffer(logger, buffer, meshFrequencyPropertyCollections, 2);
				}
				else
				{
					assert(false);
				}

				updateSet->UpdateBinding(binding.Binding, 0, buffer);

				break;
			}
		case GraphicsBindingType::Sampler:
			{
				MaterialProperty* matBinding = GetMaterialPropertyFromCollections(binding.BindToHash, meshFrequencyPropertyCollections, 2);
				if (matBinding == nullptr)
				{
					logger->WriteWarning(LogCategory::Resources, "[%-30s] Could not bind '%s' to mesh, property '%s' does not exist.", name.c_str(), binding.Name.c_str(), binding.BindTo.c_str());
					continue;
				}

				if (matBinding->Format != GraphicsBindingFormat::Texture)
				{
					String expectedFormat = EnumToString<GraphicsBindingFormat>(GraphicsBindingFormat::Texture);
					String actualFormat = EnumToString<GraphicsBindingFormat>(matBinding->Format);

					logger->WriteWarning(LogCategory::Resources, "[%-30s] Could not bind '%s' to mesh, property format is '%s' expected '%s'.", name.c_str(), binding.Name.c_str(), actualFormat, expectedFormat);
					continue;
				}

				std::shared_ptr<Texture> texture = matBinding->Value_Texture.Get();
				if (matBinding->Value_ImageSampler == nullptr &&
					matBinding->Value_ImageView == nullptr)
				{
					updateSet->UpdateBinding(binding.Binding, 0, texture->GetSampler(), texture->GetImageView());
				}
				else
				{
					updateSet->UpdateBinding(binding.Binding, 0, matBinding->Value_ImageSampler, matBinding->Value_ImageView);
				}

				break;
			}
		case GraphicsBindingType::SamplerCube:
			{
				MaterialProperty* matBinding = GetMaterialPropertyFromCollections(binding.BindToHash, meshFrequencyPropertyCollections, 2);
				if (matBinding == nullptr)
				{
					logger->WriteWarning(LogCategory::Resources, "[%-30s] Could not bind '%s' to mesh, property '%s' does not exist.", name.c_str(), binding.Name.c_str(), binding.BindTo.c_str());
					continue;
				}

				if (matBinding->Format != GraphicsBindingFormat::TextureCube)
				{
					String expectedFormat = EnumToString<GraphicsBindingFormat>(GraphicsBindingFormat::TextureCube);
					String actualFormat = EnumToString<GraphicsBindingFormat>(matBinding->Format);

					logger->WriteWarning(LogCategory::Resources, "[%-30s] Could not bind '%s' to mesh, property format is '%s' expected '%s'.", name.c_str(), binding.Name.c_str(), actualFormat, expectedFormat);
					continue;
				}

				std::shared_ptr<TextureCube> texture = matBinding->Value_TextureCube.Get();
				if (matBinding->Value_ImageSampler == nullptr &&
					matBinding->Value_ImageView == nullptr)
				{
					updateSet->UpdateBinding(binding.Binding, 0, texture->GetSampler(), texture->GetImageView());
				}
				else
				{
					updateSet->UpdateBinding(binding.Binding, 0, matBinding->Value_ImageSampler, matBinding->Value_ImageView);
				}

				break;
			}
		}
	}
}