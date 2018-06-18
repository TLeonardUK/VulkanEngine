#include "Engine/Rendering/Renderer.h"
#include "Engine/Rendering/RenderView.h"
#include "Engine/Rendering/RendererEnums.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/GraphicsResourceSet.h"

#include "Engine/UI/ImguiManager.h"

// batch everything using same material

static const MaterialPropertyHash ModelMatrixHash = CalculateMaterialPropertyHash("ModelMatrix");
static const MaterialPropertyHash ViewMatrixHash = CalculateMaterialPropertyHash("ViewMatrix");
static const MaterialPropertyHash ProjectionMatrixHash = CalculateMaterialPropertyHash("ProjectionMatrix");
static const MaterialPropertyHash CameraPositionHash = CalculateMaterialPropertyHash("CameraPosition");
static const MaterialPropertyHash GBuffer0Hash = CalculateMaterialPropertyHash("GBuffer0");
static const MaterialPropertyHash GBuffer1Hash = CalculateMaterialPropertyHash("GBuffer1");
static const MaterialPropertyHash GBuffer2Hash = CalculateMaterialPropertyHash("GBuffer2");

Renderer::Renderer(std::shared_ptr<IGraphics> graphics)
	: m_graphics(graphics)
	, m_frameIndex(0)
	, m_drawGBufferEnabled(false)
	, m_drawWireframeEnabled(false)
{
	// Fill various global properties with dummies, saves showing errors on initial resource binding. 
	Matrix4 identityMatrix = Matrix4(1.0);
	m_globalMaterialProperties.Set(ModelMatrixHash, identityMatrix);
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
			ImGui::MenuItem("Show G-Buffer", nullptr, &m_drawGBufferEnabled);
			ImGui::MenuItem("Toggle Wireframe", nullptr, &m_drawWireframeEnabled);
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

	});
}

void Renderer::Dispose()
{
	m_imguiManager->UnregisterCallback(m_debugMenuCallbackToken);
	
	FreeResources();
}

void Renderer::CreateResources()
{
	m_commandBufferPool = m_graphics->CreateCommandBufferPool("Main Command Buffer Pool");
	m_resourceSetPool = m_graphics->CreateResourceSetPool("Main Resource Set Pool");

	// todo: we should abstract this all into rendering-task classes.
	m_resolveToSwapchainMaterial = m_resourceManager->Load<Material>("Engine/Materials/resolve_to_swapchain.json");
	m_resolveToSwapchainMaterial.WaitUntilLoaded();
	m_clearGBufferMaterial = m_resourceManager->Load<Material>("Engine/Materials/clear_gbuffer.json");
	m_clearGBufferMaterial.WaitUntilLoaded();

	VertexBufferBindingDescription description;
	m_resolveToSwapchainMaterial.Get()->GetVertexBufferFormat(description);
	m_fullscreenQuadVertexBuffer = m_graphics->CreateVertexBuffer("Full Screen Quad Vertex Buffer", description, 4);
	m_fullscreenQuadIndexBuffer = m_graphics->CreateIndexBuffer("Full Screen Quad Index Buffer", sizeof(uint16_t), 6);
	m_fullscreenQuadsUploaded = false;

	// todo: I do not like this manual vertex binding, may cause issues with
	// renderers with non-c style packing. We should just create a model out of the 
	// data and allow that to deal with platform-specific issues.
	struct Vertex
	{
		Vector2 position;
		Vector2 uv;
	};

	Array<Vertex> fullscreenQuadVerts;
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

	m_fullscreenQuadVertexBuffer->Stage(fullscreenQuadVerts.data(), 0, sizeof(Vertex) * fullscreenQuadVerts.size());
	m_fullscreenQuadIndexBuffer->Stage(fullscreenQuadIndices.data(), 0, sizeof(uint16_t) * fullscreenQuadIndices.size());

	CreateSwapChainDependentResources();
}

void Renderer::FreeResources()
{
	m_commandBufferPool = nullptr;

	FreeSwapChainDependentResources();
}

void Renderer::FreeSwapChainDependentResources()
{
	m_depthBufferImage = nullptr;
	m_depthBufferView = nullptr;
	m_resolveToSwapChainRenderPass = nullptr;
	m_swapChainFramebuffers.clear();
	m_commandBuffers.clear();

	for (int i = 0; i < GBufferImageCount; i++)
	{
		m_gbufferImages[i] = nullptr;
		m_gbufferViews[i] = nullptr;
	}
	m_gbufferFrameBuffer = nullptr;
}

