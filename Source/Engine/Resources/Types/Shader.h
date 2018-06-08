#pragma once

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

};

struct ShaderBinding
{
public:
	String Name;
	GraphicsBindingType Type;
	int Binding;
	String BindTo;
	Array<ShaderBindingField> Fields;

};

struct ShaderStage
{
public:
	GraphicsPipelineStage Stage;
	std::shared_ptr<IGraphicsShader> Shader;
	Array<ShaderBinding> Bindings;
	Array<ShaderVertexStream> Streams;

};

class Shader
	: public IResource
{
private:
	Array<ShaderStage> m_stages;

public:
	static const char* Tag;

	Shader(const Array<ShaderStage>& stages);

};
