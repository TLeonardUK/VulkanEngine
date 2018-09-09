#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Types/Rectangle.h"

#include "Engine/Rendering/RenderPropertyCollection.h"
//#include "Engine/Resources/Types/Model.h"
//#include "Engine/Resources/Types/Material.h"
#include "Engine/Rendering/MeshRenderState.h"
#include "Engine/Rendering/MeshBatcher.h"

#include "Engine/Rendering/RenderView.h"

#include "Engine/UI/ImguiManager.h"

#include "Engine/Types/OctTree.h"

#include "Engine/ThirdParty/imgui/imgui.h"

class World;
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

#define HASH(x) extern const RenderPropertyHash x##Hash;
#include "Engine/Rendering/RendererHashes.inc"
#undef HASH

struct RenderCommand
{
	typedef std::function<void(std::shared_ptr<IGraphicsCommandBuffer> buffer)> CommandSignature_t;

	RenderCommandStage stage;
	CommandSignature_t command;
};

struct DrawViewState
{
	World* world = nullptr;
	Frustum frustum = Frustum::Empty; 

	MaterialVariant materialVariant = MaterialVariant::Normal;
	RenderPropertyCollection* viewProperties = nullptr;

	String name = "Untitled";
	RenderCommandStage stage = RenderCommandStage::View_GBuffer;

	Rect viewport = Rect::Empty;
	uint64_t viewId = 0;

	RenderFlags requiredFlags = RenderFlags::None;
	RenderFlags excludedFlags = RenderFlags::None;

	std::shared_ptr<IGraphicsFramebuffer> framebuffer = nullptr;

private:
	friend class Renderer;

	OctTree<Entity>::Result visibleEntitiesResult;
	MeshBatcher meshBatcher;

	Array<std::shared_ptr<IGraphicsCommandBuffer>> batchBuffers;
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
		uint64_t viewId;
		float renderOrder;
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

public:
	static const int GBufferImageCount = 3;
	static const int MaxShadowCascades = 8;

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

	Array<std::shared_ptr<IGraphicsFramebuffer>> m_swapChainFramebuffers;
	Array<std::shared_ptr<IGraphicsImageView>> m_swapChainViews;
	int m_swapChainWidth;
	int m_swapChainHeight;

	std::shared_ptr<IGraphicsImage> m_depthBufferImage;
	std::shared_ptr<IGraphicsImageView> m_depthBufferView;

	RenderPropertyCollection m_globalRenderProperties;

	Mutex m_queuedRenderCommandsMutex;
	Array<RenderCommand> m_queuedRenderCommands;

	std::shared_ptr<IGraphicsRenderPass> m_gbufferRenderPass;
	std::shared_ptr<IGraphicsImage> m_gbufferImages[GBufferImageCount];
	std::shared_ptr<IGraphicsImageView> m_gbufferViews[GBufferImageCount];
	std::shared_ptr<IGraphicsSampler> m_gbufferSamplers[GBufferImageCount];
	std::shared_ptr<IGraphicsFramebuffer> m_gbufferFrameBuffer;

	std::shared_ptr<IGraphicsRenderPass> m_shadowMaskRenderPass;
	std::shared_ptr<IGraphicsImage> m_shadowMaskImage;
	std::shared_ptr<IGraphicsImageView> m_shadowMaskImageView;
	std::shared_ptr<IGraphicsSampler> m_shadowMaskSampler;
	std::shared_ptr<IGraphicsFramebuffer> m_shadowMaskFrameBuffer;

	std::shared_ptr<IGraphicsRenderPass> m_lightAccumulationRenderPass;
	std::shared_ptr<IGraphicsImage> m_lightAccumulationImage;
	std::shared_ptr<IGraphicsImageView> m_lightAccumulationImageView;
	std::shared_ptr<IGraphicsSampler> m_lightAccumulationSampler;
	std::shared_ptr<IGraphicsFramebuffer> m_lightAccumulationFrameBuffer;

