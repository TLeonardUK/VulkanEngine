#include "Pch.h"

#include "Engine/Components/Mesh/MeshComponent.h"

#include "Engine/Rendering/Renderer.h"
#include "Engine/Rendering/RenderView.h"
#include "Engine/Rendering/RendererEnums.h"
#include "Engine/Rendering/RenderPropertyHeirarchy.h"
#include "Engine/Profiling/Profiling.h"

#include "Engine/Engine/Logging.h"
#include "Engine/ECS/World.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/GraphicsResourceSet.h"

#include "Engine/UI/ImguiManager.h"

#include "Engine/Types/Math.h"

#include "Engine/Utilities/Statistic.h"
#include "Engine/Threading/ParallelFor.h"

#include "Engine/Systems/Transform/SpatialIndexSystem.h"

#define HASH(x) const RenderPropertyHash x##Hash = CalculateRenderPropertyHash(#x);
#include "Engine/Rendering/RendererHashes.inc"
#undef HASH

Statistic Stat_Rendering_Budgets_MeshesRendered		("Rendering/Budgets/Meshs Rendered",		StatisticFrequency::PerFrame,	StatisticFormat::Integer);
Statistic Stat_Rendering_Budgets_BatchesRendered	("Rendering/Budgets/Batchs Rendered",		StatisticFrequency::PerFrame,	StatisticFormat::Integer);
Statistic Stat_Rendering_Budgets_TrianglesRendered	("Rendering/Budgets/Triangles Rendered",	StatisticFrequency::PerFrame,	StatisticFormat::Integer);

Statistic Stat_Rendering_Memory_PoolBlockCount		("Rendering/Memory/Pool Block Count",		StatisticFrequency::Persistent, StatisticFormat::Integer);
Statistic Stat_Rendering_Memory_PoolAllocationCount	("Rendering/Memory/Pool Allocation Count",	StatisticFrequency::Persistent, StatisticFormat::Integer);
Statistic Stat_Rendering_Memory_PoolUsedBytes		("Rendering/Memory/Pool Used Bytes",		StatisticFrequency::Persistent, StatisticFormat::Bytes);
Statistic Stat_Rendering_Memory_PoolUnusedBytes		("Rendering/Memory/Pool Unused Bytes",		StatisticFrequency::Persistent, StatisticFormat::Bytes);

thread_local int Renderer::m_commandBufferPoolIndexTls = -1;

Renderer::Renderer(std::shared_ptr<Logger> logger, std::shared_ptr<IGraphics> graphics)
	: m_logger(logger)
	, m_graphics(graphics)
	, m_frameIndex(0)
	// DEBUG DEBUG DEBUG
	, m_drawFrameBuffersEnabled(true)
	// DEBUG DEBUG DEBUG
	, m_drawBoundsEnabled(false)
	, m_drawWireframeEnabled(false)
	, m_renderingIsFrozen(false)
	, m_frameCounter(0)
{
}

void Renderer::QueueRenderCommand(RenderCommandStage stage, RenderCommand::CommandSignature_t callback)
{
	ScopeLock lock(m_queuedRenderCommandsMutex);

	RenderCommand command;
	command.stage = stage;
	command.command = callback;

	m_queuedRenderCommands.push_back(command);
}

void Renderer::RunQueuedCommands(RenderCommandStage stage, std::shared_ptr<IGraphicsCommandBuffer> buffer)
{
	ScopeLock lock(m_queuedRenderCommandsMutex);

	for (auto iter = m_queuedRenderCommands.begin(); iter != m_queuedRenderCommands.end(); )
	{
		RenderCommand command = *iter;
		if (command.stage == stage)
		{
			m_queuedRenderCommands.erase(iter);

			command.command(buffer);

			iter = m_queuedRenderCommands.begin();
		}
		else
		{
			iter++;
		}
	}
}

bool Renderer::Init(std::shared_ptr<ResourceManager> resourceManager)
{
	m_resourceManager = resourceManager;

	CreateResources();

	return true;
}

void Renderer::DisplayDebugFrameBuffer(std::shared_ptr<IGraphicsImageView> view)
{
	ScopeLock lock(m_debugDisplayFrameBuffersMutex);
	m_debugDisplayFrameBuffers.push_back(view);
}

