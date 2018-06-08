#include "Engine/Resources/Types/MaterialResourceLoader.h"
#include "Engine/Resources/Types/Material.h"
#include "Engine/Resources/Types/Texture.h"
#include "Engine/Resources/Types/Shader.h"
#include "Engine/Resources/Resource.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Engine/Logging.h"
#include "Engine/Graphics/Graphics.h"

MaterialResourceLoader::MaterialResourceLoader(std::shared_ptr<Logger> logger, std::shared_ptr<IGraphics> graphics)
	: m_logger(logger)
	, m_graphics(graphics)
{
}

String MaterialResourceLoader::GetTag()
{
	return Material::Tag;
}

void MaterialResourceLoader::LoadDefaults(std::shared_ptr<ResourceManager> manager)
{
	m_defaultMaterial = manager->Load<Material>("Engine/Materials/default.json");
}

void MaterialResourceLoader::AssignDefault(std::shared_ptr<ResourceStatus> resource)
{
	resource->DefaultResource = m_defaultMaterial.Get();
}

std::shared_ptr<IResource> MaterialResourceLoader::Load(std::shared_ptr<ResourceManager> manager, std::shared_ptr<ResourceStatus> resource, json& jsonValue)
{
	if (jsonValue.count("ShaderPath") == 0)
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Material does not include required paramater 'ShaderPath'.", resource->Path.c_str());
		return nullptr;
	}

	ResourcePtr<Shader> shader = manager->Load<Shader>(jsonValue["ShaderPath"]);
	Array<MaterialBinding> bindings;

	if (jsonValue.count("Bindings") != 0)
	{
		json bindingJson = jsonValue["Bindings"];
		if (!bindingJson.is_object())
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] Material definition parameter 'Bindings' expected to be an object.", resource->Path.c_str());
			return nullptr;
		}

		for (auto iter = bindingJson.begin(); iter != bindingJson.end(); iter++)
		{
			MaterialBinding binding;
			binding.Name = iter.key();

			json bindingJson = iter.value();

			if (bindingJson.count("Format") == 0)
			{
				m_logger->WriteError(LogCategory::Resources, "[%-30s] Material binding '%s' does not include required paramater 'Type'.", resource->Path.c_str(), binding.Name.c_str());
				return nullptr;
			}

			if (bindingJson.count("Value") == 0)
			{
				m_logger->WriteError(LogCategory::Resources, "[%-30s] Material binding '%s' does not include required paramater 'Value'.", resource->Path.c_str(), binding.Name.c_str());
				return nullptr;
			}

			String formatString = bindingJson["Format"];
			if (!StringToEnum<GraphicsBindingFormat>(formatString, binding.Format))
			{
				m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised binding format.", resource->Path.c_str(), formatString.c_str());
				return false;
			}

			json valueJson = bindingJson["Value"];

			int valueCount = GetValueCountForGraphicsBindingFormat(binding.Format);
			if (valueCount == 1)
			{
				if (valueJson.is_array())
				{
					m_logger->WriteError(LogCategory::Resources, "[%-30s] Expected singular value for value of binding %s.", resource->Path.c_str(), binding.Name.c_str());
					return false;
				}
			}
			else
			{
				if (!valueJson.is_array())
				{
					m_logger->WriteError(LogCategory::Resources, "[%-30s] Expected array for value of binding %s.", resource->Path.c_str(), binding.Name.c_str());
					return false;
				}
				if (valueJson.size() != valueCount)
				{
					m_logger->WriteError(LogCategory::Resources, "[%-30s] Expected array of length %i for binding %s.", resource->Path.c_str(), valueCount, binding.Name.c_str());
					return false;
				}
			}

			Array<json> values;
			for (auto iter = valueJson.begin(); iter != valueJson.end(); iter++)
			{
				values.push_back(*iter);
			}

			if (binding.Format == GraphicsBindingFormat::Texture)
			{
				binding.Value_Texture = manager->Load<Texture>(values[0]);
			}
			else
			{
				binding.ParseJsonValue(values);
			}

			bindings.push_back(binding);
		}
	}

	return std::make_shared<Material>(shader, bindings);
}

