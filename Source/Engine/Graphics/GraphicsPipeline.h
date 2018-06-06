#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Graphics/GraphicsEnums.h"
#include "Engine/Graphics/GraphicsVertexBuffer.h"
#include "Engine/Graphics/GraphicsRenderPass.h"
#include "Engine/Graphics/GraphicsResourceSet.h"

#include <memory>

class IGraphicsShader;

enum class GraphicsPipelineStage
{
	Vertex,
	Fragment,
	Count
};

struct GraphicsPipelineSettings
{
public:
	std::shared_ptr<IGraphicsRenderPass> renderPass;
	std::shared_ptr<IGraphicsShader> shaderStages[(int)GraphicsPipelineStage::Count];
	Array<std::shared_ptr<IGraphicsResourceSet>> resourceSets;

	bool depthTestEnabled;
	bool depthWriteEnabled;

	VertexBufferBindingDescription vertexFormatDescription;
	bool hasVertexFormatDescription;

public:
	GraphicsPipelineSettings();

	void SetShaderStage(GraphicsPipelineStage stage, std::shared_ptr<IGraphicsShader> shader);
	void SetRenderPass(std::shared_ptr<IGraphicsRenderPass> renderPass);
	void SetVertexFormat(const VertexBufferBindingDescription& format);
	void AddResourceSet(std::shared_ptr<IGraphicsResourceSet> set);
	void SetDepthTestEnabled(bool bValue);
	void SetDepthWriteEnabled(bool bValue);

};

class IGraphicsPipeline
{
protected:
	IGraphicsPipeline() { };

public:
	virtual ~IGraphicsPipeline() { };

};
