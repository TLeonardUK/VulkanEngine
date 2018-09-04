#pragma once
#include "Pch.h"

#include "Engine/Resources/Resource.h"
#include "Engine/Rendering/RendererEnums.h"
#include "Engine/Rendering/UniformBufferLayout.h"

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

struct ShaderBinding
{
public:
	String Name;
	GraphicsBindingFrequency Frequency;
	GraphicsBindingType Type;
	int ArrayLength;
	int Binding;
	int Set;
	String BindTo;
	RenderPropertyHash BindToHash;
	UniformBufferLayout UniformBufferLayout;
};

struct ShaderStage
{
public:
	GraphicsPipelineStage Stage;
	std::shared_ptr<IGraphicsShader> Shader;
	Array<ShaderVertexStream> Streams;

};

struct ShaderProperties
{
	bool ShadowCaster;
	bool ShadowReciever;
};

class Shader
	: public IResource
{
private:
	Array<ShaderStage> m_stages;
	Array<ShaderBinding> m_bindings;
	GraphicsPipelineSettings m_pipelineDescription;

	ShaderProperties m_properties;

	FrameBufferTarget m_target;

public:
	static const char* Tag;

	Shader(const Array<ShaderStage>& stages, const Array<ShaderBinding>& bindings, const GraphicsPipelineSettings& pipelineDescription, FrameBufferTarget target, const ShaderProperties& properties);
	virtual ~Shader();

	bool GetStage(GraphicsPipelineStage stage, const ShaderStage** result);
	const Array<ShaderStage>& GetStages();
	const Array<ShaderBinding>& GetBindings();

	FrameBufferTarget GetTarget();

	const ShaderProperties& GetProperties();

	const GraphicsPipelineSettings& GetPipelineDescription();

};
