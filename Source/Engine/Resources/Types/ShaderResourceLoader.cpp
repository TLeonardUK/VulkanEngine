#include "Pch.h"

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

bool ShaderResourceLoader::LoadBindings(Array<ShaderBinding>& bindings, json& jsonData, std::shared_ptr<ResourceStatus> resource)
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
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised binding type.", resource->Path.c_str(), bindingTypeName.c_str());
			return false;
		}

		if (bindingJson.count("Binding") == 0)
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] Shader binding '%s' does not include required paramater 'Binding'.", resource->Path.c_str(), bindingName.c_str());
			return false;
		}

		if (bindingJson.count("Set") == 0)
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] Shader binding '%s' does not include required paramater 'Set'.", resource->Path.c_str(), bindingName.c_str());
			return false;
		}

		if (bindingJson.count("Frequency") == 0)
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] Shader binding '%s' does not include required paramater 'Frequency'.", resource->Path.c_str(), bindingName.c_str());
			return false;
		}

		String freqencyName = bindingJson["Frequency"];
		if (!StringToEnum<GraphicsBindingFrequency>(freqencyName, binding.Frequency))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised frequency type.", resource->Path.c_str(), freqencyName.c_str());
			return false;
		}

		if (binding.Type != GraphicsBindingType::UniformBufferObject)
		{
			if (bindingJson.count("BindTo") == 0)
			{
				m_logger->WriteError(LogCategory::Resources, "[%-30s] Shader binding '%s' does not include required paramater 'BindTo'.", resource->Path.c_str(), bindingName.c_str());
				return false;
			}

			String bindToName = bindingJson["BindTo"];
			binding.BindTo = bindToName;
			binding.BindToHash = CalculateRenderPropertyHash(binding.BindTo);
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
				if (fieldJson.count("Location") == 0)
				{
					m_logger->WriteError(LogCategory::Resources, "[%-30s] Shader binding field '%s' does not include required paramater 'Location'.", resource->Path.c_str(), fieldName.c_str());
					return false;
				}

				UniformBufferLayoutField fieldBinding;
				fieldBinding.Name = fieldName;
				fieldBinding.BindTo = fieldJson["BindTo"];
				fieldBinding.BindToHash = CalculateRenderPropertyHash(fieldBinding.BindTo);
				fieldBinding.Location = fieldJson["Location"];

				String fieldFormat = fieldJson["Format"];

				if (!StringToEnum<GraphicsBindingFormat>(fieldFormat, fieldBinding.Format))
				{
					m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised binding format.", resource->Path.c_str(), fieldFormat.c_str());
					return false;
				}

				binding.UniformBufferLayout.Fields.push_back(fieldBinding);
			}
			
			// Sort by location in struct.
			std::sort(binding.UniformBufferLayout.Fields.begin(), binding.UniformBufferLayout.Fields.end(),
				[](const UniformBufferLayoutField& a, const UniformBufferLayoutField& b) -> bool
			{
				return a.Location < b.Location;
			});

			// Ensure fields are sequential.
			for (int i = 0; i < binding.UniformBufferLayout.Fields.size(); i++)
			{
				if (binding.UniformBufferLayout.Fields[i].Location != i)
				{
					m_logger->WriteError(LogCategory::Resources, "[%-30s] Shader binding field '%s' is not sequential, expected location %i.", resource->Path.c_str(), binding.UniformBufferLayout.Fields[i].Name, i);
					return false;
				}
			}

			binding.UniformBufferLayout.CalculateHashCode();
		}

		binding.Binding = bindingJson["Binding"];
		binding.Set = bindingJson["Set"];
		binding.Name = bindingName;		

		bindings.push_back(binding);
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

	// Sort by location in struct.
	std::sort(stage.Streams.begin(), stage.Streams.end(),
		[](const ShaderVertexStream& a, const ShaderVertexStream& b) -> bool
	{
		return a.Location < b.Location;
	});


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

		shaderFile += m_graphics->GetShaderPathPostfix();

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

	Array<ShaderBinding> shaderBindings;

	// Parse bindings.
	if (jsonValue.count("Bindings"))
	{
		json bindingsJson = jsonValue["Bindings"];
		if (!bindingsJson.is_object())
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] Expected 'Bindings' parameter to be an object.", resource->Path.c_str());
			return nullptr;
		}

		if (!LoadBindings(shaderBindings, bindingsJson, resource))
		{
			return nullptr;
		}
	}
	
	// Parse pipeline description.
	GraphicsPipelineSettings pipelineDescription;

	for (auto& stage : shaderStages)
	{
		pipelineDescription.ShaderStages[(int)stage.Stage] = stage.Shader;
	}

	if (jsonValue.count("PolygonMode"))
	{
		String value = jsonValue["PolygonMode"];
		if (!StringToEnum<GraphicsPolygonMode>(value, pipelineDescription.PolygonMode))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised polygon mode.", resource->Path.c_str(), value.c_str());
			return false;
		}
	}
	if (jsonValue.count("PrimitiveType"))
	{
		String value = jsonValue["PrimitiveType"];
		if (!StringToEnum<GraphicsPrimitiveType>(value, pipelineDescription.PrimitiveType))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised primitive type.", resource->Path.c_str(), value.c_str());
			return false;
		}
	}
	if (jsonValue.count("CullMode"))
	{
		String value = jsonValue["CullMode"];
		if (!StringToEnum<GraphicsCullMode>(value, pipelineDescription.CullMode))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised culling mode.", resource->Path.c_str(), value.c_str());
			return false;
		}
	}
	if (jsonValue.count("FaceWindingOrder"))
	{
		String value = jsonValue["FaceWindingOrder"];
		if (!StringToEnum<GraphicsFaceWindingOrder>(value, pipelineDescription.FaceWindingOrder))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised face winding order.", resource->Path.c_str(), value.c_str());
			return false;
		}
	}
	if (jsonValue.count("DepthTestEnabled"))
	{
		pipelineDescription.DepthTestEnabled = jsonValue["DepthTestEnabled"];
	}
	if (jsonValue.count("DepthWriteEnabled"))
	{
		pipelineDescription.DepthTestEnabled = jsonValue["DepthWriteEnabled"];
	}
	if (jsonValue.count("DepthCompareOp"))
	{
		String value = jsonValue["DepthCompareOp"];
		if (!StringToEnum<GraphicsDepthCompareOp>(value, pipelineDescription.DepthCompareOp))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised depth compare op.", resource->Path.c_str(), value.c_str());
			return false;
		}
	}
	if (jsonValue.count("DepthBiasEnabled"))
	{
		pipelineDescription.DepthBiasEnabled = jsonValue["DepthBiasEnabled"];
	}
	if (jsonValue.count("LineWidth"))
	{
		pipelineDescription.LineWidth = jsonValue["LineWidth"];
	}
	if (jsonValue.count("DepthBiasConstant"))
	{
		pipelineDescription.DepthBiasConstant = jsonValue["DepthBiasConstant"];
	}
	if (jsonValue.count("DepthBiasClamp"))
	{
		pipelineDescription.DepthBiasClamp = jsonValue["DepthBiasClamp"];
	}
	if (jsonValue.count("DepthBiasSlopeFactor"))
	{
		pipelineDescription.DepthBiasSlopeFactor = jsonValue["DepthBiasSlopeFactor"];
	}
	if (jsonValue.count("StencilTestEnabled"))
	{
		pipelineDescription.StencilTestEnabled = jsonValue["StencilTestEnabled"];
	}
	if (jsonValue.count("StencilTestReference"))
	{
		pipelineDescription.StencilTestReference = jsonValue["StencilTestReference"];
	}
	if (jsonValue.count("StencilTestReadMask"))
	{
		pipelineDescription.StencilTestReadMask = jsonValue["StencilTestReadMask"];
	}
	if (jsonValue.count("StencilTestWriteMask"))
	{
		pipelineDescription.StencilTestWriteMask = jsonValue["StencilTestWriteMask"];
	}
	if (jsonValue.count("StencilTestCompareOp"))
	{
		String value = jsonValue["StencilTestCompareOp"];
		if (!StringToEnum<GraphicsStencilTestCompareOp>(value, pipelineDescription.StencilTestCompareOp))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised stencil test compare op.", resource->Path.c_str(), value.c_str());
			return false;
		}
	}
	if (jsonValue.count("StencilTestPassOp"))
	{
		String value = jsonValue["StencilTestPassOp"];
		if (!StringToEnum<GraphicsStencilTestOp>(value, pipelineDescription.StencilTestPassOp))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised stencil test op.", resource->Path.c_str(), value.c_str());
			return false;
		}
	}
	if (jsonValue.count("StencilTestFailOp"))
	{
		String value = jsonValue["StencilTestFailOp"];
		if (!StringToEnum<GraphicsStencilTestOp>(value, pipelineDescription.StencilTestFailOp))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised stencil test op.", resource->Path.c_str(), value.c_str());
			return false;
		}
	}
	if (jsonValue.count("StencilTestZFailOp"))
	{
		String value = jsonValue["StencilTestZFailOp"];
		if (!StringToEnum<GraphicsStencilTestOp>(value, pipelineDescription.StencilTestZFailOp))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised stencil test op.", resource->Path.c_str(), value.c_str());
			return false;
		}
	}
	if (jsonValue.count("BlendEnabled"))
	{
		pipelineDescription.BlendEnabled = jsonValue["BlendEnabled"];
	}
	if (jsonValue.count("SrcColorBlendFactor"))
	{
		String value = jsonValue["SrcColorBlendFactor"];
		if (!StringToEnum<GraphicsBlendFactor>(value, pipelineDescription.SrcColorBlendFactor))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised blend factor.", resource->Path.c_str(), value.c_str());
			return false;
		}
	}
	if (jsonValue.count("DstColorBlendFactor"))
	{
		String value = jsonValue["DstColorBlendFactor"];
		if (!StringToEnum<GraphicsBlendFactor>(value, pipelineDescription.DstColorBlendFactor))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised blend factor.", resource->Path.c_str(), value.c_str());
			return false;
		}
	}
	if (jsonValue.count("SrcAlphaBlendFactor"))
	{
		String value = jsonValue["SrcAlphaBlendFactor"];
		if (!StringToEnum<GraphicsBlendFactor>(value, pipelineDescription.SrcAlphaBlendFactor))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised blend factor.", resource->Path.c_str(), value.c_str());
			return false;
		}
	}
	if (jsonValue.count("DstAlphaBlendFactor"))
	{
		String value = jsonValue["DstAlphaBlendFactor"];
		if (!StringToEnum<GraphicsBlendFactor>(value, pipelineDescription.DstAlphaBlendFactor))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised blend factor.", resource->Path.c_str(), value.c_str());
			return false;
		}
	}
	if (jsonValue.count("ColorBlendOp"))
	{
		String value = jsonValue["ColorBlendOp"];
		if (!StringToEnum<GraphicsBlendOp>(value, pipelineDescription.ColorBlendOp))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised blend op.", resource->Path.c_str(), value.c_str());
			return false;
		}
	}
	if (jsonValue.count("AlphaBlendOp"))
	{
		String value = jsonValue["AlphaBlendOp"];
		if (!StringToEnum<GraphicsBlendOp>(value, pipelineDescription.AlphaBlendOp))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised blend op.", resource->Path.c_str(), value.c_str());
			return false;
		}
	}

	// Parse frame buffer target.
	FrameBufferTarget target = FrameBufferTarget::GBuffer;
	if (jsonValue.count("FrameBuffer"))
	{
		String value = jsonValue["FrameBuffer"];
		if (!StringToEnum<FrameBufferTarget>(value, target))
		{
			m_logger->WriteError(LogCategory::Resources, "[%-30s] %s is not a recognised frame buffer target.", resource->Path.c_str(), value.c_str());
			return false;
		}
	}

	return std::make_shared<Shader>(shaderStages, shaderBindings, pipelineDescription, target);
}