#include "Pch.h"

#include "Engine/Resources/Types/ModelResourceLoader.h"
#include "Engine/Resources/Types/Model.h"
#include "Engine/Resources/Types/Texture.h"
#include "Engine/Resources/Types/Shader.h"
#include "Engine/Resources/Types/Material.h"
#include "Engine/Resources/Resource.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Rendering/Renderer.h"
#include "Engine/Engine/Logging.h"
#include "Engine/Graphics/Graphics.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

struct membuf : std::streambuf 
{
	membuf(char const* base, size_t size) 
	{
		char* p(const_cast<char*>(base));
		this->setg(p, p, p + size);
	}
};

struct imemstream : virtual membuf, std::istream 
{
	imemstream(char const* base, size_t size)
		: membuf(base, size)
		, std::istream(static_cast<std::streambuf*>(this)) 
	{
	}
};

struct MeshLoadState
{
	String name;
	String material;

	Array<tinyobj::index_t> tmpIndices;

	Array<int> indices;
	Array<Vector3> vertices;
	Array<Vector3> normals;
	Array<Vector2> texcoords;
};

struct ModelLoadState
{
	String name;
	Array<MeshLoadState> meshes;

	Array<Vector3> tmpVertices;
	Array<Vector3> tmpNormals;
	Array<Vector3> tmpTexcoords;

public:
	MeshLoadState& GetCurrentMesh()
	{
		if (meshes.size() == 0)
		{
			MeshLoadState state;
			state.name = "Default";
			state.material = "";
			meshes.push_back(state);
		}

		return meshes[meshes.size() - 1];
	}
};

ModelResourceLoader::ModelResourceLoader(std::shared_ptr<Logger> logger, std::shared_ptr<IGraphics> graphics, std::shared_ptr<Renderer> renderer)
	: m_logger(logger)
	, m_graphics(graphics)
	, m_renderer(renderer)
{
}

String ModelResourceLoader::GetTag()
{
	return Model::Tag;
}

void ModelResourceLoader::LoadDefaults(std::shared_ptr<ResourceManager> manager)
{
	m_defaultModel = manager->Load<Model>("Engine/Models/default.json");
}

void ModelResourceLoader::AssignDefault(std::shared_ptr<ResourceStatus> resource)
{
	resource->DefaultResource = m_defaultModel.Get();
}

