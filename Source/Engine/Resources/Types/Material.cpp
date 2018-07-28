#include "Pch.h"

#include "Engine/Engine/Logging.h"
#include "Engine/Graphics/Graphics.h"
#include "Engine/Resources/Types/Material.h"
#include "Engine/Resources/Types/Shader.h"
#include "Engine/Rendering/Renderer.h"

const char* Material::Tag = "Material";

Material::Material(std::shared_ptr<IGraphics> graphics, std::shared_ptr<Renderer> renderer, std::shared_ptr<Logger> logger, const String& name, ResourcePtr<Shader> shader, MaterialPropertyCollection& properties)
	: m_graphics(graphics)
	, m_renderer(renderer)
	, m_logger(logger)
	, m_name(name)
	, m_shader(shader)
	, m_dirty(true)
	, m_properties(properties)
{
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

bool Material::GetVertexBufferFormat(VertexBufferBindingDescription& vertexBufferFormat)
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

		vertexBufferFormat.AddAttribute(stream.Name, stream.Location, streamFormat, vertexSize, bindingOffset);
		bindingOffset += streamSize;
	}

	return true;
}

GraphicsResourceSetDescription Material::GetResourceSetDescription()
{
	return m_resourceSetDescription;
}

void Material::UpdateResources()
{
	std::shared_ptr<Shader> shader = m_shader.Get();
	if (shader != m_lastUpdatedShader)
	{
		m_lastUpdatedShader = shader;
		m_dirty = true;
	}

	if (m_dirty)
	{
		m_dirty = false;
		RecreateResources();
	}
}

