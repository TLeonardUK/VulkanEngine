#include "Pch.h"

#include "Engine/Resources/Types/Model.h"
#include "Engine/Resources/Types/Shader.h"
#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/GraphicsIndexBuffer.h"
#include "Engine/Graphics/GraphicsVertexBuffer.h"
#include "Engine/Graphics/GraphicsCommandBuffer.h"
#include "Engine/Rendering/Renderer.h"
#include "Engine/Engine/Logging.h"
#include "Engine/Utilities/Statistic.h"

const char* Model::Tag = "Model";

Statistic Stat_Resources_ModelCount("Resources/Model Count", StatisticFrequency::Persistent, StatisticFormat::Integer);
Statistic Stat_Resources_MeshCount("Resources/Mesh Count", StatisticFrequency::Persistent, StatisticFormat::Integer);

Model::Model(std::shared_ptr<Logger> logger, std::shared_ptr<Renderer> renderer, std::shared_ptr<IGraphics> graphics, const String& name)
	: m_graphics(graphics)
	, m_renderer(renderer)
	, m_logger(logger)
	, m_name(name)
{
	Stat_Resources_ModelCount.Add(1);
}

Model::~Model()
{
	Stat_Resources_ModelCount.Add(-1);
}

std::shared_ptr<Mesh> Model::CreateMesh()
{
	std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(m_logger, m_renderer, m_graphics, StringFormat("%s (Mesh %i)", m_name.c_str(), m_meshes.size() + 1));
	m_meshes.push_back(mesh);
	return mesh;
}

const Array<std::shared_ptr<Mesh>>& Model::GetMeshes()
{
	return m_meshes;
}

void Model::UpdateResources()
{
	for (auto& mesh : m_meshes)
	{
		mesh->UpdateResources();
	}
}

Mesh::Mesh(std::shared_ptr<Logger> logger, std::shared_ptr<Renderer> renderer, std::shared_ptr<IGraphics> graphics, const String& name)
	: m_graphics(graphics)
	, m_renderer(renderer)
	, m_logger(logger)
	, m_name(name)
	, m_dirty(true)
{
	Stat_Resources_MeshCount.Add(1);
}

Mesh::~Mesh()
{
	Stat_Resources_MeshCount.Add(-1);
}

ResourcePtr<Material> Mesh::GetMaterial()
{
	return m_material;
}

Bounds Mesh::GetBounds()
{
	return m_bounds;
}

const std::shared_ptr<IGraphicsIndexBuffer>& Mesh::GetIndexBuffer()
{
	return m_indexBuffer;
}

const std::shared_ptr<IGraphicsVertexBuffer>& Mesh::GetVertexBuffer()
{
	return m_vertexBuffer;
}

int Mesh::GetIndexCount()
{
	return (int)m_indices.size();
}

void Mesh::SetMaterial(ResourcePtr<Material> material)
{
	m_material = material;
	m_dirty = true;
}

void Mesh::SetVertices(const Array<Vector3>& vertices)
{
	m_vertices = vertices;
	m_dirty = true;
}

void Mesh::SetNormals(const Array<Vector3>& normals)
{
	m_normals = normals;
	m_dirty = true;
}

void Mesh::SetTexCoords(int index, const Array<Vector2>& texCoords)
{
	if (m_texCoords.count(index) > 0)
	{
		m_texCoords[index] = texCoords;
	}
	else
	{
		std::pair<int, Array<Vector2>> pair(index, texCoords);
		m_texCoords.insert(pair);
	}
	m_dirty = true;
}

void Mesh::SetColors(const Array<Vector4>& colors)
{
	m_colors = colors;
	m_dirty = true;
}

void Mesh::SetIndices(const Array<int>& indices)
{
	m_indices = indices;
	m_dirty = true;
}

String Mesh::GetName()
{
	return m_name;
}

void Mesh::RecalculateBounds()
{
	m_bounds = Bounds(m_vertices);
}

