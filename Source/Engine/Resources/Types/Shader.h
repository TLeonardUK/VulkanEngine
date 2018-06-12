#pragma once

#include "Engine/Resources/Types/MaterialPropertyCollection.h"
#include "Engine/Resources/ResourceLoader.h"
#include "Engine/Resources/Resource.h"

#include "Engine/Graphics/GraphicsPipeline.h"

#include "Engine/Utilities/Enum.h"

class IGraphicsShader;
class TextureResourceLoader;

enum_begin_declaration(ShaderVertexStreamBinding)
#include "Engine/Resources/Types/EShaderVertexStreamBinding.inc"
enum_end_declaration(ShaderVertexStreamBinding)

struct ShaderVertexStream
{
public:
	String Name;
	GraphicsBindingFormat Format;
	int Location;
	ShaderVertexStreamBinding BindTo;

};

struct ShaderBindingField
{
public:
	String Name;
	GraphicsBindingFormat Format;
	String BindTo;
	MaterialPropertyHash BindToHash;
	int Location;

};

struct ShaderBinding
{
public:
	String Name;
	GraphicsBindingType Type;
	int Binding;
	String BindTo;
	MaterialPropertyHash BindToHash;
	Array<ShaderBindingField> Fields;

public:
	const int UniformPaddingAlignment = 16;

	int GetUniformBufferSize() const;
	int GetUniformBufferFieldOffset(int fieldIndex) const;

};

struct ShaderStage
{
public:
	GraphicsPipelineStage Stage;
	std::shared_ptr<IGraphicsShader> Shader;
	Array<ShaderVertexStream> Streams;

};

class Shader
	: public IResource
{
private:
	Array<ShaderStage> m_stages;
	Array<ShaderBinding> m_bindings;

public:
	static const char* Tag;

	Shader(const Array<ShaderStage>& stages, const Array<ShaderBinding>& bindings);

	bool GetStage(GraphicsPipelineStage stage, const ShaderStage** result);
	const Array<ShaderStage>& GetStages();
	const Array<ShaderBinding>& GetBindings();

};
