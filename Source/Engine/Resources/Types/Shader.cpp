#include "Engine/Resources/Types/Shader.h"

#include "Engine/Utilities/EnumImplementation.h"

enum_begin_implementation(ShaderVertexStreamBinding)
#include "Engine/Resources/Types/EShaderVertexStreamBinding.inc"
enum_end_implementation(ShaderVertexStreamBinding)

const char* Shader::Tag = "Shader";

Shader::Shader(const Array<ShaderStage>& stages)
	: m_stages(stages)
{

}