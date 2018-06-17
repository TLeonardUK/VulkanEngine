#pragma once

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

#include "Engine/Resources/Types/MaterialPropertyCollection.h"
#include "Engine/Resources/Types/Model.h"
#include "Engine/Resources/Types/Material.h"

#include "Engine/UI/ImguiManager.h"

#include "Engine/ThirdParty/imgui/imgui.h"

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
class IGraphicsSampler;
class IGraphicsImage;
struct GraphicsResourceSetDescription;
class RenderView;

enum class RenderCommandStage
{
	PreRender,
	PostViewsRendered
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
	std::shared_ptr<ResourceManager> m_resourceManager;
	std::shared_ptr<ImguiManager> m_imguiManager;
	int m_frameIndex;

	std::shared_ptr<IGraphicsCommandBufferPool> m_commandBufferPool;
	std::shared_ptr<IGraphicsResourceSetPool> m_resourceSetPool;
	Array<std::shared_ptr<IGraphicsCommandBuffer>> m_commandBuffers;

	std::shared_ptr<IGraphicsRenderPass> m_resolveToSwapChainRenderPass;

	std::shared_ptr<IGraphicsRenderPass> m_gbufferRenderPass;

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

	static const int GBufferImageCount = 3;
	std::shared_ptr<IGraphicsImage> m_gbufferImages[GBufferImageCount];
	std::shared_ptr<IGraphicsImageView> m_gbufferViews[GBufferImageCount];
	std::shared_ptr<IGraphicsSampler> m_gbufferSamplers[GBufferImageCount];
	std::shared_ptr<IGraphicsFramebuffer> m_gbufferFrameBuffer;

	ResourcePtr<Material> m_resolveToSwapchainMaterial;
	std::shared_ptr<IGraphicsVertexBuffer> m_fullscreenQuadVertexBuffer;
	std::shared_ptr<IGraphicsIndexBuffer> m_fullscreenQuadIndexBuffer;
	bool m_fullscreenQuadsUploaded;

	ImguiCallbackToken m_debugMenuCallbackToken;

private:
	friend class Material;

	void CreateResources();
	void FreeResources();

	void FreeSwapChainDependentResources();
	void CreateSwapChainDependentResources();
	void CreateGBufferResources();

	void SwapChainModified();

	void BuildCommandBuffer(std::shared_ptr<IGraphicsCommandBuffer> buffer);
	void BuildViewCommandBuffer(std::shared_ptr<RenderView> view, std::shared_ptr<IGraphicsCommandBuffer> buffer);

	std::shared_ptr<IGraphicsResourceSet> AllocateResourceSet(const GraphicsResourceSetDescription& set);

	void RunQueuedCommands(RenderCommandStage stage, std::shared_ptr<IGraphicsCommandBuffer> buffer);

	std::shared_ptr<IGraphicsRenderPass> GetRenderPassForTarget(FrameBufferTarget target);
	std::shared_ptr<IGraphicsFramebuffer> GetFramebufferForTarget(FrameBufferTarget target);

	void DrawFullScreenQuad(std::shared_ptr<IGraphicsCommandBuffer> buffer, std::shared_ptr<Material> material);

public:
	Renderer(std::shared_ptr<IGraphics> graphics);

	void QueueRenderCommand(RenderCommandStage stage, RenderCommand::CommandSignature_t callback);

	void InitDebugMenus(std::shared_ptr<ImguiManager> manager);

	bool Init(std::shared_ptr<ResourceManager> resourceManager);
	void Dispose();
	void Present();

	void AddView(std::shared_ptr<RenderView> view);
	void RemoveView(std::shared_ptr<RenderView> view);

	MaterialPropertyCollection& GetGlobalMaterialProperties();
	int GetSwapChainWidth();
	int GetSwapChainHeight();

	std::shared_ptr<IGraphicsFramebuffer> GetCurrentFramebuffer();

	void TmpAddModelToRender(ResourcePtr<Model> model);

};