#include "Pch.h"

#include "Engine/Components/Mesh/MeshComponent.h"

#include "Engine/Rendering/Renderer.h"
#include "Engine/Rendering/RenderView.h"
#include "Engine/Rendering/RendererEnums.h"
#include "Engine/Profiling/Profiling.h"

#include "Engine/Engine/Logging.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/GraphicsResourceSet.h"

#include "Engine/UI/ImguiManager.h"

#include "Engine/Types/Math.h"

#include "Engine/Utilities/Statistic.h"
#include "Engine/Threading/ParallelFor.h"

const MaterialPropertyHash ModelMatrixHash = CalculateMaterialPropertyHash("ModelMatrix");
const MaterialPropertyHash ViewMatrixHash = CalculateMaterialPropertyHash("ViewMatrix");
const MaterialPropertyHash ProjectionMatrixHash = CalculateMaterialPropertyHash("ProjectionMatrix");
const MaterialPropertyHash CameraPositionHash = CalculateMaterialPropertyHash("CameraPosition");
const MaterialPropertyHash GBuffer0Hash = CalculateMaterialPropertyHash("GBuffer0");
const MaterialPropertyHash GBuffer1Hash = CalculateMaterialPropertyHash("GBuffer1");
const MaterialPropertyHash GBuffer2Hash = CalculateMaterialPropertyHash("GBuffer2");

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
	, m_drawGBufferEnabled(false)
	, m_drawWireframeEnabled(false)
	, m_drawBoundsEnabled(false)
	, m_renderingIsFrozen(false)
	, m_frameCounter(0)
{
	// Fill various global properties with dummies, saves showing errors on initial resource binding. 
	Matrix4 identityMatrix = Matrix4::Identity;
	m_globalMaterialProperties.Set(ViewMatrixHash, identityMatrix);
	m_globalMaterialProperties.Set(ProjectionMatrixHash, identityMatrix);
}

void Renderer::QueueRenderCommand(RenderCommandStage stage, RenderCommand::CommandSignature_t callback)
{
	std::lock_guard<std::recursive_mutex> lock(m_queuedRenderCommandsMutex);

	RenderCommand command;
	command.stage = stage;
	command.command = callback;

	m_queuedRenderCommands.push_back(command);
}

