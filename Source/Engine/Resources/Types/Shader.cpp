#include "Pch.h"

#include "Engine/Resources/Types/Shader.h"
#include "Engine/Graphics/GraphicsEnums.h"
#include "Engine/Utilities/Statistic.h"

#include "Engine/Utilities/EnumImplementation.h"

enum_begin_implementation(ShaderVertexStreamBinding)
#include "Engine/Resources/Types/EShaderVertexStreamBinding.inc"
enum_end_implementation(ShaderVertexStreamBinding)

const char* Shader::Tag = "Shader";

Statistic Stat_Resources_ShaderCount("Resources/Shader Count", StatisticFrequency::Persistent, StatisticFormat::Integer);

Shader::Shader(const Array<ShaderStage>& stages, const Array<ShaderBinding>& bindings, const GraphicsPipelineSettings& pipelineDescription, FrameBufferTarget target)
	: m_stages(stages)
	, m_bindings(bindings)
	, m_pipelineDescription(pipelineDescription)
	, m_target(target)
{
	Stat_Resources_ShaderCount.Add(1);
}

Shader::~Shader()
{
	Stat_Resources_ShaderCount.Add(-1);
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