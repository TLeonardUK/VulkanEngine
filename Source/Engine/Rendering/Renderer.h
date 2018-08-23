#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

#include "Engine/Rendering/RenderPropertyCollection.h"
//#include "Engine/Resources/Types/Model.h"
//#include "Engine/Resources/Types/Material.h"
#include "Engine/Rendering/MeshRenderState.h"

#include "Engine/Rendering/RenderView.h"

#include "Engine/UI/ImguiManager.h"

#include "Engine/ThirdParty/imgui/imgui.h"

class Material;
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
struct Statistic;
struct GraphicsResourceSetDescription;

extern const RenderPropertyHash ModelMatrixHash;
extern const RenderPropertyHash ViewMatrixHash;
extern const RenderPropertyHash ProjectionMatrixHash;
extern const RenderPropertyHash CameraPositionHash;
extern const RenderPropertyHash GBuffer0Hash;
extern const RenderPropertyHash GBuffer1Hash;
extern const RenderPropertyHash GBuffer2Hash;

enum class RenderCommandStage
{
	PreRender,		// Before primary render buffers.
	Shadow,			// Shadow map rendering.
	Render,			// During primary render buffers.
	Debug,			// Debug rendering stage.
	PostRender,		// After gbuffer etc has completed.
	PostResolve,	// After gbuffer has been resolved to swapchain.
	PrePresent,		// Just before present, used to transition swap chain to appropriate layout.
};

struct RenderCommand
{
	typedef std::function<void(std::shared_ptr<IGraphicsCommandBuffer> buffer)> CommandSignature_t;

	RenderCommandStage stage;
	CommandSignature_t command;
};

