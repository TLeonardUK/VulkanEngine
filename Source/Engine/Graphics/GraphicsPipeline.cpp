#include "Pch.h"

#include "Engine/Graphics/GraphicsPipeline.h"

GraphicsPipelineSettings::GraphicsPipelineSettings()
	: RenderPass(nullptr)
	, HasVertexFormatDescription(false)
	, PrimitiveType(GraphicsPrimitiveType::TriangleList)
	, PolygonMode(GraphicsPolygonMode::Fill)
	, CullMode(GraphicsCullMode::Back)
	, FaceWindingOrder(GraphicsFaceWindingOrder::CounterClockwise)
	, DepthTestEnabled(true)
	, DepthWriteEnabled(true)
	, DepthCompareOp(GraphicsDepthCompareOp::Less)
	, DepthBiasEnabled(false)
	, StencilTestEnabled(false)
	, StencilTestReference(0)
	, StencilTestReadMask(0)
	, StencilTestWriteMask(0)
	, StencilTestCompareOp(GraphicsStencilTestCompareOp::Always)
	, StencilTestPassOp(GraphicsStencilTestOp::Keep)
	, StencilTestFailOp(GraphicsStencilTestOp::Keep)
	, StencilTestZFailOp(GraphicsStencilTestOp::Keep)
	, BlendEnabled(false)
	, SrcColorBlendFactor(GraphicsBlendFactor::SrcAlpha)
	, DstColorBlendFactor(GraphicsBlendFactor::OneMinusSrcAlpha)
	, ColorBlendOp(GraphicsBlendOp::Add)
	, SrcAlphaBlendFactor(GraphicsBlendFactor::OneMinusSrcAlpha)
	, DstAlphaBlendFactor(GraphicsBlendFactor::Zero)
	, AlphaBlendOp(GraphicsBlendOp::Add)
	, LineWidth(1.0f)
{	
}
