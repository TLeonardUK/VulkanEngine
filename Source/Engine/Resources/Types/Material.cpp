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

std::shared_ptr<IGraphicsRenderPass> Material::GetRenderPass()
{
	return m_renderPass;
}

std::shared_ptr<IGraphicsPipeline> Material::GetPipeline()
{
	return m_pipeline;
}

std::shared_ptr<IGraphicsResourceSet> Material::GetResourceSet()
{
	return m_resourceSet;
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

	UpdateBindings();
}

bool Material::FillUniformBuffer(std::shared_ptr<IGraphicsUniformBuffer> buffer, const ShaderBinding& binding)
{
	MaterialPropertyCollection& globalProperties = m_renderer->GetGlobalMaterialProperties();

	Array<char> uboData(binding.GetUniformBufferSize());

	for (int i = 0; i < binding.Fields.size(); i++)
	{
		const ShaderBindingField& field = binding.Fields[i];
		int fieldOffset = binding.GetUniformBufferFieldOffset(i);

		MaterialProperty* matBinding;
		if (!m_properties.Get(field.BindToHash, &matBinding))
		{
			if (!globalProperties.Get(field.BindToHash, &matBinding))
			{
				m_logger->WriteWarning(LogCategory::Resources, "[%-30s] Could not bind '%s', property '%s' does not exist.", m_name.c_str(), field.Name.c_str(), field.BindTo.c_str());
				continue;
			}
		}

		// insert into ubo.
		char* destination = uboData.data() + fieldOffset;
		char* source;
		int sourceSize;

		switch (field.Format)
		{
		case GraphicsBindingFormat::Bool:		sourceSize = sizeof(bool);		source = reinterpret_cast<char*>(&matBinding->Value_Bool);		break;
		case GraphicsBindingFormat::Bool2:		sourceSize = sizeof(BVector2);	source = reinterpret_cast<char*>(&matBinding->Value_Bool2);		break;
		case GraphicsBindingFormat::Bool3:		sourceSize = sizeof(BVector3);	source = reinterpret_cast<char*>(&matBinding->Value_Bool3);		break;
		case GraphicsBindingFormat::Bool4:		sourceSize = sizeof(BVector4);	source = reinterpret_cast<char*>(&matBinding->Value_Bool4);		break;
		case GraphicsBindingFormat::Int:		sourceSize = sizeof(int32_t);	source = reinterpret_cast<char*>(&matBinding->Value_Int);		break;
		case GraphicsBindingFormat::Int2:		sourceSize = sizeof(IVector2);	source = reinterpret_cast<char*>(&matBinding->Value_Int2);		break;
		case GraphicsBindingFormat::Int3:		sourceSize = sizeof(IVector3);	source = reinterpret_cast<char*>(&matBinding->Value_Int3);		break;
		case GraphicsBindingFormat::Int4:		sourceSize = sizeof(IVector4);	source = reinterpret_cast<char*>(&matBinding->Value_Int4);		break;
		case GraphicsBindingFormat::UInt:		sourceSize = sizeof(uint32_t);	source = reinterpret_cast<char*>(&matBinding->Value_UInt);		break;
		case GraphicsBindingFormat::UInt2:		sourceSize = sizeof(UVector2);	source = reinterpret_cast<char*>(&matBinding->Value_UInt2);		break;
		case GraphicsBindingFormat::UInt3:		sourceSize = sizeof(UVector3);	source = reinterpret_cast<char*>(&matBinding->Value_UInt3);		break;
		case GraphicsBindingFormat::UInt4:		sourceSize = sizeof(UVector4);	source = reinterpret_cast<char*>(&matBinding->Value_UInt4);		break;
		case GraphicsBindingFormat::Float:		sourceSize = sizeof(float);		source = reinterpret_cast<char*>(&matBinding->Value_Float);		break;
		case GraphicsBindingFormat::Float2:		sourceSize = sizeof(Vector2);	source = reinterpret_cast<char*>(&matBinding->Value_Float2);	break;
		case GraphicsBindingFormat::Float3:		sourceSize = sizeof(Vector3);	source = reinterpret_cast<char*>(&matBinding->Value_Float3);	break;
		case GraphicsBindingFormat::Float4:		sourceSize = sizeof(Vector4);	source = reinterpret_cast<char*>(&matBinding->Value_Float4);	break;
		case GraphicsBindingFormat::Double:		sourceSize = sizeof(double);	source = reinterpret_cast<char*>(&matBinding->Value_Double);	break;
		case GraphicsBindingFormat::Double2:	sourceSize = sizeof(DVector2);	source = reinterpret_cast<char*>(&matBinding->Value_Double2);	break;
		case GraphicsBindingFormat::Double3:	sourceSize = sizeof(DVector3);	source = reinterpret_cast<char*>(&matBinding->Value_Double3);	break;
		case GraphicsBindingFormat::Double4:	sourceSize = sizeof(DVector4);	source = reinterpret_cast<char*>(&matBinding->Value_Double4);	break;
		case GraphicsBindingFormat::Matrix2:	sourceSize = sizeof(Matrix2);	source = reinterpret_cast<char*>(&matBinding->Value_Matrix2);	break;
		case GraphicsBindingFormat::Matrix3:	sourceSize = sizeof(Matrix3);	source = reinterpret_cast<char*>(&matBinding->Value_Matrix3);	break;
		case GraphicsBindingFormat::Matrix4:	sourceSize = sizeof(Matrix4);	source = reinterpret_cast<char*>(&matBinding->Value_Matrix4);	break;
		default:
		{
			m_logger->WriteWarning(LogCategory::Resources, "[%-30s] Could not bind '%s', unknown format.", m_name.c_str(), field.Name.c_str());
			continue;
		}
		}

		memcpy(destination, source, sourceSize);
	}

	buffer->Upload(uboData.data(), 0, uboData.size());

	return true;
}