	ResourcePtr<Material> m_resolveToSwapchainMaterial;
	std::shared_ptr<MeshRenderState> m_resolveToSwapchainMeshRenderState;

	ResourcePtr<Material> m_clearGBufferMaterial;
	std::shared_ptr<MeshRenderState> m_clearGBufferMeshRenderState;

	ResourcePtr<Shader> m_depthOnlyShader;
	std::shared_ptr<IGraphicsRenderPass> m_depthOnlyRenderPass;

	ResourcePtr<Shader> m_normalizedDistanceShader;
	std::shared_ptr<IGraphicsRenderPass> m_normalizedDistanceRenderPass;

	std::shared_ptr<IGraphicsVertexBuffer> m_fullscreenQuadVertexBuffer;
	std::shared_ptr<IGraphicsIndexBuffer> m_fullscreenQuadIndexBuffer;
	bool m_fullscreenQuadsUploaded;

	ImguiCallbackToken m_debugMenuCallbackToken;

	Mutex m_queuedBuffersMutex;
	Array<QueuedBuffer> m_queuedBuffers;

	Mutex m_renderStateCreationMutex;

	Array<std::shared_ptr<IGraphicsImageView>> m_debugDisplayFrameBuffers;
	Mutex m_debugDisplayFrameBuffersMutex;

	// Debug functionality.
	bool m_drawFrameBuffersEnabled;
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
	void CreateShadowMaskResources();
	void CreateLightingResources();

	void SwapChainModified();

	void GeneratePreRender();
	void GeneratePostRender();
	void GeneratePrePresent();

	std::shared_ptr<IGraphicsResourceSet> AllocateResourceSet(const GraphicsResourceSetDescription& set);

	void RunQueuedCommands(RenderCommandStage stage, std::shared_ptr<IGraphicsCommandBuffer> buffer);

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
	ResourcePtr<Shader> GetNormalizedDistanceShader();

	void DisplayDebugFrameBuffer(std::shared_ptr<IGraphicsImageView> view);

	void DrawFullScreenQuad(
		std::shared_ptr<IGraphicsCommandBuffer> buffer, 
		std::shared_ptr<Material> material, 
		std::shared_ptr<MeshRenderState>* MeshRenderState,
		RenderPropertyCollection* viewProperties,
		RenderPropertyCollection* meshProperties,
		Rect viewport = Rect::Empty,
		Rect scissor = Rect::Empty);

	void DrawView(DrawViewState& state);

	std::shared_ptr<IGraphicsRenderPass> GetRenderPassForTarget(FrameBufferTarget target);
	std::shared_ptr<IGraphicsFramebuffer> GetFramebufferForTarget(FrameBufferTarget target);
	std::shared_ptr<IGraphicsFramebuffer> GetCurrentFramebuffer();

	std::shared_ptr<IGraphicsCommandBuffer> RequestSecondaryBuffer();
	std::shared_ptr<IGraphicsCommandBuffer> RequestPrimaryBuffer();

	void CreateMeshRenderState(std::shared_ptr<MeshRenderState>* state);

	void QueuePrimaryBuffer(const String& name, RenderCommandStage stage, std::shared_ptr<IGraphicsCommandBuffer>& buffer, uint64_t viewId = -1, float renderOrder = 0.0f);
	void QueueRenderCommand(RenderCommandStage stage, RenderCommand::CommandSignature_t callback);

	ResourcePtr<Material> GetResolveToSwapChainMaterial();
	std::shared_ptr<MeshRenderState>& GetResolveToSwapChainRenderState();

	ResourcePtr<Material> GetClearGBufferMaterial();
	std::shared_ptr<MeshRenderState>& GetClearGBufferRenderState();

	std::shared_ptr<IGraphicsImage> GetGBufferImage(int index);
	std::shared_ptr<IGraphicsImage> GetShadowMaskImage();
	std::shared_ptr<IGraphicsImage> GetLightAccumulationImage();
	std::shared_ptr<IGraphicsImage> GetDepthImage();

};