void Renderer::InitDebugMenus(std::shared_ptr<ImguiManager> manager)
{
	m_imguiManager = manager;
	
	m_debugMenuCallbackToken = m_imguiManager->RegisterCallback(ImguiCallback::MainMenu, [=] {

		if (ImGui::BeginMenu("Render"))
		{
			//ImGui::MenuItem("Toggle Scene Graph", nullptr, &m_drawFrameBuffersEnabled);
			ImGui::MenuItem("Toggle Frame Buffers", nullptr, &m_drawFrameBuffersEnabled);
			ImGui::MenuItem("Toggle Wireframe", nullptr, &m_drawWireframeEnabled);
			ImGui::MenuItem("Toggle Statistics", nullptr, &m_drawStatisticsEnabled);
			ImGui::MenuItem("Toggle Bounds", nullptr, &m_drawBoundsEnabled);			
			ImGui::Separator();
			ImGui::MenuItem("Toggle Frozen Rendering", nullptr, &m_renderingIsFrozen);			
			ImGui::EndMenu();
		}
	});

	m_debugMenuCallbackToken = m_imguiManager->RegisterCallback(ImguiCallback::PostMainMenu, [=] {

		if (m_drawFrameBuffersEnabled)
		{
			int imageSize = 250;
			
			ImGui::SetNextWindowContentSize(ImVec2(imageSize * 3.0f, 0.0f));

			ImGui::Begin("Frame Buffers", &m_drawFrameBuffersEnabled, ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::Columns(3);
			ImGui::SetColumnWidth(0, (float)imageSize);
			ImGui::SetColumnWidth(1, (float)imageSize);
			ImGui::SetColumnWidth(2, (float)imageSize);

			ScopeLock lock(m_debugDisplayFrameBuffersMutex);

			// We sort the display buffers as they are enqueued on different threads, so this ensures somewhat
			// consistent ordering.
			std::sort(m_debugDisplayFrameBuffers.begin(), m_debugDisplayFrameBuffers.end(),
				[](const std::shared_ptr<IGraphicsImageView>& a, const std::shared_ptr<IGraphicsImageView>& b) -> bool
			{
				return reinterpret_cast<intptr_t>(&*a) < reinterpret_cast<intptr_t>(&*b);
			});

			for (int i = 0; i < m_debugDisplayFrameBuffers.size(); i++)
			{
				ImTextureID textureId = manager->StoreImage(m_debugDisplayFrameBuffers[i]);
				ImGui::Text(m_debugDisplayFrameBuffers[i]->GetImage()->GetImageName().c_str());
				ImGui::Image(textureId, ImVec2(static_cast<float>(imageSize), static_cast<float>(imageSize)));
				ImGui::NextColumn();
			}

			m_debugDisplayFrameBuffers.clear();

			ImGui::Columns();
			ImGui::End();
		}

		if (m_drawStatisticsEnabled)
		{
			UpdateStatistics();

			ImGui::Begin("Statistics", &m_drawStatisticsEnabled);
			ImGui::Columns(4);

			ImGui::Text("Name"); 
			ImGui::NextColumn();
			ImGui::Text("Value");
			ImGui::NextColumn();
			ImGui::Text("Average");
			ImGui::NextColumn();
			ImGui::Text("Peak");
			ImGui::NextColumn();

			ImGui::Separator();

			ShowStatisticTree(Statistic::GetRoot());

			ImGui::Columns();
			ImGui::End();
		}

	});
}

void Renderer::UpdateStatistics()
{
	m_graphics->UpdateStatistics();
}

void Renderer::ShowStatisticTree(Statistic* stat)
{
	// Child folders.
	for (auto& child : stat->GetChildren())
	{
		if (child->GetChildren().size() != 0)
		{
			if (ImGui::TreeNode(child->GetName().c_str()))
			{
				ImGui::NextColumn();
				ImGui::NextColumn();
				ImGui::NextColumn();
				ImGui::NextColumn();

				ShowStatisticTree(child);

				ImGui::TreePop();
			}
		}
	}

	// Values
	for (auto& child : stat->GetChildren())
	{
		if (child->GetChildren().size() == 0)
		{
			ImGui::Text("%s", child->GetName().c_str());
			ImGui::NextColumn();
			ImGui::Text("%s", child->GetFormattedValue().c_str());
			ImGui::NextColumn();
			ImGui::Text("%s", child->GetFormattedAverage().c_str());
			ImGui::NextColumn();
			ImGui::Text("%s", child->GetFormattedMax().c_str());
			ImGui::NextColumn();
		}
	}
}

void Renderer::Dispose()
{
	m_imguiManager->UnregisterCallback(m_debugMenuCallbackToken);
	
	FreeResources();
}

void Renderer::CreateResources()
{
	m_resourceSetPool = m_graphics->CreateResourceSetPool("Main Resource Set Pool");

	m_clearGBufferMaterial = m_resourceManager->Load<Material>("Engine/Materials/clear_gbuffer.json");
	m_resolveToSwapchainMaterial = m_resourceManager->Load<Material>("Engine/Materials/resolve_to_swapchain.json");
	m_depthOnlyShader = m_resourceManager->Load<Shader>("Engine/Shaders/depth_only.json");
	m_normalizedDistanceShader = m_resourceManager->Load<Shader>("Engine/Shaders/normalized_distance.json");

	m_resourceManager->WaitUntilIdle();

	CreateMeshRenderState(&m_resolveToSwapchainMeshRenderState);
	CreateMeshRenderState(&m_clearGBufferMeshRenderState);

	VertexBufferBindingDescription description;
	m_resolveToSwapchainMaterial.Get()->GetVertexBufferFormat(description);
	m_fullscreenQuadVertexBuffer = m_graphics->CreateVertexBuffer("Full Screen Quad Vertex Buffer", description, 4);
	m_fullscreenQuadIndexBuffer = m_graphics->CreateIndexBuffer("Full Screen Quad Index Buffer", sizeof(uint16_t), 6);
	m_fullscreenQuadsUploaded = false;

	Array<FullScreenQuadVertex> fullscreenQuadVerts;
	fullscreenQuadVerts.push_back({ Vector2(-1.0f, -1.0f), Vector2(0.0f, 0.0f) }); // Top Left
	fullscreenQuadVerts.push_back({ Vector2( 1.0f, -1.0f), Vector2(1.0f, 0.0f) }); // Top Right
	fullscreenQuadVerts.push_back({ Vector2(-1.0f,  1.0f), Vector2(0.0f, 1.0f) }); // Bottom Left
	fullscreenQuadVerts.push_back({ Vector2( 1.0f,  1.0f), Vector2(1.0f, 1.0f) }); // Bottom Right

	Array<uint16_t> fullscreenQuadIndices;
	fullscreenQuadIndices.push_back(2);
	fullscreenQuadIndices.push_back(1);
	fullscreenQuadIndices.push_back(0);
	fullscreenQuadIndices.push_back(3);
	fullscreenQuadIndices.push_back(1);
	fullscreenQuadIndices.push_back(2);

	m_fullscreenQuadVertexBuffer->Stage(fullscreenQuadVerts.data(), 0, sizeof(FullScreenQuadVertex) * (int)fullscreenQuadVerts.size());
	m_fullscreenQuadIndexBuffer->Stage(fullscreenQuadIndices.data(), 0, sizeof(uint16_t) * (int)fullscreenQuadIndices.size());

	CreateSwapChainDependentResources();
}

void Renderer::CreateMeshRenderState(std::shared_ptr<MeshRenderState>* state)
{
	ScopeLock lock(m_renderStateCreationMutex); // todo: not required, create up-front.
	if (*state == nullptr)
	{
		*state = std::make_shared<MeshRenderState>(m_logger, shared_from_this(), m_graphics);
	}
}

void Renderer::FreeResources()
{
	for (int i = 0; i < m_commandBufferPools.size(); i++)
	{
		m_commandBufferPools[i] = nullptr;
	}

	FreeSwapChainDependentResources();
}

void Renderer::FreeSwapChainDependentResources()
{
	m_depthBufferImage = nullptr;
	m_depthBufferView = nullptr;
	m_resolveToSwapChainRenderPass = nullptr;
	m_depthOnlyRenderPass = nullptr;
	m_normalizedDistanceRenderPass = nullptr;
	m_swapChainFramebuffers.clear();

	for (int i = 0; i < m_commandBufferPools.size(); i++)
	{
		m_commandBufferPools[i] = nullptr;
	}

	for (int i = 0; i < GBufferImageCount; i++)
	{
		m_gbufferImages[i] = nullptr;
		m_gbufferViews[i] = nullptr;
		m_gbufferSamplers[i] = nullptr;
	}
	m_gbufferFrameBuffer = nullptr;

	m_shadowMaskImage = nullptr;
	m_shadowMaskImageView = nullptr;
	m_shadowMaskSampler = nullptr;
	m_shadowMaskFrameBuffer = nullptr;

	m_lightAccumulationImage = nullptr;
	m_lightAccumulationImageView = nullptr;
	m_lightAccumulationSampler = nullptr;
	m_lightAccumulationFrameBuffer = nullptr;
}

void Renderer::CreateSwapChainDependentResources()
{
	m_swapChainViews = m_graphics->GetSwapChainViews();
	m_swapChainWidth = m_swapChainViews[0]->GetWidth();
	m_swapChainHeight = m_swapChainViews[0]->GetHeight();

	// Create render pass.
	{
		GraphicsRenderPassSettings renderPassSettings;
		renderPassSettings.transitionToPresentFormat = false;
		renderPassSettings.AddColorAttachment(m_graphics->GetSwapChainFormat(), true);
		renderPassSettings.AddDepthAttachment(GraphicsFormat::UNORM_D24_UINT_S8);

		GraphicsSubPassIndex subPass1 = renderPassSettings.AddSubPass();
		renderPassSettings.AddSubPassDependency(GraphicsExternalPassIndex, GraphicsAccessMask::None, subPass1, GraphicsAccessMask::ReadWrite);

		m_resolveToSwapChainRenderPass = m_graphics->CreateRenderPass("Swap Chain Render Pass", renderPassSettings);
	}

	// Create depth-only render pass.
	{
		GraphicsRenderPassSettings renderPassSettings;
		renderPassSettings.transitionToPresentFormat = false;
		renderPassSettings.AddDepthAttachment(GraphicsFormat::SFLOAT_D32);

		GraphicsSubPassIndex subPass1 = renderPassSettings.AddSubPass();
		renderPassSettings.AddSubPassDependency(GraphicsExternalPassIndex, GraphicsAccessMask::None, subPass1, GraphicsAccessMask::ReadWrite);

		m_depthOnlyRenderPass = m_graphics->CreateRenderPass("Depth Only Render Pass", renderPassSettings);
	}

	// Create normalized distance render pass.
	{
		GraphicsRenderPassSettings renderPassSettings;
		renderPassSettings.transitionToPresentFormat = false;
		renderPassSettings.AddDepthAttachment(GraphicsFormat::SFLOAT_D32);

		GraphicsSubPassIndex subPass1 = renderPassSettings.AddSubPass();
		renderPassSettings.AddSubPassDependency(GraphicsExternalPassIndex, GraphicsAccessMask::None, subPass1, GraphicsAccessMask::ReadWrite);

		m_normalizedDistanceRenderPass = m_graphics->CreateRenderPass("Normalized Distance Render Pass", renderPassSettings);
	}

	// Create depth buffer.
	m_depthBufferImage = m_graphics->CreateImage("Depth Buffer", m_swapChainWidth, m_swapChainHeight, 1, GraphicsFormat::UNORM_D24_UINT_S8, false, GraphicsUsage::DepthAttachment);
	m_depthBufferView = m_graphics->CreateImageView("Depth Buffer View", m_depthBufferImage);

	// Create frame buffers for each swap chain image.
	m_swapChainFramebuffers.resize(m_swapChainViews.size());
	for (int i = 0; i < m_swapChainViews.size(); i++)
	{
		std::shared_ptr<IGraphicsImageView> imageView = m_swapChainViews[i];

		GraphicsFramebufferSettings frameBufferSettings;
		frameBufferSettings.width = imageView->GetWidth();
		frameBufferSettings.height = imageView->GetHeight();
		frameBufferSettings.renderPass = m_resolveToSwapChainRenderPass;
		frameBufferSettings.attachments.push_back(imageView);
		frameBufferSettings.attachments.push_back(m_depthBufferView);

		m_swapChainFramebuffers[i] = m_graphics->CreateFramebuffer(StringFormat("Swap Chain Framebuffer %i", i), frameBufferSettings);;
	}

	CreateLightingResources();
	CreateShadowMaskResources();
	CreateGBufferResources();

	m_frameIndex = 0;
	m_frameCounter = 0;
}

void Renderer::CreateShadowMaskResources()
{
	// Create render pass.
	GraphicsRenderPassSettings renderPassSettings;
	renderPassSettings.transitionToPresentFormat = false;
	// DEBUG DEBUG DEBUG
	//renderPassSettings.AddColorAttachment(GraphicsFormat::UNORM_R8, false);
	renderPassSettings.AddColorAttachment(GraphicsFormat::SFLOAT_R16G16B16A16, false);
	// DEBUG DEBUG DEBUG

	GraphicsSubPassIndex subPass1 = renderPassSettings.AddSubPass();
	renderPassSettings.AddSubPassDependency(GraphicsExternalPassIndex, GraphicsAccessMask::None, subPass1, GraphicsAccessMask::ReadWrite);

	m_shadowMaskRenderPass = m_graphics->CreateRenderPass("Shadow Mask Render Pass", renderPassSettings);

	SamplerDescription samplerDescription;

	// Create shadow mask buffer.
	// DEBUG DEBUG DEBUG
	//m_shadowMaskImage = m_graphics->CreateImage("Shadow Mask", m_swapChainWidth, m_swapChainHeight, 1, GraphicsFormat::UNORM_R8, false, GraphicsUsage::ColorAttachment);
	m_shadowMaskImage = m_graphics->CreateImage("Shadow Mask", m_swapChainWidth, m_swapChainHeight, 1, GraphicsFormat::SFLOAT_R16G16B16A16, false, GraphicsUsage::ColorAttachment);
	// DEBUG DEBUG DEBUG
	m_shadowMaskImageView = m_graphics->CreateImageView("Shadow Mask View", m_shadowMaskImage);
	m_shadowMaskSampler = m_graphics->CreateSampler("Shadow Mask Sampler", samplerDescription);

	GraphicsFramebufferSettings frameBufferSettings;
	frameBufferSettings.width = m_swapChainWidth;
	frameBufferSettings.height = m_swapChainHeight;
	frameBufferSettings.renderPass = m_shadowMaskRenderPass;
	frameBufferSettings.attachments.push_back(m_shadowMaskImageView);

	m_shadowMaskFrameBuffer = m_graphics->CreateFramebuffer("Shadow Mask FrameBuffer", frameBufferSettings);
}

void Renderer::CreateLightingResources()
{
	// Create render pass.
	GraphicsRenderPassSettings renderPassSettings;
	renderPassSettings.transitionToPresentFormat = false;
	renderPassSettings.AddColorAttachment(GraphicsFormat::UNORM_R8, false);

	GraphicsSubPassIndex subPass1 = renderPassSettings.AddSubPass();
	renderPassSettings.AddSubPassDependency(GraphicsExternalPassIndex, GraphicsAccessMask::None, subPass1, GraphicsAccessMask::ReadWrite);

	m_lightAccumulationRenderPass = m_graphics->CreateRenderPass("Lighting Render Pass", renderPassSettings);

	SamplerDescription samplerDescription;

	// Create shadow mask buffer.
	m_lightAccumulationImage = m_graphics->CreateImage("Light Accumulation", m_swapChainWidth, m_swapChainHeight, 1, GraphicsFormat::UNORM_R8, false, GraphicsUsage::ColorAttachment);
	m_lightAccumulationImageView = m_graphics->CreateImageView("Light Accumulation View", m_lightAccumulationImage);
	m_lightAccumulationSampler = m_graphics->CreateSampler("Light Accumulation Sampler", samplerDescription);

	GraphicsFramebufferSettings frameBufferSettings;
	frameBufferSettings.width = m_swapChainWidth;
	frameBufferSettings.height = m_swapChainHeight;
	frameBufferSettings.renderPass = m_lightAccumulationRenderPass;
	frameBufferSettings.attachments.push_back(m_lightAccumulationImageView);

	m_lightAccumulationFrameBuffer = m_graphics->CreateFramebuffer("Light Accumulation FrameBuffer", frameBufferSettings);
}

void Renderer::CreateGBufferResources()
{
	// Create render pass.
	GraphicsRenderPassSettings renderPassSettings;
	renderPassSettings.transitionToPresentFormat = false;
	renderPassSettings.AddColorAttachment(GraphicsFormat::UNORM_R16G16B16A16, false);
	renderPassSettings.AddColorAttachment(GraphicsFormat::SFLOAT_R16G16B16A16, false);
	renderPassSettings.AddColorAttachment(GraphicsFormat::SFLOAT_R32G32B32A32, false);
	renderPassSettings.AddDepthAttachment(GraphicsFormat::UNORM_D24_UINT_S8);

	GraphicsSubPassIndex subPass1 = renderPassSettings.AddSubPass();
	renderPassSettings.AddSubPassDependency(GraphicsExternalPassIndex, GraphicsAccessMask::None, subPass1, GraphicsAccessMask::ReadWrite);

	m_gbufferRenderPass = m_graphics->CreateRenderPass("GBuffer Render Pass", renderPassSettings);

	// Create our G-Buffer.
	m_gbufferImages[0] = m_graphics->CreateImage("GBuffer 0 - RGB:Diffuse A:Flags", m_swapChainWidth, m_swapChainHeight, 1, GraphicsFormat::UNORM_R16G16B16A16, false, GraphicsUsage::ColorAttachment);
	m_gbufferImages[1] = m_graphics->CreateImage("GBuffer 1 - RGB:World Normal A:Unused", m_swapChainWidth, m_swapChainHeight, 1, GraphicsFormat::SFLOAT_R16G16B16A16, false, GraphicsUsage::ColorAttachment);
	m_gbufferImages[2] = m_graphics->CreateImage("GBuffer 2 - RGB:World Position A:Unused", m_swapChainWidth, m_swapChainHeight, 1, GraphicsFormat::SFLOAT_R32G32B32A32, false, GraphicsUsage::ColorAttachment);

	for (int i = 0; i < GBufferImageCount; i++)
	{
		m_gbufferViews[i] = m_graphics->CreateImageView(StringFormat("GBuffer %i View", i), m_gbufferImages[i]);

		SamplerDescription description;
		m_gbufferSamplers[i] = m_graphics->CreateSampler(StringFormat("GBuffer %i Sampler", i), description);
	}

	GraphicsFramebufferSettings gbufferFrameBufferSettings;
	gbufferFrameBufferSettings.width = m_swapChainWidth;
	gbufferFrameBufferSettings.height = m_swapChainHeight;
	gbufferFrameBufferSettings.renderPass = m_gbufferRenderPass;
	for (int i = 0; i < GBufferImageCount; i++)
	{
		gbufferFrameBufferSettings.attachments.push_back(m_gbufferViews[i]);
	}
	gbufferFrameBufferSettings.attachments.push_back(m_depthBufferView);

	m_gbufferFrameBuffer = m_graphics->CreateFramebuffer("GBuffer FrameBuffer", gbufferFrameBufferSettings);

	// Global properties.
	m_globalRenderProperties.Set(GBuffer0Hash, RenderPropertyImageSamplerValue(m_gbufferViews[0], m_gbufferSamplers[0]));
	m_globalRenderProperties.Set(GBuffer1Hash, RenderPropertyImageSamplerValue(m_gbufferViews[1], m_gbufferSamplers[1]));
	m_globalRenderProperties.Set(GBuffer2Hash, RenderPropertyImageSamplerValue(m_gbufferViews[2], m_gbufferSamplers[2]));
	m_globalRenderProperties.Set(ShadowMaskHash, RenderPropertyImageSamplerValue(m_shadowMaskImageView, m_shadowMaskSampler));
	m_globalRenderProperties.Set(LightAccumulationHash, RenderPropertyImageSamplerValue(m_lightAccumulationImageView, m_lightAccumulationSampler));
}

void Renderer::SwapChainModified()
{
	FreeSwapChainDependentResources();
	CreateSwapChainDependentResources();
}

std::shared_ptr<IGraphicsResourceSet>  Renderer::AllocateResourceSet(const GraphicsResourceSetDescription& set)
{
	return m_resourceSetPool->Allocate(set);
}

void Renderer::GeneratePreRender()
{
	ProfileScope scope(ProfileColors::Draw, "Renderer::GeneratePreRender");

	std::shared_ptr<IGraphicsCommandBuffer> buffer = RequestPrimaryBuffer();

	buffer->Reset();
	buffer->Begin();

	// Transition swap chain image to writeable.
	buffer->TransitionResource(m_swapChainViews[m_frameIndex]->GetImage(), GraphicsAccessMask::ReadWrite);
	//buffer->TransitionResource(m_depthBufferImage, GraphicsAccessMask::ReadWrite);

	RunQueuedCommands(RenderCommandStage::Global_PreRender, buffer);

	buffer->Clear(m_swapChainViews[m_frameIndex]->GetImage(), Color(0.1f, 0.1f, 0.1f, 1.0f), 1.0f, 0.0f);
	//buffer->Clear(m_depthBufferImage, Color(0.1f, 0.1f, 0.1f, 1.0f), 1.0f, 0.0f);

	buffer->End();

	QueuePrimaryBuffer("Main Pre-Render", RenderCommandStage::Global_PreRender, buffer);
}

void Renderer::GeneratePostRender()
{
	ProfileScope scope(ProfileColors::Draw, "Renderer::GeneratePostRender");

	std::shared_ptr<IGraphicsCommandBuffer> buffer = RequestPrimaryBuffer();

	buffer->Reset();
	buffer->Begin();

	RunQueuedCommands(RenderCommandStage::Global_PrePresent, buffer);

	buffer->End();

	QueuePrimaryBuffer("Main Post-Render", RenderCommandStage::Global_PrePresent, buffer);
}

void Renderer::GeneratePrePresent()
{
	ProfileScope scope(ProfileColors::Draw, "Renderer::GeneratePrePresent");

	std::shared_ptr<IGraphicsCommandBuffer> buffer = RequestPrimaryBuffer();

	buffer->Reset();
	buffer->Begin();
	
	buffer->TransitionResource(m_swapChainViews[m_frameIndex]->GetImage(), GraphicsAccessMask::Present);
	
	buffer->End();

	QueuePrimaryBuffer("Main Pre-Present", RenderCommandStage::Global_PrePresent, buffer);
}

void Renderer::DrawFullScreenQuad(
	std::shared_ptr<IGraphicsCommandBuffer> buffer, 
	std::shared_ptr<Material> material, 
	std::shared_ptr<MeshRenderState>* MeshRenderState,
	RenderPropertyCollection* viewProperties,
	RenderPropertyCollection* meshProperties,
	Rect viewport,
	Rect scissor)
{
	material->UpdateResources();

	if (viewport == Rect::Empty)
	{
		viewport = Rect(0.0f, 0.0f, 1.0f, 1.0f);
	}
	if (scissor == Rect::Empty)
	{
		scissor = Rect(0.0f, 0.0f, 1.0f, 1.0f);
	}

	// Create render property heirarchy.
	RenderPropertyHeirarchy renderHeirarchy;
	renderHeirarchy.Set(GraphicsBindingFrequency::Global, &m_globalRenderProperties);
	renderHeirarchy.Set(GraphicsBindingFrequency::View, viewProperties);
	renderHeirarchy.Set(GraphicsBindingFrequency::Material, &material->GetProperties());
	renderHeirarchy.Set(GraphicsBindingFrequency::Mesh, meshProperties);

	if (!m_fullscreenQuadsUploaded)
	{
		buffer->Upload(m_fullscreenQuadVertexBuffer);
		buffer->Upload(m_fullscreenQuadIndexBuffer);
		m_fullscreenQuadsUploaded = true;
	}

	const Array<std::shared_ptr<IGraphicsResourceSet>>& resourceSets = (*MeshRenderState)->UpdateAndGetResourceSets(material, &renderHeirarchy);

	buffer->BeginPass(material->GetRenderPass(), material->GetFrameBuffer());
	buffer->BeginSubPass();

	buffer->SetPipeline(material->GetPipeline());
	buffer->SetViewport(viewport.x * m_swapChainWidth, viewport.y * m_swapChainHeight, viewport.width * m_swapChainWidth, viewport.height * m_swapChainHeight);
	buffer->SetScissor(viewport.x * m_swapChainWidth, viewport.y * m_swapChainHeight, viewport.width * m_swapChainWidth, viewport.height * m_swapChainHeight);

	buffer->SetIndexBuffer(m_fullscreenQuadIndexBuffer);
	buffer->SetVertexBuffer(m_fullscreenQuadVertexBuffer);

	buffer->SetResourceSets(resourceSets.data(), (int)resourceSets.size());

	buffer->DrawIndexedElements(6, 1, 0, 0, 0);

	buffer->EndSubPass();
	buffer->EndPass();
}

void Renderer::Present()
{
	ProfileScope scope(ProfileColors::Draw, "Renderer::Present");

	// Allocate and build stage buffers.
	// todo: remove this stuff plz.
	GeneratePreRender();
	GeneratePostRender();
	GeneratePrePresent();

	// Sort queues.
	std::sort(m_queuedBuffers.begin(), m_queuedBuffers.end(),
		[](const QueuedBuffer& a, const QueuedBuffer& b) -> bool
	{
		if (((int)a.stage >= (int)RenderCommandStage::View_START && (int)a.stage <= (int)RenderCommandStage::View_END) &&
			((int)b.stage >= (int)RenderCommandStage::View_START && (int)b.stage <= (int)RenderCommandStage::View_END))
		{
			if (a.viewId == b.viewId)
			{
				if (a.stage == b.stage)
				{
					return a.renderOrder < b.renderOrder;
				}
				else
				{
					return (int)a.stage < (int)b.stage;
				}
			}
			else
			{
				return a.viewId < b.viewId;
			}
		}
		else
		{
			return (int)a.stage < (int)b.stage;
		}
	});

	// Dispatch all buffers.
	Array<std::shared_ptr<IGraphicsCommandBuffer>> buffers;

	Array<QueuedBuffer> queuedBuffers = m_queuedBuffers;
	
	//printf("==============================================\n");
	for (auto& buffer : queuedBuffers)
	{
		//printf("Buffer[%i:%i]: %s\n", (int)buffer.stage, (int)buffer.viewId, buffer.name.c_str());
		m_graphics->Dispatch(buffer.name, buffer.buffer);
	}

	m_queuedBuffers.clear();

	// Increment frame counter.
	m_frameCounter++;
	m_frameIndex = m_frameCounter % m_swapChainViews.size();

	// Present.
	if (m_graphics->Present())
	{
		SwapChainModified();
	}

	// Reset command buffers for this frame.
	UpdateCommandBufferPools();

	// Add gbuffers to debug framebuffer list.
	DisplayDebugFrameBuffer(m_shadowMaskImageView);
	DisplayDebugFrameBuffer(m_lightAccumulationImageView);
	for (int i = 0; i < GBufferImageCount; i++)
	{
		DisplayDebugFrameBuffer(m_gbufferViews[i]);
	}

	// Update properties.
	m_globalRenderProperties.UpdateResources(m_graphics, m_logger);
}

RenderPropertyCollection& Renderer::GetGlobalRenderProperties()
{
	return m_globalRenderProperties;
}

int Renderer::GetSwapChainWidth()
{
	return m_swapChainWidth;
}

int Renderer::GetSwapChainHeight()
{
	return m_swapChainHeight;
}

ResourcePtr<Shader> Renderer::GetDepthOnlyShader()
{
	return m_depthOnlyShader;
}

ResourcePtr<Shader> Renderer::GetNormalizedDistanceShader()
{
	return m_normalizedDistanceShader;
}

std::shared_ptr<IGraphicsFramebuffer> Renderer::GetCurrentFramebuffer()
{
	return m_swapChainFramebuffers[m_frameIndex];
}

bool Renderer::IsRenderingFrozen()
{
	return m_renderingIsFrozen;
}

bool Renderer::IsDrawBoundsEnabled()
{
	return m_drawBoundsEnabled;
}

bool Renderer::IsWireframeEnabled()
{
	return m_drawWireframeEnabled;
}

std::shared_ptr<IGraphicsRenderPass> Renderer::GetRenderPassForTarget(FrameBufferTarget target)
{
	switch (target)
	{
	case FrameBufferTarget::GBuffer:
		{
			return m_gbufferRenderPass;
		}
	case FrameBufferTarget::SwapChain:
		{
			return m_resolveToSwapChainRenderPass;
		}
	case FrameBufferTarget::DepthBuffer:
		{
			return m_depthOnlyRenderPass;
		}
	case FrameBufferTarget::ShadowMask:
		{
			return m_shadowMaskRenderPass;
		}
	}

	return nullptr;
}

std::shared_ptr<IGraphicsFramebuffer> Renderer::GetFramebufferForTarget(FrameBufferTarget target)
{
	switch (target)
	{
	case FrameBufferTarget::GBuffer:
		{
			return m_gbufferFrameBuffer;
		}
	case FrameBufferTarget::SwapChain:
		{
			return GetCurrentFramebuffer();
		}
	case FrameBufferTarget::DepthBuffer:
		{
			return nullptr;
		}
	case FrameBufferTarget::ShadowMask:
		{
			return m_shadowMaskFrameBuffer;
		}
	}

	return nullptr;
}

std::shared_ptr<Renderer::ThreadLocalCommandBufferPool>& Renderer::GetCommandBufferPoolForThread()
{
	ScopeLock lock(m_commandBufferPoolsMutex);

	if (m_commandBufferPoolIndexTls == -1)
	{
		m_commandBufferPoolIndexTls = (int)m_commandBufferPools.size();

		std::shared_ptr<ThreadLocalCommandBufferPool> pool = std::make_shared<ThreadLocalCommandBufferPool>();
		pool->pool = m_graphics->CreateCommandBufferPool(StringFormat("Thread Local Command Buffer %i", std::this_thread::get_id()));
		pool->frameData.resize(m_swapChainViews.size() + 1);		

		m_commandBufferPools.push_back(pool);
	}
	else
	{
		if (m_commandBufferPools[m_commandBufferPoolIndexTls] == nullptr)
		{
			std::shared_ptr<ThreadLocalCommandBufferPool> pool = std::make_shared<ThreadLocalCommandBufferPool>();
			pool->pool = m_graphics->CreateCommandBufferPool(StringFormat("Thread Local Command Buffer %i", std::this_thread::get_id()));
			pool->frameData.resize(m_swapChainViews.size() + 1);

			m_commandBufferPools[m_commandBufferPoolIndexTls] = pool;
		}
	}

	return m_commandBufferPools[m_commandBufferPoolIndexTls];
}

std::shared_ptr<IGraphicsCommandBuffer> Renderer::RequestSecondaryBuffer()
{
	std::shared_ptr<Renderer::ThreadLocalCommandBufferPool>& pool = GetCommandBufferPoolForThread();
	ScopeLock lock(pool->mutex);

	int frameIndex = m_frameCounter % pool->frameData.size();
	ThreadLocalCommandBufferPoolFrameData& frameData = pool->frameData[frameIndex];

	frameData.secondaryBuffersAllocated++;
	if (frameData.secondaryBuffersAllocated >= frameData.secondaryBuffers.size())
	{
		frameData.secondaryBuffers.push_back(pool->pool->Allocate(false));
	}

	return frameData.secondaryBuffers[frameData.secondaryBuffersAllocated - 1];
}

std::shared_ptr<IGraphicsCommandBuffer> Renderer::RequestPrimaryBuffer()
{
	std::shared_ptr<Renderer::ThreadLocalCommandBufferPool>& pool = GetCommandBufferPoolForThread();
	ScopeLock lock(pool->mutex);

	int frameIndex = m_frameCounter % pool->frameData.size();
	ThreadLocalCommandBufferPoolFrameData& frameData = pool->frameData[frameIndex];

	frameData.primaryBuffersAllocated++;
	if (frameData.primaryBuffersAllocated >= frameData.primaryBuffers.size())
	{
		frameData.primaryBuffers.push_back(pool->pool->Allocate(true));
	}

	return frameData.primaryBuffers[frameData.primaryBuffersAllocated - 1];
}

void Renderer::UpdateCommandBufferPools()
{
	ScopeLock lock(m_commandBufferPoolsMutex);

	for (auto& pool : m_commandBufferPools)
	{
		if (pool != nullptr)
		{
			ScopeLock lock(pool->mutex);

			int frameIndex = m_frameCounter % pool->frameData.size();
			pool->frameData[frameIndex].primaryBuffersAllocated = 0;
			pool->frameData[frameIndex].secondaryBuffersAllocated = 0;
		}
	}
}

void Renderer::QueuePrimaryBuffer(const String& name, RenderCommandStage stage, std::shared_ptr<IGraphicsCommandBuffer>& buffer, uint64_t viewId, float renderOrder)
{
	ScopeLock lock(m_queuedBuffersMutex);

	QueuedBuffer qbuffer;
	qbuffer.name = name;
	qbuffer.stage = stage;
	qbuffer.buffer = buffer;
	qbuffer.viewId = viewId;
	qbuffer.renderOrder = renderOrder;
	m_queuedBuffers.push_back(qbuffer);
}

ResourcePtr<Material> Renderer::GetResolveToSwapChainMaterial()
{
	return m_resolveToSwapchainMaterial;
}

std::shared_ptr<MeshRenderState>& Renderer::GetResolveToSwapChainRenderState()
{
	return m_resolveToSwapchainMeshRenderState;
}

ResourcePtr<Material> Renderer::GetClearGBufferMaterial()
{
	return m_clearGBufferMaterial;
}

std::shared_ptr<MeshRenderState>& Renderer::GetClearGBufferRenderState()
{
	return m_clearGBufferMeshRenderState;
}

std::shared_ptr<IGraphicsImage> Renderer::GetGBufferImage(int index)
{
	return m_gbufferImages[index];
}

std::shared_ptr<IGraphicsImage> Renderer::GetShadowMaskImage()
{
	return m_shadowMaskImage;
}

std::shared_ptr<IGraphicsImage> Renderer::GetLightAccumulationImage()
{
	return m_lightAccumulationImage;
}

std::shared_ptr<IGraphicsImage> Renderer::GetDepthImage()
{
	return m_depthBufferImage;
}

void Renderer::DrawView(DrawViewState& state)
{
	SpatialIndexSystem* spatialSystem = state.world->GetSystem<SpatialIndexSystem>();

	// Grab all visible entities from the oct-tree.
	{
		ProfileScope scope(ProfileColors::Draw, "Search OctTree");
		spatialSystem->GetTree().Get(state.frustum, state.visibleEntitiesResult, false);
	}

	// Batch up all meshes.
	{
		ProfileScope scope(ProfileColors::Draw, "Batch Meshes");
		state.meshBatcher.Batch(
			*state.world, 
			shared_from_this(), 
			m_logger, 
			m_graphics, 
			state.visibleEntitiesResult.entries, 
			state.materialVariant, 
			state.viewProperties, 
			state.requiredFlags, 
			state.excludedFlags
		);
	}

	// Generate command buffers for each batch.
	Array<MaterialRenderBatch*>& renderBatches = state.meshBatcher.GetBatches();

	state.batchBuffers.resize(renderBatches.size());

	ParallelFor((int)renderBatches.size(), [&](int index)
	{
		std::shared_ptr<IGraphicsCommandBuffer> drawBuffer = RequestPrimaryBuffer();
		state.batchBuffers[index] = drawBuffer;

		MaterialRenderBatch*& batch = renderBatches[index];

		ProfileScope scope(ProfileColors::Draw, "Render material batch: " + batch->material->GetName());

		// Generate drawing buffer.
		drawBuffer->Reset();
		drawBuffer->Begin();

		drawBuffer->BeginPass(batch->material->GetRenderPass(), state.framebuffer ? state.framebuffer : batch->material->GetFrameBuffer(), true);
		drawBuffer->BeginSubPass();

		if (IsWireframeEnabled())
		{
			drawBuffer->SetPipeline(batch->material->GetWireframePipeline());
		}
		else
		{
			drawBuffer->SetPipeline(batch->material->GetPipeline());
		}

		drawBuffer->SetScissor(
			static_cast<int>(state.viewport.x),
			static_cast<int>(state.viewport.y),
			static_cast<int>(state.viewport.width),
			static_cast<int>(state.viewport.height)
		);
		drawBuffer->SetViewport(
			static_cast<int>(state.viewport.x),
			static_cast<int>(state.viewport.y),
			static_cast<int>(state.viewport.width),
			static_cast<int>(state.viewport.height)
		);

		{
			ProfileScope scope(ProfileColors::Draw, "Draw Meshes");
			
			for (MeshInstance* instance = batch->firstMesh; instance != nullptr; instance = instance->nextMesh)
			{
				drawBuffer->SetIndexBuffer(*instance->indexBuffer);
				drawBuffer->SetVertexBuffer(*instance->vertexBuffer);
				drawBuffer->SetResourceSets(instance->resourceSets->data(), (int)instance->resourceSets->size());
				drawBuffer->DrawIndexedElements(instance->indexCount, 1, 0, 0, 0);
			}
		}

		drawBuffer->EndSubPass();
		drawBuffer->EndPass();

		drawBuffer->End();

	}, 1, "Command Buffer Creation");
	
	for (int i = 0; i < state.batchBuffers.size(); i++)
	{
		QueuePrimaryBuffer(state.name, state.stage, state.batchBuffers[i], state.viewId, renderBatches[i]->material->GetShader().Get()->GetProperties().RenderOrder);
	}
}