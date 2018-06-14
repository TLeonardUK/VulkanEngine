#include "Engine/Resources/Types/Shader.h"
#include "Engine/Graphics/GraphicsEnums.h"

#include "Engine/Utilities/EnumImplementation.h"

enum_begin_implementation(ShaderVertexStreamBinding)
#include "Engine/Resources/Types/EShaderVertexStreamBinding.inc"
enum_end_implementation(ShaderVertexStreamBinding)

const char* Shader::Tag = "Shader";

Shader::Shader(const Array<ShaderStage>& stages, const Array<ShaderBinding>& bindings, const GraphicsPipelineSettings& pipelineDescription)
	: m_stages(stages)
	, m_bindings(bindings)
	, m_pipelineDescription(pipelineDescription)
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

int ShaderBinding::GetUniformBufferSize() const
{
	int size = 0;
	int baseAlignment = 0;

	for (auto& field : Fields)
	{
		int alignment = GetAlignmentForGraphicsBindingFormat(field.Format);
		if (baseAlignment == 0)
		{
			baseAlignment = alignment;
		}

		size += (size % alignment);
		size += GetByteSizeForGraphicsBindingFormat(field.Format);
	}
	
	size += (size % baseAlignment);

	return size;
}

int ShaderBinding::GetUniformBufferFieldOffset(int fieldIndex) const
{
	int size = 0;
	for (int i = 0; i < Fields.size(); i++)
	{
		auto& field = Fields[i];

		int alignment = GetAlignmentForGraphicsBindingFormat(field.Format);
		size += (size % alignment);

		if (i == fieldIndex)
		{
			return size;
		}

		size += GetByteSizeForGraphicsBindingFormat(field.Format);
	}
	return size;
}

const GraphicsPipelineSettings& Shader::GetPipelineDescription()
{
	return m_pipelineDescription;
}