void Renderer::RunQueuedCommands(RenderCommandStage stage, std::shared_ptr<IGraphicsCommandBuffer> buffer)
{
	std::lock_guard<std::recursive_mutex> lock(m_queuedRenderCommandsMutex);

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

void Renderer::InitDebugMenus(std::shared_ptr<ImguiManager> manager)
{
	m_imguiManager = manager;
	
	m_debugMenuCallbackToken = m_imguiManager->RegisterCallback(ImguiCallback::MainMenu, [=] {

		if (ImGui::BeginMenu("Rendering"))
		{
			ImGui::MenuItem("Toggle G-Buffer", nullptr, &m_drawGBufferEnabled);
			ImGui::MenuItem("Toggle Wireframe", nullptr, &m_drawWireframeEnabled);
			ImGui::MenuItem("Toggle Statistics", nullptr, &m_drawStatisticsEnabled);
			ImGui::MenuItem("Toggle Bounds", nullptr, &m_drawBoundsEnabled);			
			ImGui::Separator();
			ImGui::MenuItem("Toggle Frozen Rendering", nullptr, &m_renderingIsFrozen);			
			ImGui::EndMenu();
		}
	});

	m_debugMenuCallbackToken = m_imguiManager->RegisterCallback(ImguiCallback::PostMainMenu, [=] {

		if (m_drawGBufferEnabled)
		{
			int imageSize = 300;
		
			ImGui::Begin("G-Buffer", &m_drawGBufferEnabled, ImGuiWindowFlags_NoResize);

			for (int i = 0; i < GBufferImageCount; i++)
			{
				ImTextureID textureId = manager->StoreImage(m_gbufferViews[i]);

				ImGui::Image(textureId, ImVec2(static_cast<float>(imageSize), static_cast<float>(imageSize)));
				ImGui::SameLine();
			}

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

	m_resourceManager->WaitUntilIdle();

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
	m_swapChainFramebuffers.clear();

	for (int i = 0; i < m_commandBufferPools.size(); i++)
	{
		m_commandBufferPools[i] = nullptr;
	}

	for (int i = 0; i < GBufferImageCount; i++)
	{
		m_gbufferImages[i] = nullptr;
		m_gbufferViews[i] = nullptr;
	}
	m_gbufferFrameBuffer = nullptr;
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
		renderPassSettings.AddDepthAttachment(GraphicsFormat::UNORM_D16);

		GraphicsSubPassIndex subPass1 = renderPassSettings.AddSubPass();
		renderPassSettings.AddSubPassDependency(GraphicsExternalPassIndex, GraphicsAccessMask::None, subPass1, GraphicsAccessMask::ReadWrite);

		m_depthOnlyRenderPass = m_graphics->CreateRenderPass("Depth Only Render Pass", renderPassSettings);
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

	CreateGBufferResources();

	m_frameIndex = 0;
	m_frameCounter = 0;
}

void Renderer::CreateGBufferResources()
{
	// Create render pass.
	GraphicsRenderPassSettings renderPassSettings;
	renderPassSettings.transitionToPresentFormat = false;
	renderPassSettings.AddColorAttachment(GraphicsFormat::UNORM_R8G8B8A8, false);
	renderPassSettings.AddColorAttachment(GraphicsFormat::UNORM_R8G8B8A8, false);
	renderPassSettings.AddColorAttachment(GraphicsFormat::UNORM_R8G8B8A8, false);
	renderPassSettings.AddDepthAttachment(GraphicsFormat::UNORM_D24_UINT_S8);

	GraphicsSubPassIndex subPass1 = renderPassSettings.AddSubPass();
	renderPassSettings.AddSubPassDependency(GraphicsExternalPassIndex, GraphicsAccessMask::None, subPass1, GraphicsAccessMask::ReadWrite);

	m_gbufferRenderPass = m_graphics->CreateRenderPass("GBuffer Render Pass", renderPassSettings);

	// Create our G-Buffer.
	m_gbufferImages[0] = m_graphics->CreateImage("GBuffer 0 - RGB:Diffuse A:Unused", m_swapChainWidth, m_swapChainHeight, 1, GraphicsFormat::UNORM_R8G8B8A8, false, GraphicsUsage::ColorAttachment);
	m_gbufferImages[1] = m_graphics->CreateImage("GBuffer 1 - RGB:World Normal A:Unused", m_swapChainWidth, m_swapChainHeight, 1, GraphicsFormat::UNORM_R8G8B8A8, false, GraphicsUsage::ColorAttachment);
	m_gbufferImages[2] = m_graphics->CreateImage("GBuffer 2 - RGB:World Position A:Unused", m_swapChainWidth, m_swapChainHeight, 1, GraphicsFormat::UNORM_R8G8B8A8, false, GraphicsUsage::ColorAttachment);

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
	m_globalMaterialProperties.Set(GBuffer0Hash, m_gbufferViews[0], m_gbufferSamplers[0]);
	m_globalMaterialProperties.Set(GBuffer1Hash, m_gbufferViews[1], m_gbufferSamplers[1]);
	m_globalMaterialProperties.Set(GBuffer2Hash, m_gbufferViews[2], m_gbufferSamplers[2]);
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

void Renderer::BuildCommandBuffer_PreRender(std::shared_ptr<IGraphicsCommandBuffer> buffer)
{
	ProfileScope scope(Color::Red, "Renderer::BuildCommandBuffer_PreRender");

	buffer->Reset();
	buffer->Begin();

	// Transition swap chain image to writeable.
	buffer->TransitionResource(m_swapChainViews[m_frameIndex]->GetImage(), GraphicsAccessMask::ReadWrite);
	buffer->TransitionResource(m_depthBufferImage, GraphicsAccessMask::ReadWrite);

	// Transition all g-buffer images so they can be written to.
	for (int i = 0; i < GBufferImageCount; i++)
	{
		buffer->TransitionResource(m_gbufferImages[i], GraphicsAccessMask::Write);
	}

	RunQueuedCommands(RenderCommandStage::PreRender, buffer);

	buffer->Clear(m_swapChainViews[m_frameIndex]->GetImage(), Color(0.1f, 0.1f, 0.1f, 1.0f), 1.0f, 0.0f);
	buffer->Clear(m_depthBufferImage, Color(0.1f, 0.1f, 0.1f, 1.0f), 1.0f, 0.0f);

	// Clear G-Buffer
	DrawFullScreenQuad(buffer, m_clearGBufferMaterial.Get(), &m_clearGBufferMaterialRenderData);

	buffer->End();
}

void Renderer::BuildCommandBuffer_PostRender(std::shared_ptr<IGraphicsCommandBuffer> buffer)
{
	ProfileScope scope(Color::Red, "Renderer::BuildCommandBuffer_PostRender");

	buffer->Reset();
	buffer->Begin();

	// Transition all g-buffer images so they can be read from.
	for (int i = 0; i < GBufferImageCount; i++)
	{
		buffer->TransitionResource(m_gbufferImages[i], GraphicsAccessMask::Read);
	}

	// Resolve to framebuffer.
	DrawFullScreenQuad(buffer, m_resolveToSwapchainMaterial.Get(), &m_resolveToSwapchainMaterialRenderData);

	RunQueuedCommands(RenderCommandStage::PostRender, buffer);

	buffer->End();
}

void Renderer::BuildCommandBuffer_PrePresent(std::shared_ptr<IGraphicsCommandBuffer> buffer)
{
	ProfileScope scope(Color::Red, "Renderer::BuildCommandBuffer_PrePresent");

	buffer->Reset();
	buffer->Begin();
	
	buffer->TransitionResource(m_swapChainViews[m_frameIndex]->GetImage(), GraphicsAccessMask::Present);
	
	buffer->End();
}

void Renderer::DrawFullScreenQuad(std::shared_ptr<IGraphicsCommandBuffer> buffer, std::shared_ptr<Material> material, std::shared_ptr<MaterialRenderData>* materialRenderData)
{
	material->UpdateResources();
	UpdateMaterialRenderData(materialRenderData, material, &m_globalMaterialProperties);

	if (!m_fullscreenQuadsUploaded)
	{
		buffer->Upload(m_fullscreenQuadVertexBuffer);
		buffer->Upload(m_fullscreenQuadIndexBuffer);
		m_fullscreenQuadsUploaded = true;
	}


	const Array<std::shared_ptr<IGraphicsResourceSet>>& resourceSets = (*materialRenderData)->GetResourceSets();

	buffer->BeginPass(material->GetRenderPass(), material->GetFrameBuffer());
	buffer->BeginSubPass();

	buffer->SetPipeline(material->GetPipeline());
	buffer->SetViewport(0, 0, m_swapChainWidth, m_swapChainHeight);
	buffer->SetScissor(0, 0, m_swapChainWidth, m_swapChainHeight);

	buffer->SetIndexBuffer(m_fullscreenQuadIndexBuffer);
	buffer->SetVertexBuffer(m_fullscreenQuadVertexBuffer);

	buffer->SetResourceSets(resourceSets.data(), resourceSets.size());

	buffer->DrawIndexedElements(6, 1, 0, 0, 0);

	buffer->EndSubPass();
	buffer->EndPass();
}

void Renderer::Present()
{
	ProfileScope scope(Color::Red, "Renderer::Present");

	// Allocate and build a pre-render command buffer.
	std::shared_ptr<IGraphicsCommandBuffer> preRenderCommandBuffer = RequestPrimaryBuffer();
	BuildCommandBuffer_PreRender(preRenderCommandBuffer);
	QueuePrimaryBuffer("Main Pre-Render", RenderCommandStage::PreRender, preRenderCommandBuffer);

	// Allocate and build a post-render command buffer.
	std::shared_ptr<IGraphicsCommandBuffer> postRenderCommandBuffer = RequestPrimaryBuffer();
	BuildCommandBuffer_PostRender(postRenderCommandBuffer);
	QueuePrimaryBuffer("Main Post-Render", RenderCommandStage::PostRender, postRenderCommandBuffer);

	// Allocate and build a pre-render command buffer.
	std::shared_ptr<IGraphicsCommandBuffer> prePresentCommandBuffer = RequestPrimaryBuffer();
	BuildCommandBuffer_PrePresent(prePresentCommandBuffer);
	QueuePrimaryBuffer("Main Pre-Present", RenderCommandStage::PrePresent, prePresentCommandBuffer);

	// Sort queues.
	std::sort(m_queuedBuffers.begin(), m_queuedBuffers.end(),
		[](const QueuedBuffer& a, const QueuedBuffer& b) -> bool
	{
		return (int)a.stage < (int)b.stage;
	});

	// Dispatch all buffers.
	Array<std::shared_ptr<IGraphicsCommandBuffer>> buffers;

	Array<QueuedBuffer> queuedBuffers = m_queuedBuffers;
	m_queuedBuffers.clear();

	for (auto& buffer : queuedBuffers)
	{
		m_graphics->Dispatch(buffer.buffer);
	}

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
}

std::shared_ptr<IGraphicsUniformBuffer> Renderer::RegisterGlobalUniformBuffer(const UniformBufferLayout& layout)
{
	// Already exists?
	std::shared_ptr<IGraphicsUniformBuffer> originalBuffer = GetGlobalUniformBuffer(layout.HashCode);
	if (originalBuffer != nullptr)
	{
		return originalBuffer;
	}

	int dataSize = layout.GetSize();

	GraphicsResourceSetDescription description;
	description.AddBinding(layout.Name, 0, GraphicsBindingType::UniformBufferObject);

	GlobalUniformBuffer buffer;
	buffer.layout = layout;
	buffer.buffer = m_graphics->CreateUniformBuffer(StringFormat("Global UBO (%s)", layout.Name.c_str()), dataSize);

	m_globalUniformBuffers.emplace(layout.HashCode, buffer);

	return buffer.buffer;
}

std::shared_ptr<IGraphicsUniformBuffer> Renderer::GetGlobalUniformBuffer(uint64_t hashCode)
{
	auto iter = m_globalUniformBuffers.find(hashCode);
	if (iter == m_globalUniformBuffers.end())
	{
		return nullptr;
	}

	return iter->second.buffer;
}

std::shared_ptr<IGraphicsResourceSet> Renderer::RegisterGlobalResourceSet(const MaterialResourceSet& set)
{
	// Already exists?
	std::shared_ptr<IGraphicsResourceSet> originalBuffer = GetGlobalResourceSet(set.hashCode);
	if (originalBuffer != nullptr)
	{
		return originalBuffer;
	}

	GlobalResourceSet buffer;
	buffer.description = set;

	m_globalResourceSets.emplace(set.hashCode, buffer);

	return buffer.description.set;
}

std::shared_ptr<IGraphicsResourceSet> Renderer::GetGlobalResourceSet(uint64_t hashCode)
{
	auto iter = m_globalResourceSets.find(hashCode);
	if (iter == m_globalResourceSets.end())
	{
		return nullptr;
	}

	return iter->second.description.set;
}

void Renderer::UpdateGlobalResources()
{
	UpdateGlobalUniformBuffers();
	UpdateGlobalResourceSets();
}

void Renderer::UpdateGlobalResourceSets()
{
	Array<std::shared_ptr<IGraphicsUniformBuffer>> emptyUboList;

	for (auto& set : m_globalResourceSets)
	{
		set.second.description.UpdateBindings(
			shared_from_this(),
			m_logger,
			emptyUboList,
			&m_globalMaterialProperties,
			nullptr,
			set.second.description.set);
	}
}

void Renderer::UpdateGlobalUniformBuffers()
{
	MaterialPropertyCollection* propCollections[1] = {
		&m_globalMaterialProperties
	};

	for (auto& ubo : m_globalUniformBuffers)
	{
		ubo.second.layout.FillBuffer(m_logger, ubo.second.buffer, propCollections, 1);
	}
}

MaterialPropertyCollection& Renderer::GetGlobalMaterialProperties()
{
	return m_globalMaterialProperties;
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
	}

	return nullptr;
}

void Renderer::UpdateMaterialRenderData(std::shared_ptr<MaterialRenderData>* data, const std::shared_ptr<Material>& material, MaterialPropertyCollection* collection)
{
	if (*data == nullptr)
	{
		*data = std::make_shared<MaterialRenderData>(m_logger, shared_from_this(), m_graphics);
	}

	(*data)->Update(material, collection);
}

std::shared_ptr<Renderer::ThreadLocalCommandBufferPool>& Renderer::GetCommandBufferPoolForThread()
{
	std::lock_guard<std::mutex> lock(m_commandBufferPoolsMutex);

	if (m_commandBufferPoolIndexTls == -1)
	{
		m_commandBufferPoolIndexTls = m_commandBufferPools.size();

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
	std::lock_guard<std::mutex> lock(pool->mutex);

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
	std::lock_guard<std::mutex> lock(pool->mutex);

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
	std::lock_guard<std::mutex> lock(m_commandBufferPoolsMutex);

	for (auto& pool : m_commandBufferPools)
	{
		if (pool != nullptr)
		{
			std::lock_guard<std::mutex> lock(pool->mutex);

			int frameIndex = m_frameCounter % pool->frameData.size();
			pool->frameData[frameIndex].primaryBuffersAllocated = 0;
			pool->frameData[frameIndex].secondaryBuffersAllocated = 0;
		}
	}
}

void Renderer::QueuePrimaryBuffer(const String& name, RenderCommandStage stage, std::shared_ptr<IGraphicsCommandBuffer>& buffer)
{
	std::lock_guard<std::mutex> lock(m_queuedBuffersMutex);

	QueuedBuffer qbuffer;
	qbuffer.name = name;
	qbuffer.stage = stage;
	qbuffer.buffer = buffer;
	m_queuedBuffers.push_back(qbuffer);
}
