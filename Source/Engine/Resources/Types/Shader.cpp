#include "Pch.h"

#include "Engine/Resources/Types/Shader.h"
#include "Engine/Graphics/GraphicsEnums.h"

#include "Engine/Utilities/EnumImplementation.h"

enum_begin_implementation(ShaderVertexStreamBinding)
#include "Engine/Resources/Types/EShaderVertexStreamBinding.inc"
enum_end_implementation(ShaderVertexStreamBinding)

const char* Shader::Tag = "Shader";

Shader::Shader(const Array<ShaderStage>& stages, const Array<ShaderBinding>& bindings, const GraphicsPipelineSettings& pipelineDescription, FrameBufferTarget target)
	: m_stages(stages)
	, m_bindings(bindings)
	, m_pipelineDescription(pipelineDescription)
	, m_target(target)
{
}

bool Shader::GetStage(GraphicsPipelineStage stage, const ShaderStage** result)
{
	for (auto& value : m_stages)
	{
		if (value.Stage == stage)
		{
			*result = &value;
			return true;
		}
	}

	return false;
}

const Array<ShaderStage>& Shader::GetStages()
{
	return m_stages;
}

const Array<ShaderBinding>& Shader::GetBindings()
{
	return m_bindings;
}

const GraphicsPipelineSettings& Shader::GetPipelineDescription()
{
	return m_pipelineDescription;
}

FrameBufferTarget Shader::GetTarget()
{
	return m_target;
}