void Renderer::CreateSwapChainDependentResources()
{
	// Create render pass.
	GraphicsRenderPassSettings renderPassSettings;
	renderPassSettings.AddColorAttachment(m_graphics->GetSwapChainFormat(), true);
	renderPassSettings.AddDepthAttachment(GraphicsFormat::UNORM_D24_UINT_S8);

	GraphicsSubPassIndex subPass1 = renderPassSettings.AddSubPass();
	renderPassSettings.AddSubPassDependency(GraphicsExternalPassIndex, GraphicsAccessMask::None, subPass1, GraphicsAccessMask::ReadWrite);

	m_resolveToSwapChainRenderPass = m_graphics->CreateRenderPass("Swap Chain Render Pass", renderPassSettings);

	m_swapChainViews = m_graphics->GetSwapChainViews();
	m_swapChainWidth = m_swapChainViews[0]->GetWidth();
	m_swapChainHeight = m_swapChainViews[0]->GetHeight();

	// Create depth buffer.
	m_depthBufferImage = m_graphics->CreateImage("Depth Buffer", m_swapChainWidth, m_swapChainHeight, 1, GraphicsFormat::UNORM_D24_UINT_S8, false);
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

	// Create command buffer.
	m_commandBuffers.resize(m_swapChainViews.size());
	for (int i = 0; i < m_commandBuffers.size(); i++)
	{
		m_commandBuffers[i] = m_commandBufferPool->Allocate();
	}

	CreateGBufferResources();
}