void Material::UpdateBindings()
{
	MaterialPropertyCollection& globalProperties = m_renderer->GetGlobalMaterialProperties();

	std::shared_ptr<Shader> shader = m_shader.Get();
	const Array<ShaderBinding>& bindings = shader->GetBindings();

	for (const ShaderBinding& binding : bindings)
	{
		switch (binding.Type)
		{
		case GraphicsBindingType::UniformBufferObject:
			{
				std::shared_ptr<IGraphicsUniformBuffer> buffer = m_uniformBuffers[binding.Name];

				if (!FillUniformBuffer(buffer, binding))
				{
					continue;
				}

				m_resourceSet->UpdateBinding(binding.Binding, 0, buffer);
				break;
			}
		case GraphicsBindingType::Sampler:
			{
				MaterialProperty* matBinding;
				
				if (!m_properties.Get(binding.BindToHash, &matBinding))
				{
					if (!globalProperties.Get(binding.BindToHash, &matBinding))
					{
						m_logger->WriteWarning(LogCategory::Resources, "[%-30s] Could not bind '%s', property '%s' does not exist.", m_name.c_str(), binding.Name.c_str(), binding.BindTo.c_str());
						continue;
					}
				}

				if (matBinding->Format != GraphicsBindingFormat::Texture)
				{
					String expectedFormat = EnumToString<GraphicsBindingFormat>(GraphicsBindingFormat::Texture);
					String actualFormat = EnumToString<GraphicsBindingFormat>(matBinding->Format);

					m_logger->WriteWarning(LogCategory::Resources, "[%-30s] Could not bind '%s', property format is '%s' expected '%s'.", m_name.c_str(), binding.Name.c_str(), actualFormat, expectedFormat);
					continue;
				}

				std::shared_ptr<Texture> texture = matBinding->Value_Texture.Get();
				m_resourceSet->UpdateBinding(binding.Binding, 0, texture->GetSampler(), texture->GetImageView());

				break;
			}
		}
	}
}

void Material::RecreateResources()
{
	std::shared_ptr<Shader> shader = m_shader.Get();

	// Create render pass.
	GraphicsRenderPassSettings renderPassSettings;
	renderPassSettings.AddColorAttachment(m_graphics->GetSwapChainFormat(), true);
	renderPassSettings.AddDepthAttachment(GraphicsFormat::UNORM_D24_UINT_S8);

	GraphicsSubPassIndex subPass1 = renderPassSettings.AddSubPass();
	renderPassSettings.AddSubPassDependency(GraphicsExternalPassIndex, GraphicsAccessMask::None, subPass1, GraphicsAccessMask::ReadWrite);

	m_renderPass = m_graphics->CreateRenderPass(StringFormat("%s Render Pass", m_name.c_str()), renderPassSettings);

	VertexBufferBindingDescription vertexBufferFormat;
	if (!GetVertexBufferFormat(vertexBufferFormat))
	{
		return;
	}

	// Create resource set and uniform buffers as appropriate.
	GraphicsResourceSetDescription resourceSetDescription = {};
	const Array<ShaderBinding>& bindings = shader->GetBindings();

	m_uniformBuffers.clear();

	for (const ShaderBinding& binding : bindings)
	{
		resourceSetDescription.AddBinding(binding.Name, binding.Binding, binding.Type);

		if (binding.Type == GraphicsBindingType::UniformBufferObject)
		{
			int dataSize = binding.GetUniformBufferSize();

			std::shared_ptr<IGraphicsUniformBuffer> uniformBuffer = m_graphics->CreateUniformBuffer(StringFormat("%s (%s - %i)", m_name.c_str(), binding.Name.c_str(), binding.Fields.size()), dataSize);

			std::pair<String, std::shared_ptr<IGraphicsUniformBuffer>> pair(binding.Name, uniformBuffer);

			m_uniformBuffers.insert(pair);
		}
	}

	m_resourceSet = m_renderer->AllocateResourceSet(resourceSetDescription);

	// Create pipeline.
	GraphicsPipelineSettings pipelineSettings = shader->GetPipelineDescription();
	pipelineSettings.RenderPass = m_renderPass;
	pipelineSettings.VertexFormatDescription = vertexBufferFormat;
	pipelineSettings.HasVertexFormatDescription = true;
	pipelineSettings.ResourceSets.push_back(m_resourceSet);

	m_pipeline = m_graphics->CreatePipeline(StringFormat("%s Pipeline", m_name.c_str()), pipelineSettings);
}

MaterialPropertyCollection& Material::GetProperties()
{
	return m_properties;
}