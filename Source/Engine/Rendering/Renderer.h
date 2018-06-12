#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

#include "Engine/Resources/Types/MaterialPropertyCollection.h"
#include "Engine/Resources/Types/Model.h"

#include <memory>
#include <functional>

class IGraphics;
class IGraphicsCommandBuffer;
class IGraphicsCommandBufferPool;
class IGraphicsResourceSetPool;
class IGraphicsResourceSet;
class IGraphicsCommandBuffer;
class IGraphicsRenderPass;
class IGraphicsFramebuffer;
class IGraphicsImageView;
class IGraphicsImage;
struct GraphicsResourceSetDescription;
class RenderView;

enum class RenderCommandStage
{
	PreRender
};

struct RenderCommand
{
	typedef std::function<void(std::shared_ptr<IGraphicsCommandBuffer> buffer)> CommandSignature_t;

	RenderCommandStage stage;
	CommandSignature_t command;
};

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

	MaterialPropertyCollection m_globalMaterialProperties;

	std::recursive_mutex m_queuedRenderCommandsMutex;
	Array<RenderCommand> m_queuedRenderCommands;

	Array<ResourcePtr<Model>> m_tmpModelToRender;

	Array<std::shared_ptr<RenderView>> m_renderViews;

private:
	friend class Material;

	void CreateResources();
	void FreeResources();

	void FreeSwapChainDependentResources();
	void CreateSwapChainDependentResources();

	void SwapChainModified();

	void BuildCommandBuffer(std::shared_ptr<IGraphicsCommandBuffer> buffer);
	void BuildViewCommandBuffer(std::shared_ptr<RenderView> view, std::shared_ptr<IGraphicsFramebuffer> frameBuffer, std::shared_ptr<IGraphicsCommandBuffer> buffer);

	std::shared_ptr<IGraphicsResourceSet> AllocateResourceSet(const GraphicsResourceSetDescription& set);

	void RunQueuedCommands(RenderCommandStage stage, std::shared_ptr<IGraphicsCommandBuffer> buffer);

public:
	Renderer(std::shared_ptr<IGraphics> graphics);

	void QueueRenderCommand(RenderCommandStage stage, RenderCommand::CommandSignature_t callback);

	bool Init();
	void Dispose();
	void Present();

	void AddView(std::shared_ptr<RenderView> view);
	void RemoveView(std::shared_ptr<RenderView> view);

	MaterialPropertyCollection& GetGlobalMaterialProperties();
	int GetSwapChainWidth();
	int GetSwapChainHeight();

	void TmpAddModelToRender(ResourcePtr<Model> model);

};