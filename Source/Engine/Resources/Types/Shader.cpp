#include "Engine/Resources/Types/Shader.h"
#include "Engine/Graphics/GraphicsEnums.h"

#include "Engine/Utilities/EnumImplementation.h"

enum_begin_implementation(ShaderVertexStreamBinding)
#include "Engine/Resources/Types/EShaderVertexStreamBinding.inc"
enum_end_implementation(ShaderVertexStreamBinding)

const char* Shader::Tag = "Shader";

Shader::Shader(const Array<ShaderStage>& stages, const Array<ShaderBinding>& bindings)
	: m_stages(stages)
	, m_bindings(bindings)
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
	for (auto& field : Fields)
	{
		size += GetByteSizeForGraphicsBindingFormat(field.Format);
		size += (size % UniformPaddingAlignment);
	}
	return size;
}

int ShaderBinding::GetUniformBufferFieldOffset(int fieldIndex) const
{
	int offset = 0;
	for (int i = 0; i < Fields.size(); i++)
	{
		if (i == fieldIndex)
		{
			return offset;
		}
		offset += GetByteSizeForGraphicsBindingFormat(Fields[i].Format);
		offset += (offset % UniformPaddingAlignment);
	}
	return offset;
}