class Renderer
	: public std::enable_shared_from_this<Renderer>
{
private:

	// todo: I do not like this manual vertex binding, may cause issues with
	// renderers with non-c style packing. We should just create a model out of the 
	// data and allow that to deal with platform-specific issues.
	struct FullScreenQuadVertex
	{
		Vector2 position;
		Vector2 uv;
	};

	struct QueuedBuffer
	{
		String name;
		RenderCommandStage stage;
		std::shared_ptr<IGraphicsCommandBuffer> buffer;
	};

	struct ThreadLocalCommandBufferPoolFrameData
	{
		Array<std::shared_ptr<IGraphicsCommandBuffer>> secondaryBuffers;
		int secondaryBuffersAllocated;

		Array<std::shared_ptr<IGraphicsCommandBuffer>> primaryBuffers;
		int primaryBuffersAllocated;

		ThreadLocalCommandBufferPoolFrameData()
			: secondaryBuffersAllocated(0)
			, primaryBuffersAllocated(0)
		{
		}
	};

	struct ThreadLocalCommandBufferPool
	{
		Mutex mutex;
		std::shared_ptr<IGraphicsCommandBufferPool> pool;
		Array<ThreadLocalCommandBufferPoolFrameData> frameData;
	};

private:
	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<IGraphics> m_graphics;
	std::shared_ptr<ResourceManager> m_resourceManager;
	std::shared_ptr<ImguiManager> m_imguiManager;
	int m_frameIndex;
	int m_frameCounter;

	static thread_local int m_commandBufferPoolIndexTls;

	Mutex m_commandBufferPoolsMutex;
	Array<std::shared_ptr<ThreadLocalCommandBufferPool>> m_commandBufferPools;

	std::shared_ptr<IGraphicsResourceSetPool> m_resourceSetPool;

	std::shared_ptr<IGraphicsRenderPass> m_resolveToSwapChainRenderPass;
	std::shared_ptr<IGraphicsRenderPass> m_gbufferRenderPass;

	Array<std::shared_ptr<IGraphicsFramebuffer>> m_swapChainFramebuffers;
	Array<std::shared_ptr<IGraphicsImageView>> m_swapChainViews;
	int m_swapChainWidth;
	int m_swapChainHeight;

	std::shared_ptr<IGraphicsImage> m_depthBufferImage;
	std::shared_ptr<IGraphicsImageView> m_depthBufferView;

	RenderPropertyCollection m_globalRenderProperties;

	Mutex m_queuedRenderCommandsMutex;
	Array<RenderCommand> m_queuedRenderCommands;

	static const int GBufferImageCount = 3;
	std::shared_ptr<IGraphicsImage> m_gbufferImages[GBufferImageCount];
	std::shared_ptr<IGraphicsImageView> m_gbufferViews[GBufferImageCount];
	std::shared_ptr<IGraphicsSampler> m_gbufferSamplers[GBufferImageCount];
	std::shared_ptr<IGraphicsFramebuffer> m_gbufferFrameBuffer;
	
	ResourcePtr<Material> m_resolveToSwapchainMaterial;
	std::shared_ptr<MeshRenderState> m_resolveToSwapchainMeshRenderState;

	ResourcePtr<Material> m_clearGBufferMaterial;
	std::shared_ptr<MeshRenderState> m_clearGBufferMeshRenderState;

	ResourcePtr<Shader> m_depthOnlyShader;
	std::shared_ptr<IGraphicsRenderPass> m_depthOnlyRenderPass;

	std::shared_ptr<IGraphicsVertexBuffer> m_fullscreenQuadVertexBuffer;
	std::shared_ptr<IGraphicsIndexBuffer> m_fullscreenQuadIndexBuffer;
	bool m_fullscreenQuadsUploaded;

	ImguiCallbackToken m_debugMenuCallbackToken;

	Mutex m_queuedBuffersMutex;
	Array<QueuedBuffer> m_queuedBuffers;

	Mutex m_renderStateCreationMutex;

	// Debug functionality.
	bool m_drawGBufferEnabled;
	bool m_drawWireframeEnabled;
	bool m_drawStatisticsEnabled;
	bool m_drawBoundsEnabled;
	bool m_renderingIsFrozen;

private:
	friend class Material;
	friend class MeshRenderState;
	friend struct RenderPropertyCollection;

	void CreateResources();
	void FreeResources();

	void FreeSwapChainDependentResources();
	void CreateSwapChainDependentResources();
	void CreateGBufferResources();

	void SwapChainModified();

	void BuildCommandBuffer_PreRender(std::shared_ptr<IGraphicsCommandBuffer> buffer);
	void BuildCommandBuffer_PostRender(std::shared_ptr<IGraphicsCommandBuffer> buffer);
	void BuildCommandBuffer_PrePresent(std::shared_ptr<IGraphicsCommandBuffer> buffer);

	std::shared_ptr<IGraphicsResourceSet> AllocateResourceSet(const GraphicsResourceSetDescription& set);

	void RunQueuedCommands(RenderCommandStage stage, std::shared_ptr<IGraphicsCommandBuffer> buffer);

	void DrawFullScreenQuad(std::shared_ptr<IGraphicsCommandBuffer> buffer, std::shared_ptr<Material> material, std::shared_ptr<MeshRenderState>* MeshRenderState);

	std::shared_ptr<ThreadLocalCommandBufferPool>& GetCommandBufferPoolForThread();
	
	void UpdateStatistics();
	void UpdateCommandBufferPools();

public:
	Renderer(std::shared_ptr<Logger> m_logger, std::shared_ptr<IGraphics> graphics);

	void InitDebugMenus(std::shared_ptr<ImguiManager> manager);
	void ShowStatisticTree(Statistic* stat);

	bool Init(std::shared_ptr<ResourceManager> resourceManager);
	void Dispose();
	void Present();

	bool IsRenderingFrozen();
	bool IsDrawBoundsEnabled();
	bool IsWireframeEnabled();

	RenderPropertyCollection& GetGlobalRenderProperties();
	int GetSwapChainWidth();
	int GetSwapChainHeight();

	ResourcePtr<Shader> GetDepthOnlyShader();

	std::shared_ptr<IGraphicsRenderPass> GetRenderPassForTarget(FrameBufferTarget target);
	std::shared_ptr<IGraphicsFramebuffer> GetFramebufferForTarget(FrameBufferTarget target);
	std::shared_ptr<IGraphicsFramebuffer> GetCurrentFramebuffer();

	std::shared_ptr<IGraphicsCommandBuffer> RequestSecondaryBuffer();
	std::shared_ptr<IGraphicsCommandBuffer> RequestPrimaryBuffer();

	void CreateMeshRenderState(std::shared_ptr<MeshRenderState>* state);

	void QueuePrimaryBuffer(const String& name, RenderCommandStage stage, std::shared_ptr<IGraphicsCommandBuffer>& buffer);
	void QueueRenderCommand(RenderCommandStage stage, RenderCommand::CommandSignature_t callback);

};