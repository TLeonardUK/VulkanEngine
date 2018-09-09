#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"

#include "Engine/Graphics/GraphicsShader.h"
#include "Engine/Graphics/GraphicsRenderPass.h"
#include "Engine/Graphics/GraphicsPipeline.h"
#include "Engine/Graphics/GraphicsFramebuffer.h"
#include "Engine/Graphics/GraphicsImage.h"
#include "Engine/Graphics/GraphicsImageView.h"
#include "Engine/Graphics/GraphicsCommandBufferPool.h"
#include "Engine/Graphics/GraphicsCommandBuffer.h"
#include "Engine/Graphics/GraphicsVertexBuffer.h"
#include "Engine/Graphics/GraphicsIndexBuffer.h"
#include "Engine/Graphics/GraphicsUniformBuffer.h"
#include "Engine/Graphics/GraphicsResourceSetPool.h"
#include "Engine/Graphics/GraphicsSampler.h"
#include "Engine/Graphics/GraphicsEnums.h"

class IWindow;

class IGraphics
{
protected:
	IGraphics() { };

public:
	virtual ~IGraphics() { };
	virtual void Dispose() = 0;

	virtual String GetShaderPathPostfix() = 0;

	virtual bool AttachToWindow(std::shared_ptr<IWindow> window) = 0;
	virtual void SetPresentMode(GraphicsPresentMode mode) = 0;
	virtual bool Present() = 0;

	virtual std::shared_ptr<IGraphicsShader> CreateShader(const String& name, const String& entryPoint, GraphicsPipelineStage stage, const Array<char>& data) = 0;
	virtual std::shared_ptr<IGraphicsRenderPass> CreateRenderPass(const String& name, const GraphicsRenderPassSettings& settings) = 0;
	virtual std::shared_ptr<IGraphicsPipeline> CreatePipeline(const String& name, const GraphicsPipelineSettings& settings) = 0;
	virtual std::shared_ptr<IGraphicsFramebuffer> CreateFramebuffer(const String& name, const GraphicsFramebufferSettings& settings) = 0;
	virtual std::shared_ptr<IGraphicsImageView> CreateImageView(const String& name, std::shared_ptr<IGraphicsImage> image, int baseLayer = -1, int layerCount = -1) = 0;
	virtual std::shared_ptr<IGraphicsVertexBuffer> CreateVertexBuffer(const String& name, const VertexBufferBindingDescription& binding, int vertexCount) = 0;
	virtual std::shared_ptr<IGraphicsIndexBuffer> CreateIndexBuffer(const String& name, int indexSize, int indexCount) = 0;
	virtual std::shared_ptr<IGraphicsUniformBuffer> CreateUniformBuffer(const String& name, int dataSize) = 0;
	virtual std::shared_ptr<IGraphicsCommandBufferPool> CreateCommandBufferPool(const String& name) = 0;
	virtual std::shared_ptr<IGraphicsResourceSetPool> CreateResourceSetPool(const String& name) = 0;
	virtual std::shared_ptr<IGraphicsImage> CreateImage(const String& name, int width, int height, int layers, GraphicsFormat format, bool generateMips, GraphicsUsage usage) = 0;
	virtual std::shared_ptr<IGraphicsSampler> CreateSampler(const String& name, const SamplerDescription& settings) = 0;

	virtual void UpdateStatistics() = 0;

	virtual void Dispatch(const String& name, std::shared_ptr<IGraphicsCommandBuffer> buffer) = 0;

	virtual void WaitForDeviceIdle() = 0;

	virtual GraphicsFormat GetSwapChainFormat() = 0;
	virtual Array<std::shared_ptr<IGraphicsImageView>> GetSwapChainViews() = 0;

};