void Renderer::CreateGBufferResources()
{
	// Create render pass.
	GraphicsRenderPassSettings renderPassSettings;
	renderPassSettings.AddColorAttachment(GraphicsFormat::UNORM_R8G8B8A8, false);
	renderPassSettings.AddColorAttachment(GraphicsFormat::UNORM_R8G8B8A8, false);
	renderPassSettings.AddColorAttachment(GraphicsFormat::UNORM_R8G8B8A8, false);
	renderPassSettings.AddDepthAttachment(GraphicsFormat::UNORM_D24_UINT_S8);

	GraphicsSubPassIndex subPass1 = renderPassSettings.AddSubPass();
	renderPassSettings.AddSubPassDependency(GraphicsExternalPassIndex, GraphicsAccessMask::None, subPass1, GraphicsAccessMask::ReadWrite);

	m_gbufferRenderPass = m_graphics->CreateRenderPass("GBuffer Render Pass", renderPassSettings);

	// Create our G-Buffer.
	m_gbufferImages[0] = m_graphics->CreateImage("GBuffer 0 - RGB:Diffuse A:Unused", m_swapChainWidth, m_swapChainHeight, 1, GraphicsFormat::UNORM_R8G8B8A8, false);
	m_gbufferImages[1] = m_graphics->CreateImage("GBuffer 1 - RGB:World Normal A:Unused", m_swapChainWidth, m_swapChainHeight, 1, GraphicsFormat::UNORM_R8G8B8A8, false);
	m_gbufferImages[2] = m_graphics->CreateImage("GBuffer 2 - RGB:World Position A:Unused", m_swapChainWidth, m_swapChainHeight, 1, GraphicsFormat::UNORM_R8G8B8A8, false);

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

void Renderer::BuildCommandBuffer(std::shared_ptr<IGraphicsCommandBuffer> buffer)
{
	buffer->Reset();
	buffer->Begin();

	RunQueuedCommands(RenderCommandStage::PreRender, buffer);

	buffer->Clear(m_swapChainViews[m_frameIndex]->GetImage(), Color(0.1f, 0.1f, 0.1f, 1.0f), 1.0f, 0.0f);
	buffer->Clear(m_depthBufferImage, Color(0.1f, 0.1f, 0.1f, 1.0f), 1.0f, 0.0f);

	// Clear G-Buffer
	DrawFullScreenQuad(buffer, m_clearGBufferMaterial.Get());

	// TODO: Z Pre Pass

	// Render all views to gbuffer.
	for (auto& view : m_renderViews)
	{
		BuildViewCommandBuffer(view, buffer);
	}

	// pipeline sync here.

	// TODO: Light accumulation.

	// resolve gbuffer layouts to read access?

	// Resolve to framebuffer.
	DrawFullScreenQuad(buffer, m_resolveToSwapchainMaterial.Get());

	RunQueuedCommands(RenderCommandStage::PostViewsRendered, buffer);

	buffer->End();
}

void Renderer::DrawFullScreenQuad(std::shared_ptr<IGraphicsCommandBuffer> buffer, std::shared_ptr<Material> material)
{
	material->UpdateResources();

	if (!m_fullscreenQuadsUploaded)
	{
		buffer->Upload(m_fullscreenQuadVertexBuffer);
		buffer->Upload(m_fullscreenQuadIndexBuffer);
		m_fullscreenQuadsUploaded = true;
	}

	buffer->BeginPass(material->GetRenderPass(), material->GetFrameBuffer());
	buffer->BeginSubPass();

	buffer->SetPipeline(material->GetPipeline());
	buffer->SetViewport(0, 0, m_swapChainWidth, m_swapChainHeight);
	buffer->SetScissor(0, 0, m_swapChainWidth, m_swapChainHeight);

	buffer->SetIndexBuffer(m_fullscreenQuadIndexBuffer);
	buffer->SetVertexBuffer(m_fullscreenQuadVertexBuffer);
	buffer->SetResourceSets({ material->GetResourceSet() });

	buffer->DrawIndexedElements(6, 1, 0, 0, 0);

	buffer->EndSubPass();
	buffer->EndPass();
}

void Renderer::BuildViewCommandBuffer(std::shared_ptr<RenderView> view, std::shared_ptr<IGraphicsCommandBuffer> buffer)
{
	Matrix4 identityMatrix = Matrix4(1.0);

	m_globalMaterialProperties.Set(ModelMatrixHash, identityMatrix);
	m_globalMaterialProperties.Set(ViewMatrixHash, view->ViewMatrix);
	m_globalMaterialProperties.Set(ProjectionMatrixHash, view->ProjectionMatrix);
	m_globalMaterialProperties.Set(CameraPositionHash, view->Location);

	// Draw each model to view.
	for (auto& modelResource : m_tmpModelToRender)
	{
		std::shared_ptr<Model> model = modelResource.Get();
		if (model != nullptr)
		{
			model->UpdateResources();

			Array<std::shared_ptr<Mesh>> meshes = model->GetMeshes();

			for (auto& mesh : meshes)
			{
				std::shared_ptr<Material> material = mesh->GetMaterial().Get();

				buffer->BeginPass(material->GetRenderPass(), material->GetFrameBuffer());
				buffer->BeginSubPass();

				if (m_drawWireframeEnabled)
				{
					buffer->SetPipeline(material->GetWireframePipeline());
				}
				else
				{
					buffer->SetPipeline(material->GetPipeline());
				}
				buffer->SetScissor(
					static_cast<int>(view->Viewport.x),
					static_cast<int>(view->Viewport.y),
					static_cast<int>(view->Viewport.width),
					static_cast<int>(view->Viewport.height)
				);
				buffer->SetViewport(
					static_cast<int>(view->Viewport.x), 
					static_cast<int>(view->Viewport.y),
					static_cast<int>(view->Viewport.width),
					static_cast<int>(view->Viewport.height)
				);

				buffer->SetIndexBuffer(mesh->GetIndexBuffer());
				buffer->SetVertexBuffer(mesh->GetVertexBuffer());
				buffer->SetResourceSets({ material->GetResourceSet() });
				buffer->DrawIndexedElements(mesh->GetIndexCount(), 1, 0, 0, 0);

				buffer->EndSubPass();
				buffer->EndPass();
			}
		}
	}
}

void Renderer::Present()
{
	std::shared_ptr<IGraphicsCommandBuffer> commandBuffer = m_commandBuffers[m_frameIndex];
	BuildCommandBuffer(commandBuffer);
	m_graphics->Dispatch(commandBuffer);

	m_frameIndex = (m_frameIndex + 1) % m_commandBuffers.size();

	if (m_graphics->Present())
	{
		SwapChainModified();
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

std::shared_ptr<IGraphicsFramebuffer> Renderer::GetCurrentFramebuffer()
{
	return m_swapChainFramebuffers[m_frameIndex];
}

void Renderer::TmpAddModelToRender(ResourcePtr<Model> model)
{
	m_tmpModelToRender.push_back(model);
}

void Renderer::AddView(std::shared_ptr<RenderView> view)
{
	m_renderViews.push_back(view);
}

void Renderer::RemoveView(std::shared_ptr<RenderView> view)
{
	auto iter = std::find(m_renderViews.begin(), m_renderViews.end(), view);
	if (iter != m_renderViews.end())
	{
		m_renderViews.erase(iter);
	}
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
	}

	return nullptr;
}