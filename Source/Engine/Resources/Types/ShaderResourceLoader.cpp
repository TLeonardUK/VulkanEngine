#include "Engine/Resources/Types/ShaderResourceLoader.h"
#include "Engine/Resources/Types/Shader.h"
#include "Engine/Resources/Resource.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/Engine/Logging.h"
#include "Engine/Graphics/Graphics.h"

// todo: parse out pipeline values here as well? blending modes etc? 

ShaderResourceLoader::ShaderResourceLoader(std::shared_ptr<Logger> logger, std::shared_ptr<IGraphics> graphics)
	: m_logger(logger)
	, m_graphics(graphics)
{
}

String ShaderResourceLoader::GetTag()
{
	return Shader::Tag;
}

void ShaderResourceLoader::LoadDefaults(std::shared_ptr<ResourceManager> manager)
{
	m_defaultShader = manager->Load<Shader>("Engine/Shaders/default.json");
}

void ShaderResourceLoader::AssignDefault(std::shared_ptr<ResourceStatus> resource)
{
	resource->DefaultResource = m_defaultShader.Get();
}

bool ShaderResourceLoader::LoadStageBindings(ShaderStage& stage, const String& stageName, json& jsonData, std::shared_ptr<ResourceStatus> resource)
{
	for (auto iter = jsonData.begin(); iter != jsonData.end(); iter++)
	{
		String bindingName = iter.key();
		json bindingJson = iter.value();

		String bindingTypeName = bindingJson["Type"];

		ShaderBinding binding;

		if (bindingJson.count("Type") == 0)
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] Shader binding '%s' does not include required paramater 'Type'.", resource->Path.c_str(), bindingName.c_str());
			return false;
		}
		if (!StringToEnum<GraphicsBindingType>(bindingTypeName, binding.Type))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised binding type type.", resource->Path.c_str(), bindingTypeName.c_str());
			return false;
		}

		if (bindingJson.count("Binding") == 0)
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] Shader binding '%s' does not include required paramater 'Binding'.", resource->Path.c_str(), bindingName.c_str());
			return false;
		}

		if (binding.Type != GraphicsBindingType::UniformBufferObject)
		{
			binding.BindTo = bindingJson["BindTo"];

			if (bindingJson.count("BindTo") == 0)
			{
				m_logger->WriteError(LogCategory::Resources, "[%-30s] Shader binding '%s' does not include required paramater 'BindTo'.", resource->Path.c_str(), bindingName.c_str());
				return false;
			}
		}
		else
		{
			if (bindingJson.count("Fields") == 0)
			{
				m_logger->WriteError(LogCategory::Resources, "[%-30s] Shader binding '%s' does not include required paramater 'Fields', listing contents of UBO.", resource->Path.c_str(), bindingName.c_str());
				return false;
			}

			json fieldsJson = bindingJson["Fields"];
			if (!fieldsJson.is_object())
			{
				m_logger->WriteError(LogCategory::Resources, "[%-30s] Shader definition parameter 'Fields' expected to be an object.", resource->Path.c_str());
				return false;
			}

			for (auto iter = fieldsJson.begin(); iter != fieldsJson.end(); iter++)
			{
				String fieldName = iter.key();
				json fieldJson = iter.value();

				if (fieldJson.count("Format") == 0)
				{
					m_logger->WriteError(LogCategory::Resources, "[%-30s] Shader binding field '%s' does not include required paramater 'Format'.", resource->Path.c_str(), fieldName.c_str());
					return false;
				}
				if (fieldJson.count("BindTo") == 0)
				{
					m_logger->WriteError(LogCategory::Resources, "[%-30s] Shader binding field '%s' does not include required paramater 'BindTo'.", resource->Path.c_str(), fieldName.c_str());
					return false;
				}

				ShaderBindingField fieldBinding;
				fieldBinding.Name = fieldName;
				fieldBinding.BindTo = fieldJson["BindTo"];

				String fieldFormat = fieldJson["Format"];

				if (!StringToEnum<GraphicsBindingFormat>(fieldFormat, fieldBinding.Format))
				{
					m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised binding format.", resource->Path.c_str(), fieldFormat.c_str());
					return false;
				}

				binding.Fields.push_back(fieldBinding);
			}
		}

		binding.Binding = bindingJson["Binding"];
		binding.Name = bindingName;		

		stage.Bindings.push_back(binding);
	}

	return true;
}

