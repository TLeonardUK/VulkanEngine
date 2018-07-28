#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"

#include "Engine/Resources/Types/MaterialPropertyCollection.h"
#include "Engine/Resources/Types/Model.h"
#include "Engine/Resources/Types/Material.h"

#include "Engine/Rendering/RenderView.h"

#include "Engine/UI/ImguiManager.h"

#include "Engine/ThirdParty/imgui/imgui.h"

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

extern const MaterialPropertyHash ModelMatrixHash;
extern const MaterialPropertyHash ViewMatrixHash;
extern const MaterialPropertyHash ProjectionMatrixHash;
extern const MaterialPropertyHash CameraPositionHash;
extern const MaterialPropertyHash GBuffer0Hash;
extern const MaterialPropertyHash GBuffer1Hash;
extern const MaterialPropertyHash GBuffer2Hash;

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

struct MeshInstance
{
	std::shared_ptr<IGraphicsResourceSet> resourceSet;
	std::shared_ptr<IGraphicsResourceSetInstance> resourceSetInstance;
	std::shared_ptr<IGraphicsIndexBuffer> indexBuffer;
	std::shared_ptr<IGraphicsVertexBuffer> vertexBuffer;
	int indexCount;
};

struct MaterialBatch
{
	Material* material;

	Array<MeshInstance> meshInstances;
	int meshInstanceCount;
};

class Renderer
	: public std::enable_shared_from_this<Renderer>
{
private:
	struct GlobalUniformBuffer
	{
		std::shared_ptr<IGraphicsUniformBuffer> buffer;
		UniformBufferLayout layout;
	};

	// todo: I do not like this manual vertex binding, may cause issues with
	// renderers with non-c style packing. We should just create a model out of the 
	// data and allow that to deal with platform-specific issues.
	struct FullScreenQuadVertex
	{
		Vector2 position;
		Vector2 uv;
	};

	struct DebugLineVertex
	{
		Vector3 position;
		Vector4 color;
	};

private:
	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<IGraphics> m_graphics;
	std::shared_ptr<ResourceManager> m_resourceManager;
	std::shared_ptr<ImguiManager> m_imguiManager;
	int m_frameIndex;
	int m_frameCounter;

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

	std::mutex m_renderViewsMutex;
	Array<std::shared_ptr<RenderView>> m_renderViews;
	Array<std::shared_ptr<RenderView>> m_freeRenderViews;

	static const int GBufferImageCount = 3;
	std::shared_ptr<IGraphicsImage> m_gbufferImages[GBufferImageCount];
	std::shared_ptr<IGraphicsImageView> m_gbufferViews[GBufferImageCount];
	std::shared_ptr<IGraphicsSampler> m_gbufferSamplers[GBufferImageCount];
	std::shared_ptr<IGraphicsFramebuffer> m_gbufferFrameBuffer;
	
	ResourcePtr<Material> m_resolveToSwapchainMaterial;
	std::shared_ptr<MaterialRenderData> m_resolveToSwapchainMaterialRenderData;

	ResourcePtr<Material> m_clearGBufferMaterial;
	std::shared_ptr<MaterialRenderData> m_clearGBufferMaterialRenderData;

	ResourcePtr<Material> m_debugLineMaterial;
	std::shared_ptr<MaterialRenderData> m_debugLineMaterialRenderData;

	std::shared_ptr<IGraphicsVertexBuffer> m_fullscreenQuadVertexBuffer;
	std::shared_ptr<IGraphicsIndexBuffer> m_fullscreenQuadIndexBuffer;
	bool m_fullscreenQuadsUploaded;

	ImguiCallbackToken m_debugMenuCallbackToken;

	Dictionary<Material*, MaterialBatch> m_materialBatches;

	Dictionary<size_t, GlobalUniformBuffer> m_globalUniformBuffers;

	// Stats.
	int m_statMeshesRendered;
	int m_statBatchesRendered;
	int m_statTrianglesRendered;

	// Debug functionality.
	bool m_drawGBufferEnabled;
	bool m_drawWireframeEnabled;
	bool m_drawStatisticsEnabled;
	bool m_drawBoundsEnabled;
	bool m_renderingIsFrozen;

private:
	friend class Material;
	friend class MaterialRenderData;

	void CreateResources();
	void FreeResources();

	void FreeSwapChainDependentResources();
	void CreateSwapChainDependentResources();
	void CreateGBufferResources();

	void SwapChainModified();

	void BuildCommandBuffer(std::shared_ptr<IGraphicsCommandBuffer> buffer);

	void BuildViewCommandBuffer_Lines(std::shared_ptr<RenderView> view, std::shared_ptr<IGraphicsCommandBuffer> buffer);
	void BuildViewCommandBuffer_Meshes(std::shared_ptr<RenderView> view, std::shared_ptr<IGraphicsCommandBuffer> buffer);
	void BuildViewCommandBuffer(std::shared_ptr<RenderView> view, std::shared_ptr<IGraphicsCommandBuffer> buffer);

	std::shared_ptr<IGraphicsResourceSet> AllocateResourceSet(const GraphicsResourceSetDescription& set);

	void RunQueuedCommands(RenderCommandStage stage, std::shared_ptr<IGraphicsCommandBuffer> buffer);

	std::shared_ptr<IGraphicsRenderPass> GetRenderPassForTarget(FrameBufferTarget target);
	std::shared_ptr<IGraphicsFramebuffer> GetFramebufferForTarget(FrameBufferTarget target);

	void DrawFullScreenQuad(std::shared_ptr<IGraphicsCommandBuffer> buffer, std::shared_ptr<Material> material, std::shared_ptr<MaterialRenderData>* materialRenderData);

	void UpdateGlobalUniformBuffers();

public:
	Renderer(std::shared_ptr<Logger> m_logger, std::shared_ptr<IGraphics> graphics);

	void QueueRenderCommand(RenderCommandStage stage, RenderCommand::CommandSignature_t callback);

	void InitDebugMenus(std::shared_ptr<ImguiManager> manager);

	bool Init(std::shared_ptr<ResourceManager> resourceManager);
	void Dispose();
	void Present();

	bool IsRenderingFrozen();
	bool IsDrawBoundsEnabled();

	std::shared_ptr<RenderView> QueueView();

	MaterialPropertyCollection& GetGlobalMaterialProperties();
	int GetSwapChainWidth();
	int GetSwapChainHeight();

	std::shared_ptr<IGraphicsFramebuffer> GetCurrentFramebuffer();

	void RegisterGlobalUniformBuffer(const UniformBufferLayout& layout);
	std::shared_ptr<IGraphicsUniformBuffer> GetGlobalUniformBuffer(uint64_t hashCode);

	void UpdateMaterialRenderData(std::shared_ptr<MaterialRenderData>* data, const std::shared_ptr<Material>& material, MaterialPropertyCollection* collection);

};