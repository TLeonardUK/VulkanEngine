#include "Engine/Graphics/GraphicsPipeline.h"

GraphicsPipelineSettings::GraphicsPipelineSettings()
	: hasVertexFormatDescription(false)
	, depthTestEnabled(false)
	, depthWriteEnabled(false)
{
}

void GraphicsPipelineSettings::SetShaderStage(GraphicsPipelineStage stage, std::shared_ptr<IGraphicsShader> shader)
{
	this->shaderStages[(int)stage] = shader;
}

void GraphicsPipelineSettings::SetRenderPass(std::shared_ptr<IGraphicsRenderPass> renderPass)
{
	this->renderPass = renderPass;
}

void GraphicsPipelineSettings::AddResourceSet(std::shared_ptr<IGraphicsResourceSet> set)
{
	this->resourceSets.push_back(set);
}

void GraphicsPipelineSettings::SetVertexFormat(const VertexBufferBindingDescription& format)
{
	this->vertexFormatDescription = format;
	this->hasVertexFormatDescription = true;
}

void GraphicsPipelineSettings::SetDepthTestEnabled(bool bValue)
{
	this->depthTestEnabled = bValue;
}

void GraphicsPipelineSettings::SetDepthWriteEnabled(bool bValue)
{
	this->depthWriteEnabled = bValue;
}