std::shared_ptr<IResource> ModelResourceLoader::Load(std::shared_ptr<ResourceManager> manager, std::shared_ptr<ResourceStatus> resource, json& jsonValue)
{
	if (jsonValue.count("ModelPath") == 0)
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Model does not include required paramater 'ModelPath'.", resource->Path.c_str());
		return nullptr;
	}

	String modelPath = jsonValue["ModelPath"];

	// Read model bytes.
	Array<char> buffer;
	if (!manager->ReadResourceBytes(modelPath.c_str(), buffer))
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Failed to read path", modelPath.c_str());
		return nullptr;
	}

	// Parse the model.
	std::string err;

	imemstream in(buffer.data(), buffer.size());

	tinyobj::callback_t callbacks;
	callbacks.vertex_cb = [](void *user_data, tinyobj::real_t x, tinyobj::real_t y, tinyobj::real_t z, tinyobj::real_t w) {
		ModelLoadState* loader = (ModelLoadState*)user_data;
		MeshLoadState& mesh = loader->GetCurrentMesh();

		// Convert to our texture coordinate system.
		loader->tmpVertices.push_back(Vector3(-x / w, y / w, z / w));
	};
	callbacks.normal_cb = [](void *user_data, tinyobj::real_t x, tinyobj::real_t y, tinyobj::real_t z) {
		ModelLoadState* loader = (ModelLoadState*)user_data;
		MeshLoadState& mesh = loader->GetCurrentMesh();

		// Convert to our texture coordinate system.
		loader->tmpNormals.push_back(Vector3(-x, y, z));
	};
	callbacks.texcoord_cb = [](void *user_data, tinyobj::real_t x, tinyobj::real_t y, tinyobj::real_t z) {
		ModelLoadState* loader = (ModelLoadState*)user_data;
		MeshLoadState& mesh = loader->GetCurrentMesh();

		loader->tmpTexcoords.push_back(Vector3(x, y, z));
	};
	callbacks.index_cb = [](void *user_data, tinyobj::index_t *indices, int num_indices) {
		ModelLoadState* loader = (ModelLoadState*)user_data;
		MeshLoadState& mesh = loader->GetCurrentMesh();

		for (int i = 0; i < num_indices; i++)
		{
			if (indices[i].vertex_index < 0)
			{
				indices[i].vertex_index = (int)loader->tmpVertices.size() + indices[i].vertex_index;
			}
			if (indices[i].normal_index < 0)
			{
				indices[i].normal_index = (int)loader->tmpNormals.size() + indices[i].normal_index;
			}
			if (indices[i].texcoord_index < 0)
			{
				indices[i].texcoord_index = (int)loader->tmpTexcoords.size() + indices[i].texcoord_index;
			}
		}

		if (num_indices >= 3)
		{
			for (int i = 1; i < num_indices - 1; i++)
			{
				mesh.tmpIndices.push_back(indices[0]);
				mesh.tmpIndices.push_back(indices[i]);
				mesh.tmpIndices.push_back(indices[i + 1]);
			}
		}
		else
		{
			// We only support triangles right now.
			assert(false);
		}
	};
	callbacks.usemtl_cb = [](void *user_data, const char *name, int material_id) {
		ModelLoadState* loader = (ModelLoadState*)user_data;

		MeshLoadState& mesh = loader->GetCurrentMesh();
		
		if (!mesh.material.empty() && mesh.material != name)
		{
			MeshLoadState state;
			state.name = StringFormat("%s (Sub Mesh - %s)", loader->name.c_str(), name);
			state.material = name;
			loader->meshes.push_back(state);
		}
		else
		{
			mesh.material = name;

		}
	};
	callbacks.mtllib_cb = nullptr;
	callbacks.group_cb = nullptr;
	callbacks.object_cb = [](void *user_data, const char *name) {
		ModelLoadState* loader = (ModelLoadState*)user_data;
		
		MeshLoadState state;
		state.name = StringFormat("%s (Sub Mesh - %s)", loader->name.c_str(), name);
		state.material = "";

		loader->meshes.push_back(state);
	};

	ModelLoadState objectData;
	objectData.name = resource->Path;

	{
		ProfileScope scope(ProfileColors::Streaming, "Loading obj file");

		if (!tinyobj::LoadObjWithCallback(in, callbacks, &objectData, nullptr, &err))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] Failed to load model, possibly invalid or corrupt?.", resource->Path.c_str());
			return nullptr;
		}
	}

	// De-index normals/texcoords/positions.
	{
		ProfileScope scope(ProfileColors::Streaming, "De-indexing streams");

		for (int meshIndex = 0; meshIndex < objectData.meshes.size(); meshIndex++)
		{
			MeshLoadState& mesh = objectData.meshes[meshIndex];
			mesh.vertices.reserve(mesh.tmpIndices.size());
			mesh.normals.reserve(mesh.tmpIndices.size());
			mesh.texcoords.reserve(mesh.tmpIndices.size());
			mesh.indices.reserve(mesh.tmpIndices.size());

			for (int index = 0; index < mesh.tmpIndices.size(); index++)
			{
				tinyobj::index_t tinyIndex = mesh.tmpIndices[index];

				mesh.vertices.push_back(objectData.tmpVertices[tinyIndex.vertex_index - 1]);
				if (!objectData.tmpNormals.empty())
				{
					if (tinyIndex.normal_index == 0)
					{
						mesh.normals.push_back(Vector3(0.0f, 0.0f, 0.0f));
					}
					else
					{
						mesh.normals.push_back(objectData.tmpNormals[tinyIndex.normal_index - 1]);
					}
				}
				if (!objectData.tmpTexcoords.empty())
				{
					if (tinyIndex.texcoord_index == 0)
					{
						mesh.texcoords.push_back(Vector2(0.0f, 0.0f));
					}
					else
					{
						mesh.texcoords.push_back(Vector2(objectData.tmpTexcoords[tinyIndex.texcoord_index - 1].x, 1.0f - objectData.tmpTexcoords[tinyIndex.texcoord_index - 1].y)); // no support for 3d texture coordinates (yet!)
					}
				}
				mesh.indices.push_back((int)mesh.indices.size());
			}
		}
	}

	// Load in all materials if they exist.
	std::map<String, ResourcePtr<Material>> materialMap;

	if (jsonValue.count("MaterialMapping") > 0)
	{
		json jsonMapping = jsonValue["MaterialMapping"];
		if (!jsonMapping.is_object())
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] Model definition parameter 'MaterialMappings' expected to be an object.", resource->Path.c_str());
			return nullptr;
		}

		for (auto iter = jsonMapping.begin(); iter != jsonMapping.end(); iter++)
		{
			String materialName = iter.key();
			String materialPath = iter.value();

			ResourcePtr<Material> material = manager->Load<Material>(materialPath);
			manager->AddResourceDependency(resource, material);

			std::pair<String, ResourcePtr<Material>> pair(materialName, material);

			materialMap.insert(pair);
		}
	}

	// Create model.
	std::shared_ptr<Model> model = std::make_shared<Model>(m_logger, m_renderer, m_graphics, resource->Path);

	for (int i = 0; i < objectData.meshes.size(); i++)
	{
		ProfileScope scope(ProfileColors::Streaming, "Creating mesh");

		MeshLoadState& mesh = objectData.meshes[i];
		if (mesh.material.empty())
		{
			mesh.material = "Default";
		}

		ResourcePtr<Material> material = manager->GetDefault<Material>();

		if (materialMap.count(mesh.material) == 0)
		{
			m_logger->WriteWarning(LogCategory::Resources, "[%-30s] Could not map material %s in model to any declared material mappings. Using default material.", resource->Path.c_str(), mesh.material.c_str());
		}
		else
		{
			material = materialMap[mesh.material];
		}

		if (mesh.vertices.size() == 0 || mesh.indices.size() == 0)
		{
			continue;
		}

		std::shared_ptr<Mesh> modelMesh = model->CreateMesh();
		modelMesh->SetMaterial(material);
		modelMesh->SetVertices(mesh.vertices);
		modelMesh->SetIndices(mesh.indices);
		if (mesh.normals.size() > 0)
		{
			modelMesh->SetNormals(mesh.normals);
		}
		if (mesh.texcoords.size() > 0)
		{
			modelMesh->SetTexCoords(0, mesh.texcoords);
		}
		modelMesh->RecalculateBounds();
	}

	manager->AddResourceLoadedCallback(resource, [=]() {
		m_renderer->QueueRenderCommand(RenderCommandStage::Global_PreRender, [=](std::shared_ptr<IGraphicsCommandBuffer> buffer) {
			model->UpdateResources();
		});
	});

	return model;
}