bool ShaderResourceLoader::LoadStageStreams(ShaderStage& stage, const String& stageName, json& jsonData, std::shared_ptr<ResourceStatus> resource)
{
	for (auto iter = jsonData.begin(); iter != jsonData.end(); iter++)
	{
		String streamName = iter.key();
		json streamJson = iter.value();

		if (streamJson.count("Format") == 0)
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] Shader vertex stream '%s' does not include required paramater 'Format'.", resource->Path.c_str(), streamName.c_str());
			return false;
		}
		if (streamJson.count("Location") == 0)
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] Shader vertex stream '%s' does not include required paramater 'Location'.", resource->Path.c_str(), streamName.c_str());
			return false;
		}
		if (streamJson.count("BindTo") == 0)
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] Shader vertex stream '%s' does not include required paramater 'BindTo'.", resource->Path.c_str(), streamName.c_str());
			return false;
		}

		ShaderVertexStream stream;
		stream.Name = streamName;
		stream.Location = streamJson["Location"];

		String bindingTypeName = streamJson["Format"];
		String bindingTarget = streamJson["BindTo"];

		if (!StringToEnum<GraphicsBindingFormat>(bindingTypeName, stream.Format))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised stream format.", resource->Path.c_str(), bindingTarget.c_str());
			return false;
		}

		if (!StringToEnum<ShaderVertexStreamBinding>(bindingTarget, stream.BindTo))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised binding target.", resource->Path.c_str(), bindingTarget.c_str());
			return false;
		}

		stage.Streams.push_back(stream);
	}

	return true;
}

std::shared_ptr<IResource> ShaderResourceLoader::Load(std::shared_ptr<ResourceManager> manager, std::shared_ptr<ResourceStatus> resource, json& jsonValue)
{
	if (jsonValue.count("Stages") == 0)
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Shader definition does not include required parameter 'Stages'.", resource->Path.c_str());
		return nullptr;
	}

	Array<ShaderStage> shaderStages;

	json stagesJson = jsonValue["Stages"];
	if (!stagesJson.is_object())
	{
		m_logger->WriteError(LogCategory::Resources, "[%-30s] Shader definition parameter 'Stages' expected to be an object.", resource->Path.c_str());
		return nullptr;
	}

	for (auto iter = stagesJson.begin(); iter != stagesJson.end(); iter++)
	{
		String stageName = iter.key();
		json stageJson = iter.value();

		ShaderStage shaderStage;

		// Replace this with an auto-generated ToEnum parsing.
		if (!StringToEnum<GraphicsPipelineStage>(stageName, shaderStage.Stage))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised pipeline stage.", resource->Path.c_str(), stageName.c_str());
			return nullptr;
		}
		if (!stageJson.is_object())
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] Shader stage '%s' is not valid. Expected json object.", resource->Path.c_str(), stageName.c_str());
			return nullptr;
		}
		if (stageJson.count("ShaderPath") == 0)
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] Shader stage '%s' does not include required paramter 'ShaderFile'.", resource->Path.c_str(), stageName.c_str());
			return nullptr;
		}
		if (stageJson.count("EntryPoint") == 0)
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] Shader stage '%s' does not include required paramter 'EntryPoint'.", resource->Path.c_str(), stageName.c_str());
			return nullptr;
		}

		// Load actual shader file for this stage.
		String shaderFile = stageJson["ShaderPath"];
		String entryPoint = stageJson["EntryPoint"];

		Array<char> shaderBytes;
		if (!manager->ReadResourceBytes(shaderFile, shaderBytes))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] Failed to read path", shaderFile.c_str());
			return nullptr;
		}

		shaderStage.Shader = m_graphics->CreateShader(shaderFile, entryPoint, shaderStage.Stage, shaderBytes);
		if (shaderStage.Shader == nullptr)
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] Failed to load shader file %s.", resource->Path.c_str(), shaderFile.c_str());
			return nullptr;
		}

		// Parse bindings.
		if (stageJson.count("Bindings"))
		{
			json bindingsJson = stageJson["Bindings"];
			if (!bindingsJson.is_object())
			{
				m_logger->WriteError(LogCategory::Resources, "[%-30s] Shader stage '%s' expected 'Bindings' parameter to be an object.", resource->Path.c_str(), stageName.c_str());
				return nullptr;
			}

			if (!LoadStageBindings(shaderStage, stageName, bindingsJson, resource))
			{
				return nullptr;
			}
		}

		// Parse vertex format.
		if (shaderStage.Stage == GraphicsPipelineStage::Vertex)
		{
			json streamsJson = stageJson["VertexStreams"];
			if (!streamsJson.is_object())
			{
				m_logger->WriteError(LogCategory::Resources, "[%-30s] Shader stage '%s' expected 'VertexStreams' parameter to be an object.", resource->Path.c_str(), stageName.c_str());
				return nullptr;
			}

			if (!LoadStageStreams(shaderStage, stageName, streamsJson, resource))
			{
				return nullptr;
			}
		}

		shaderStages.push_back(shaderStage);
	}

	return std::make_shared<Shader>(shaderStages);
}