void Mesh::UpdateResources()
{
	// Update material resources.
	m_material.Get()->UpdateResources();

	if (m_material.Get() != m_lastUpdatedMaterial)
	{
		m_lastUpdatedMaterial = m_material.Get();
		m_dirty = true;
	}

	std::shared_ptr<Shader> shader = m_material.Get()->GetShader().Get();
	if (shader != m_lastUpdatedShader)
	{
		m_lastUpdatedShader = shader;
		m_dirty = true;
	}

	if (!m_dirty)
	{
		return;
	}

	{
		ScopeLock lock(m_updateResourcesMutex);
		if (m_dirty == false)
		{
			return;
		}

		m_dirty = false;

		// Build description of vertex data.
		if (!m_material.Get()->GetVertexBufferFormat(m_vertexBufferFormat))
		{
			return;
		}

		// Figure out vertex size.
		Array<char> interleavedData;
		interleavedData.resize(m_vertexBufferFormat.vertexSize * m_vertices.size());

		// Get vertex stage so we can build our data.
		const ShaderStage* vertexStage;
		if (!shader->GetStage(GraphicsPipelineStage::Vertex, &vertexStage))
		{
			return;
		}

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
				dataExists = true;
				sourceData = m_vertices.data();
				break;
			}
			case ShaderVertexStreamBinding::Color:
			{
				streamFormat = GraphicsBindingFormat::Float4;
				streamSize = sizeof(Vector4);
				dataExists = (m_colors.size() > 0);
				sourceData = m_colors.data();
				break;
			}
			case ShaderVertexStreamBinding::Normal:
			{
				streamFormat = GraphicsBindingFormat::Float3;
				streamSize = sizeof(Vector3);
				dataExists = (m_normals.size() > 0);
				sourceData = m_normals.data();
				break;
			}
			case ShaderVertexStreamBinding::TexCoord1:
			{
				streamFormat = GraphicsBindingFormat::Float2;
				streamSize = sizeof(Vector2);
				dataExists = (m_texCoords.count(0) > 0);
				sourceData = dataExists ? m_texCoords[0].data() : nullptr;
				break;
			}
			case ShaderVertexStreamBinding::TexCoord2:
			{
				streamFormat = GraphicsBindingFormat::Float2;
				streamSize = sizeof(Vector2);
				dataExists = (m_texCoords.count(1) > 0);
				sourceData = dataExists ? m_texCoords[1].data() : nullptr;
				break;
			}
			case ShaderVertexStreamBinding::TexCoord3:
			{
				streamFormat = GraphicsBindingFormat::Float2;
				streamSize = sizeof(Vector2);
				dataExists = (m_texCoords.count(2) > 0);
				sourceData = dataExists ? m_texCoords[2].data() : nullptr;
				break;
			}
			case ShaderVertexStreamBinding::TexCoord4:
			{
				streamFormat = GraphicsBindingFormat::Float2;
				streamSize = sizeof(Vector2);
				dataExists = (m_texCoords.count(3) > 0);
				sourceData = dataExists ? m_texCoords[3].data() : nullptr;
				break;
			}
			case ShaderVertexStreamBinding::Internal:
			{
				m_logger->WriteWarning(LogCategory::Resources, "[%-30s ] Binding %s has internal format, model cannot bind it.", m_name.c_str(), stream.Name.c_str());
				break;
			}
			}

			if (streamFormat != stream.Format)
			{
				String expectedFormat = EnumToString<GraphicsBindingFormat>(stream.Format);
				String actualFormat = EnumToString<GraphicsBindingFormat>(streamFormat);
				m_logger->WriteWarning(LogCategory::Resources, "[%-30s] Binding %s has binded format %s that is not same as internal format %s. Data may not be delivered correctly to the vertex shader.", m_name.c_str(), stream.Name.c_str(), expectedFormat.c_str(), actualFormat.c_str());
			}

			if (!dataExists)
			{
				String bindingString = EnumToString<ShaderVertexStreamBinding>(stream.BindTo);
				m_logger->WriteWarning(LogCategory::Resources, "[%-30s] Binding %s binds to data %s that is not available, zero'd data will be used.", m_name.c_str(), stream.Name.c_str(), bindingString.c_str());
			}

			// Copy all vertex information.
			for (int vertIndex = 0; vertIndex < m_vertices.size(); vertIndex++)
			{
				char* destination = (interleavedData.data() + (vertIndex * m_vertexBufferFormat.vertexSize)) + bindingOffset;
				if (dataExists)
				{
					char* source = (char*)sourceData + (vertIndex * streamSize);
					memcpy(destination, source, streamSize);
				}
				else
				{
					memset(destination, 0, streamSize);
				}
			}
			bindingOffset += streamSize;
		}

		// Build index buffer.
		m_indexBuffer = m_graphics->CreateIndexBuffer(StringFormat("%s Index Buffer", m_name.c_str()), sizeof(int), (int)m_indices.size());
		m_indexBuffer->Stage((void*)m_indices.data(), 0, (int)m_indices.size() * sizeof(int));

		// Build vertex buffer.
		m_vertexBuffer = m_graphics->CreateVertexBuffer(StringFormat("%s Vertex Buffer", m_name.c_str()), m_vertexBufferFormat, (int)m_vertices.size());
		m_vertexBuffer->Stage((void*)interleavedData.data(), 0, (int)interleavedData.size());

		m_renderer->QueueRenderCommand(RenderCommandStage::PreRender, [=](std::shared_ptr<IGraphicsCommandBuffer> buffer) {
			buffer->Upload(m_vertexBuffer);
			buffer->Upload(m_indexBuffer);
		});
	}
}