#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Graphics/GraphicsEnums.h"
#include "Engine/Graphics/GraphicsVertexBuffer.h"
#include "Engine/Graphics/GraphicsRenderPass.h"
#include "Engine/Graphics/GraphicsResourceSet.h"

class IGraphicsShader;

struct GraphicsPipelineSettings
{
public:
	std::shared_ptr<IGraphicsRenderPass> RenderPass;
	std::shared_ptr<IGraphicsShader> ShaderStages[(int)GraphicsPipelineStage::Count];
	Array<std::shared_ptr<IGraphicsResourceSet>> ResourceSets;

	VertexBufferBindingDescription VertexFormatDescription;
	bool HasVertexFormatDescription;

	GraphicsPrimitiveType PrimitiveType;
	GraphicsPolygonMode PolygonMode;
	GraphicsCullMode CullMode;
	GraphicsFaceWindingOrder FaceWindingOrder;

	float LineWidth;

	bool DepthTestEnabled;
	bool DepthWriteEnabled;
	GraphicsDepthCompareOp DepthCompareOp;

	bool DepthClampEnabled;

	bool DepthBiasEnabled;
	float DepthBiasConstant;
	float DepthBiasClamp;
	float DepthBiasSlopeFactor;

	bool StencilTestEnabled;
	int StencilTestReference;
	int StencilTestReadMask;
	int StencilTestWriteMask;
	GraphicsStencilTestCompareOp StencilTestCompareOp;
	GraphicsStencilTestOp StencilTestPassOp;
	GraphicsStencilTestOp StencilTestFailOp;
	GraphicsStencilTestOp StencilTestZFailOp;

	bool BlendEnabled;
	GraphicsBlendFactor SrcColorBlendFactor;
	GraphicsBlendFactor DstColorBlendFactor;
	GraphicsBlendFactor SrcAlphaBlendFactor;
	GraphicsBlendFactor DstAlphaBlendFactor;
	GraphicsBlendOp ColorBlendOp;
	GraphicsBlendOp AlphaBlendOp;

public:
	GraphicsPipelineSettings();

};

class IGraphicsPipeline
{
protected:
	IGraphicsPipeline() { };

public:
	virtual ~IGraphicsPipeline() { };

};
