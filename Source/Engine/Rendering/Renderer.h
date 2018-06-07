#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

#include <memory>

class IGraphics;
class IGraphicsCommandBuffer;
class IGraphicsCommandBufferPool;
class IGraphicsResourceSetPool;
class IGraphicsCommandBuffer;
class IGraphicsRenderPass;
class IGraphicsFramebuffer;
class IGraphicsImageView;
class IGraphicsImage;

class Renderer
{
private:
	std::shared_ptr<IGraphics> m_graphics;
	int m_frameIndex;

	std::shared_ptr<IGraphicsCommandBufferPool> m_commandBufferPool;
	std::shared_ptr<IGraphicsResourceSetPool> m_resourceSetPool;
	Array<std::shared_ptr<IGraphicsCommandBuffer>> m_commandBuffers;

	std::shared_ptr<IGraphicsRenderPass> m_renderPass;

	Array<std::shared_ptr<IGraphicsFramebuffer>> m_swapChainFramebuffers;
	Array<std::shared_ptr<IGraphicsImageView>> m_swapChainViews;
	int m_swapChainWidth;
	int m_swapChainHeight;

	std::shared_ptr<IGraphicsImage> m_depthBufferImage;
	std::shared_ptr<IGraphicsImageView> m_depthBufferView;

private:
	void CreateResources();
	void FreeResources();

	void FreeSwapChainDependentResources();
	void CreateSwapChainDependentResources();

	void SwapChainModified();

	void BuildCommandBuffer(std::shared_ptr<IGraphicsCommandBuffer> buffer);

public:
	Renderer(std::shared_ptr<IGraphics> graphics);

	bool Init();
	void Dispose();

	void Present();

};