void Material::RecreateResources()
{
	std::shared_ptr<Shader> shader = m_shader.Get();

	VertexBufferBindingDescription vertexBufferFormat;
	if (!GetVertexBufferFormat(vertexBufferFormat))
	{
		return;
	}

	// Create resource set and uniform buffers as appropriate.
	m_resourceSetDescription = {};

	const Array<ShaderBinding>& bindings = shader->GetBindings();
	for (const ShaderBinding& binding : bindings)
	{
		m_resourceSetDescription.AddBinding(binding.Name, binding.Binding, binding.Type);

		// If ubo is a global, make sure to register it with the renderer to be filled.
		if (binding.Type == GraphicsBindingType::UniformBufferObject &&
			binding.UniformBufferLayout.Frequency == GraphicsBindingFrequency::Global)
		{
			m_renderer->RegisterGlobalUniformBuffer(binding.UniformBufferLayout);
		}
	}

	m_resourceSet = m_renderer->AllocateResourceSet(m_resourceSetDescription);

	// Create pipeline.
	GraphicsPipelineSettings pipelineSettings = shader->GetPipelineDescription();
	pipelineSettings.RenderPass = GetRenderPass();
	pipelineSettings.VertexFormatDescription = vertexBufferFormat;
	pipelineSettings.HasVertexFormatDescription = true;
	pipelineSettings.ResourceSets.push_back(m_resourceSet);

	m_pipeline = m_graphics->CreatePipeline(StringFormat("%s Pipeline", m_name.c_str()), pipelineSettings);

	pipelineSettings.PolygonMode = GraphicsPolygonMode::Line;
	m_wireframePipeline = m_graphics->CreatePipeline(StringFormat("%s Pipeline (Wireframe)", m_name.c_str()), pipelineSettings);
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

MaterialRenderData::MaterialRenderData(
	const std::shared_ptr<Logger>& logger,
	const std::shared_ptr<Renderer>& renderer,
	const std::shared_ptr<IGraphics>& graphics)
	: m_logger(logger)
	, m_renderer(renderer)
	, m_graphics(graphics)
{
}

void MaterialRenderData::Update(
	const std::shared_ptr<Material>& material,
	MaterialPropertyCollection* meshPropertiesCollection)
{
	bool regenerate = true;

	if (&*material != &*m_lastKnownMaterial)
	{
		m_logger->WriteInfo(LogCategory::Engine, "Mateiral changed from %s to %s",
			material == nullptr ? "null" : material->GetName().c_str(),
			m_lastKnownMaterial == nullptr ? "null" : m_lastKnownMaterial->GetName().c_str()
		);

		m_lastKnownMaterial = material;
		m_lastMeshPropertiesVersion = -1;
		m_lastMaterialPropertiesVersion = -1;

		Recreate();
	}

	int meshPropertiesVersion = (meshPropertiesCollection != nullptr ? meshPropertiesCollection->GetVersion() : m_lastMeshPropertiesVersion);
	int materialPropertiesVersion = material->GetProperties().GetVersion();

	if (meshPropertiesVersion != m_lastMeshPropertiesVersion ||
		materialPropertiesVersion != m_lastMaterialPropertiesVersion)
	{
		UpdateBindings(meshPropertiesCollection);

		m_lastMaterialPropertiesVersion = materialPropertiesVersion;
		m_lastMeshPropertiesVersion = meshPropertiesVersion;
	}
}

const std::shared_ptr<IGraphicsResourceSet>& MaterialRenderData::GetResourceSet()
{
	return m_resourceSet;
}

const std::shared_ptr<IGraphicsResourceSetInstance>& MaterialRenderData::GetResourceSetInstance()
{
	if (m_resourceSetInstance == nullptr)
	{
		m_resourceSetInstance = m_resourceSet->NewInstance();
	}
	else
	{
		m_resourceSet->UpdateInstance(m_resourceSetInstance);
	}

	return m_resourceSetInstance;
}

MaterialProperty* MaterialRenderData::GetMaterialPropertyFromCollections(MaterialPropertyHash hash, MaterialPropertyCollection** collections, int collectionCount)
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

void MaterialRenderData::Recreate()
{
	m_resourceSet = m_renderer->AllocateResourceSet(m_lastKnownMaterial->GetResourceSetDescription());

	const Array<ShaderBinding>& bindings = m_lastKnownMaterial->GetShader().Get()->GetBindings();

	m_uniformBuffers.clear();

	for (const ShaderBinding& binding : bindings)
	{
		if (binding.Type == GraphicsBindingType::UniformBufferObject)
		{
			if (binding.UniformBufferLayout.Frequency == GraphicsBindingFrequency::Mesh)
			{
				int dataSize = binding.UniformBufferLayout.GetSize();

				std::shared_ptr<IGraphicsUniformBuffer> uniformBuffer = m_graphics->CreateUniformBuffer(
					StringFormat("%s (%s)", m_lastKnownMaterial->GetName().c_str(), binding.Name.c_str()),
					dataSize);

				m_uniformBuffers.push_back(uniformBuffer);
			}
		}
	}
}

void MaterialRenderData::UpdateBindings(MaterialPropertyCollection* meshPropertiesCollection)
{
	Array<ShaderBinding> bindings = m_lastKnownMaterial->GetShader().Get()->GetBindings();

	MaterialPropertyCollection* meshFrequencyPropertyCollections[2] = {
		meshPropertiesCollection,
		&m_lastKnownMaterial->GetProperties()
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
					buffer = m_renderer->GetGlobalUniformBuffer(binding.UniformBufferLayout.HashCode);
				}
				else if (binding.UniformBufferLayout.Frequency == GraphicsBindingFrequency::Mesh)
				{
					buffer = m_uniformBuffers[meshUboIndex++];
					binding.UniformBufferLayout.FillBuffer(m_logger, buffer, meshFrequencyPropertyCollections, 2);
				}
				else
				{
					assert(false);
				}

				m_resourceSet->UpdateBinding(binding.Binding, 0, buffer);
				break;
			}
		case GraphicsBindingType::Sampler:
			{
				MaterialProperty* matBinding = GetMaterialPropertyFromCollections(binding.BindToHash, meshFrequencyPropertyCollections, 2);
				if (matBinding == nullptr)
				{
					m_logger->WriteWarning(LogCategory::Resources, "[%-30s] Could not bind '%s' to mesh, property '%s' does not exist.", m_lastKnownMaterial->GetName().c_str(), binding.Name.c_str(), binding.BindTo.c_str());
					continue;
				}

				if (matBinding->Format != GraphicsBindingFormat::Texture)
				{
					String expectedFormat = EnumToString<GraphicsBindingFormat>(GraphicsBindingFormat::Texture);
					String actualFormat = EnumToString<GraphicsBindingFormat>(matBinding->Format);

					m_logger->WriteWarning(LogCategory::Resources, "[%-30s] Could not bind '%s' to mesh, property format is '%s' expected '%s'.", m_lastKnownMaterial->GetName().c_str(), binding.Name.c_str(), actualFormat, expectedFormat);
					continue;
				}

				std::shared_ptr<Texture> texture = matBinding->Value_Texture.Get();
				if (matBinding->Value_ImageSampler == nullptr &&
					matBinding->Value_ImageView == nullptr)
				{
					m_resourceSet->UpdateBinding(binding.Binding, 0, texture->GetSampler(), texture->GetImageView());
				}
				else
				{
					m_resourceSet->UpdateBinding(binding.Binding, 0, matBinding->Value_ImageSampler, matBinding->Value_ImageView);
				}

				break;
			}
		case GraphicsBindingType::SamplerCube:
			{
				MaterialProperty* matBinding = GetMaterialPropertyFromCollections(binding.BindToHash, meshFrequencyPropertyCollections, 2);
				if (matBinding == nullptr)
				{
					m_logger->WriteWarning(LogCategory::Resources, "[%-30s] Could not bind '%s' to mesh, property '%s' does not exist.", m_lastKnownMaterial->GetName().c_str(), binding.Name.c_str(), binding.BindTo.c_str());
					continue;
				}

				if (matBinding->Format != GraphicsBindingFormat::TextureCube)
				{
					String expectedFormat = EnumToString<GraphicsBindingFormat>(GraphicsBindingFormat::TextureCube);
					String actualFormat = EnumToString<GraphicsBindingFormat>(matBinding->Format);

					m_logger->WriteWarning(LogCategory::Resources, "[%-30s] Could not bind '%s' to mesh, property format is '%s' expected '%s'.", m_lastKnownMaterial->GetName().c_str(), binding.Name.c_str(), actualFormat, expectedFormat);
					continue;
				}

				std::shared_ptr<TextureCube> texture = matBinding->Value_TextureCube.Get();
				if (matBinding->Value_ImageSampler == nullptr &&
					matBinding->Value_ImageView == nullptr)
				{
					m_resourceSet->UpdateBinding(binding.Binding, 0, texture->GetSampler(), texture->GetImageView());
				}
				else
				{
					m_resourceSet->UpdateBinding(binding.Binding, 0, matBinding->Value_ImageSampler, matBinding->Value_ImageView);
				}

				break;
			}
		}
	}
}