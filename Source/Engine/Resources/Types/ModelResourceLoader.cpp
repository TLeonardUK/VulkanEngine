#include "Engine/Resources/Types/ModelResourceLoader.h"
#include "Engine/Resources/Types/Model.h"
#include "Engine/Resources/Types/Texture.h"
#include "Engine/Resources/Types/Shader.h"
#include "Engine/Resources/Types/Material.h"
#include "Engine/Resources/Resource.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Engine/Logging.h"
#include "Engine/Graphics/Graphics.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <streambuf>
#include <istream>

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

	Array<tinyobj::index_t> indices;
	Array<tinyobj::real_t> vertices;
	Array<tinyobj::real_t> normals;
	Array<tinyobj::real_t> texcoords;
	Array<tinyobj::real_t> colors;
};

struct ModelLoadState
{
	Array<MeshLoadState> meshes;
};

ModelResourceLoader::ModelResourceLoader(std::shared_ptr<Logger> logger, std::shared_ptr<IGraphics> graphics)
	: m_logger(logger)
	, m_graphics(graphics)
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
		loader->meshes[loader->meshes.size() - 1].vertices.push_back(x);
		loader->meshes[loader->meshes.size() - 1].vertices.push_back(y);
		loader->meshes[loader->meshes.size() - 1].vertices.push_back(z);
		loader->meshes[loader->meshes.size() - 1].vertices.push_back(w);
	};
	callbacks.normal_cb = [](void *user_data, tinyobj::real_t x, tinyobj::real_t y, tinyobj::real_t z) {
		ModelLoadState* loader = (ModelLoadState*)user_data;
		loader->meshes[loader->meshes.size() - 1].normals.push_back(x);
		loader->meshes[loader->meshes.size() - 1].normals.push_back(y);
		loader->meshes[loader->meshes.size() - 1].normals.push_back(z);
	};
	callbacks.texcoord_cb = [](void *user_data, tinyobj::real_t x, tinyobj::real_t y, tinyobj::real_t z) {
		ModelLoadState* loader = (ModelLoadState*)user_data;
		loader->meshes[loader->meshes.size() - 1].texcoords.push_back(x);
		loader->meshes[loader->meshes.size() - 1].texcoords.push_back(y);
		loader->meshes[loader->meshes.size() - 1].texcoords.push_back(z);
	};
	callbacks.index_cb = [](void *user_data, tinyobj::index_t *indices, int num_indices) {
		ModelLoadState* loader = (ModelLoadState*)user_data;
		for (int i = 0; i < num_indices; i++)
		{
			loader->meshes[loader->meshes.size() - 1].indices.push_back(indices[i]);
		}
	};
	callbacks.usemtl_cb = [](void *user_data, const char *name, int material_id) {
		ModelLoadState* loader = (ModelLoadState*)user_data;
		loader->meshes[loader->meshes.size() - 1].material = name;
	};
	callbacks.mtllib_cb = nullptr;
	callbacks.group_cb = nullptr;
	callbacks.object_cb = [](void *user_data, const char *name) {
		ModelLoadState* loader = (ModelLoadState*)user_data;
		MeshLoadState state;
		state.name = name;
		loader->meshes.push_back(state);
	};

	ModelLoadState objectData;
	if (!tinyobj::LoadObjWithCallback(in, callbacks, &objectData, nullptr, &err))
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Failed to load model, possibly invalid or corrupt?.", resource->Path.c_str());
		return nullptr;
	}

	// Load in all materials if they exist.
	std::map<String, ResourcePtr<Material>> materialMap;

	if (jsonValue.count("ModelMapping") > 0)
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

			materialMap.insert(materialName, material);
		}
	}

	// Create model.
	std::shared_ptr<Model> model = std::make_shared<Model>();

	for (int i = 0; i < objectData.meshes.size(); i++)
	{
		MeshLoadState& mesh = objectData.meshes[i];
		
		ResourcePtr<Material> material = manager->GetDefault<Material>();

		if (materialMap.count(mesh.material) > 0)
		{
			m_logger->WriteWarning(LogCategory::Resources, "[%-30s] Could not map material %s in model to any declared material mappings. Using default material.", resource->Path.c_str(), mesh.material.c_str());
		}
		else
		{
			material = materialMap[mesh.material]
		}

		/*std::shared_ptr<ModelMesh> mesh = model->CreateMesh();
		mesh->SetMaterial();
		mesh->SetVertices();
		mesh->SetIndices();
		mesh->SetNormals();
		mesh->SetTexCoords();
		mesh->Regenerate();
		*/
	}
}

