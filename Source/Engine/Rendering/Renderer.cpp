#include "Pch.h"

#include "Engine/Rendering/Renderer.h"
#include "Engine/Rendering/RenderView.h"
#include "Engine/Rendering/RendererEnums.h"
#include "Engine/Profiling/Profiling.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/GraphicsResourceSet.h"
#include "Engine/Graphics/GraphicsResourceSetInstance.h"

#include "Engine/UI/ImguiManager.h"

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
	, m_drawBoundsEnabled(false)
	, m_renderingIsFrozen(false)
	, m_frameCounter(0)
	, m_statMeshesRendered(0)
	, m_statBatchesRendered(0)
	, m_statTrianglesRendered(0)
{
	// Fill various global properties with dummies, saves showing errors on initial resource binding. 
	Matrix4 identityMatrix = Matrix4::Identity;
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
			ImGui::Begin("Statistics", &m_drawStatisticsEnabled);
			ImGui::Columns(2);
			ImGui::Text("Meshes:");		ImGui::NextColumn();	ImGui::Text("%i", m_statMeshesRendered);		ImGui::NextColumn();
			ImGui::Text("Batches:");	ImGui::NextColumn();	ImGui::Text("%i", m_statBatchesRendered);		ImGui::NextColumn();
			ImGui::Text("Triangles:");	ImGui::NextColumn();	ImGui::Text("%i", m_statTrianglesRendered);		ImGui::NextColumn();
			ImGui::Columns();
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
	m_clearGBufferMaterial = m_resourceManager->Load<Material>("Engine/Materials/clear_gbuffer.json");
	m_debugLineMaterial = m_resourceManager->Load<Material>("Engine/Materials/debug_line.json");
	m_debugLineMaterial.WaitUntilLoaded();

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
	renderPassSettings.transitionToPresentFormat = true;
	renderPassSettings.AddColorAttachment(m_graphics->GetSwapChainFormat(), true);
	renderPassSettings.AddDepthAttachment(GraphicsFormat::UNORM_D24_UINT_S8);

	GraphicsSubPassIndex subPass1 = renderPassSettings.AddSubPass();
	renderPassSettings.AddSubPassDependency(GraphicsExternalPassIndex, GraphicsAccessMask::None, subPass1, GraphicsAccessMask::ReadWrite);

	m_resolveToSwapChainRenderPass = m_graphics->CreateRenderPass("Swap Chain Render Pass", renderPassSettings);

	m_swapChainViews = m_graphics->GetSwapChainViews();
	m_swapChainWidth = m_swapChainViews[0]->GetWidth();
	m_swapChainHeight = m_swapChainViews[0]->GetHeight();

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

	// Create command buffer.
	m_commandBuffers.resize(m_swapChainViews.size() + 1); // swap chain views in flight + 1 being written.
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

void Renderer::BuildCommandBuffer(std::shared_ptr<IGraphicsCommandBuffer> buffer)
{
	ProfileScope scope(Color::Red, "Renderer::BuildCommandBuffer");

	buffer->Reset();
	buffer->Begin();

	RunQueuedCommands(RenderCommandStage::PreRender, buffer);

	buffer->Clear(m_swapChainViews[m_frameIndex]->GetImage(), Color(0.1f, 0.1f, 0.1f, 1.0f), 1.0f, 0.0f);
	buffer->Clear(m_depthBufferImage, Color(0.1f, 0.1f, 0.1f, 1.0f), 1.0f, 0.0f);

	// Clear G-Buffer
	DrawFullScreenQuad(buffer, m_clearGBufferMaterial.Get());

	// Render all views to gbuffer.
	for (auto& view : m_renderViews)
	{
		BuildViewCommandBuffer(view, buffer);
	}

	// Resolve to framebuffer.
	DrawFullScreenQuad(buffer, m_resolveToSwapchainMaterial.Get());

	RunQueuedCommands(RenderCommandStage::PostViewsRendered, buffer);

	buffer->End();

	// Clear frame-specific render views.
	for (auto& iter : m_renderViews)
	{
		m_freeRenderViews.push_back(iter);
	}
	m_renderViews.clear();
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

	buffer->TransitionResourceSets(&material->GetResourceSet(), 1);

	buffer->BeginPass(material->GetRenderPass(), material->GetFrameBuffer());
	buffer->BeginSubPass();

	buffer->SetPipeline(material->GetPipeline());
	buffer->SetViewport(0, 0, m_swapChainWidth, m_swapChainHeight);
	buffer->SetScissor(0, 0, m_swapChainWidth, m_swapChainHeight);

	buffer->SetIndexBuffer(m_fullscreenQuadIndexBuffer);
	buffer->SetVertexBuffer(m_fullscreenQuadVertexBuffer);
	buffer->SetResourceSetInstances(&material->GetResourceSet()->ConsumeInstance(), 1);

	buffer->DrawIndexedElements(6, 1, 0, 0, 0);

	buffer->EndSubPass();
	buffer->EndPass();
}

void Renderer::BuildViewCommandBuffer_Meshes(std::shared_ptr<RenderView> view, std::shared_ptr<IGraphicsCommandBuffer> buffer)
{
	ProfileScope scope(Color::Red, "Renderer::BuildViewCommandBuffer_Meshes");

	m_globalMaterialProperties.Set(ModelMatrixHash, Matrix4::Identity);
	m_globalMaterialProperties.Set(ViewMatrixHash, view->ViewMatrix);
	m_globalMaterialProperties.Set(ProjectionMatrixHash, view->ProjectionMatrix);
	m_globalMaterialProperties.Set(CameraPositionHash, view->CameraPosition);

	// Batch meshes by material.

	{
		ProfileScope scope(Color::Red, "Batch meshes");

		for (auto& iter : m_materialBatches)
		{
			iter.second.meshInstanceCount = 0;
			if (iter.second.meshInstances.size() < view->Meshes.size())
			{
				iter.second.meshInstances.resize(view->Meshes.size());
			}
		}

		for (auto& visibleMesh : view->Meshes)
		{
			Material* material = &*visibleMesh.Mesh->GetMaterial().Get();
			MaterialBatch* batch = nullptr;

			auto batchIter = m_materialBatches.find(material);
			if (batchIter != m_materialBatches.end())
			{
				batch = &batchIter->second;
			}
			else
			{
				m_materialBatches.emplace(material, MaterialBatch());

				batch = &m_materialBatches[material];
				batch->material = material;
				batch->meshInstanceCount = 0;
				batch->meshInstances.resize(view->Meshes.size());
			}

			// Update mesh state.
			m_globalMaterialProperties.Set(ModelMatrixHash, visibleMesh.Matrix);
			{
				ProfileScope scope(Color::Red, "Update Resources");
				visibleMesh.Mesh->UpdateResources();
			}

			// Create new instance.
			MeshInstance& instance = batch->meshInstances[batch->meshInstanceCount++];
			instance.resourceSet = material->GetResourceSet();
			instance.resourceSetInstance = instance.resourceSet->ConsumeInstance();
			instance.indexBuffer = visibleMesh.Mesh->GetIndexBuffer();
			instance.vertexBuffer = visibleMesh.Mesh->GetVertexBuffer();
			instance.indexCount = visibleMesh.Mesh->GetIndexCount();

			m_statMeshesRendered++;
			m_statTrianglesRendered += instance.indexCount / 3; // Not stricly correct if we are using non triangle-list topologies, but close enough estimate.
		}
	}

	// Draw each batch.
	for (auto& batchIter : m_materialBatches)
	{
		Material* material = batchIter.first;
		MaterialBatch& batch = batchIter.second;

		if (batch.meshInstanceCount == 0)
		{
			continue;
		}

		ProfileScope scope(Color::Red, "Render material batch: " + material->m_name);

		{
			ProfileScope scope(Color::Red, "Transition Resources");

			for (int i = 0; i < batch.meshInstanceCount; i++)
			{
				buffer->TransitionResourceSets(&batch.meshInstances[i].resourceSet, 1);
			}
		}

		{
			ProfileScope scope(Color::Red, "Begin Pass");

			buffer->BeginPass(material->GetRenderPass(), material->GetFrameBuffer());
			buffer->BeginSubPass();
		}

		{
			ProfileScope scope(Color::Red, "Pipeline Setup");

			if (m_drawWireframeEnabled)
			{
				buffer->SetPipeline(material->GetWireframePipeline());
			}
			else
			{
				buffer->SetPipeline(material->GetPipeline());
			}
		}

		{
			ProfileScope scope(Color::Red, "Viewport Setup");

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
		}

		{
			ProfileScope scope(Color::Red, "Render meshes");

			for (int i = 0; i < batch.meshInstanceCount; i++)
			{
				MeshInstance& instance = batch.meshInstances[i];
				buffer->SetIndexBuffer(instance.indexBuffer);
				buffer->SetVertexBuffer(instance.vertexBuffer);
				buffer->SetResourceSetInstances(&instance.resourceSetInstance, 1);
				buffer->DrawIndexedElements(instance.indexCount, 1, 0, 0, 0);
			}
		}

		{
			ProfileScope scope(Color::Red, "End Pass");

			buffer->EndSubPass();
			buffer->EndPass();
		}

		{
			ProfileScope scope(Color::Red, "Cleanup");

			m_statBatchesRendered++;

			// Clear out old instance data.
			/*for (int i = batch.meshInstanceCount; i < batch.meshInstances.size(); i++)
			{
				MeshInstance& instance = batch.meshInstances[i];
				instance.resourceSet = nullptr;
				instance.resourceSetInstance = nullptr;
				instance.indexBuffer = nullptr;
				instance.vertexBuffer = nullptr;
			}*/
		}
	}

	/*
	for (auto& visibleMesh : view->Meshes)
	{
		m_globalMaterialProperties.Set(ModelMatrixHash, visibleMesh.Matrix);

		visibleMesh.Mesh->UpdateResources();

		std::shared_ptr<Material> material = visibleMesh.Mesh->GetMaterial().Get();
		buffer->TransitionResourceSets({ material->GetResourceSet() });

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

		buffer->SetIndexBuffer(visibleMesh.Mesh->GetIndexBuffer());
		buffer->SetVertexBuffer(visibleMesh.Mesh->GetVertexBuffer());
		buffer->SetResourceSets({ material->GetResourceSet() });
		buffer->DrawIndexedElements(visibleMesh.Mesh->GetIndexCount(), 1, 0, 0, 0);

		buffer->EndSubPass();
		buffer->EndPass();
	}*/
}

void Renderer::BuildViewCommandBuffer_Lines(std::shared_ptr<RenderView> view, std::shared_ptr<IGraphicsCommandBuffer> buffer)
{
	ProfileScope scope(Color::Red, "Renderer::BuildViewCommandBuffer_Lines");

	m_globalMaterialProperties.Set(ModelMatrixHash, Matrix4::Identity);

	Array<uint16_t> lineIndices;
	Array<DebugLineVertex> lineVerts;
	lineVerts.resize(view->Lines.size() * 2);
	lineIndices.resize(view->Lines.size() * 2);

	for (size_t i = 0; i < view->Lines.size(); i++)
	{
		RenderLine& line = view->Lines[i];

		lineVerts[(i * 2) + 0].position = line.Start;
		lineVerts[(i * 2) + 0].color = line.LineColor.ToVector();
		lineVerts[(i * 2) + 1].position = line.End;
		lineVerts[(i * 2) + 1].color = line.LineColor.ToVector();

		lineIndices[(i * 2) + 0] = (i * 2) + 0;
		lineIndices[(i * 2) + 1] = (i * 2) + 1;
	}

	if (lineVerts.size() == 0)
	{
		return;
	}

	if (view->LineVertexBuffer == nullptr || lineVerts.size() > view->LineVertexBuffer->GetCapacity())
	{
		VertexBufferBindingDescription description;
		m_debugLineMaterial.Get()->GetVertexBufferFormat(description);

		view->LineVertexBuffer = m_graphics->CreateVertexBuffer("Debug Line Vertex Buffer", description, lineVerts.size());
	}

	if (view->LineIndexBuffer == nullptr || lineIndices.size() > view->LineIndexBuffer->GetCapacity())
	{
		view->LineIndexBuffer = m_graphics->CreateIndexBuffer("Debug Line Index Buffer", sizeof(uint16_t), lineIndices.size());
	}

	view->LineVertexBuffer->Stage(lineVerts.data(), 0, lineVerts.size() * sizeof(DebugLineVertex));
	view->LineIndexBuffer->Stage(lineIndices.data(), 0, lineIndices.size() * sizeof(uint16_t));

	std::shared_ptr<Material> material = m_debugLineMaterial.Get();
	material->UpdateResources();

	buffer->TransitionResourceSets(&material->GetResourceSet(), 1);

	buffer->Upload(view->LineVertexBuffer);
	buffer->Upload(view->LineIndexBuffer);
	
	buffer->BeginPass(material->GetRenderPass(), material->GetFrameBuffer());
	buffer->BeginSubPass();

	buffer->SetPipeline(material->GetPipeline());
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

	buffer->SetIndexBuffer(view->LineIndexBuffer);
	buffer->SetVertexBuffer(view->LineVertexBuffer);
	buffer->SetResourceSetInstances(&material->GetResourceSet()->ConsumeInstance(), 1);

	buffer->DrawIndexedElements(view->Lines.size() * 2, 1, 0, 0, 0);

	buffer->EndSubPass();
	buffer->EndPass();

}

void Renderer::BuildViewCommandBuffer(std::shared_ptr<RenderView> view, std::shared_ptr<IGraphicsCommandBuffer> buffer)
{
	ProfileScope scope(Color::Red, "Renderer::BuildViewCommandBuffer");

	m_globalMaterialProperties.Set(ModelMatrixHash, Matrix4::Identity);
	m_globalMaterialProperties.Set(ViewMatrixHash, view->ViewMatrix);
	m_globalMaterialProperties.Set(ProjectionMatrixHash, view->ProjectionMatrix);
	m_globalMaterialProperties.Set(CameraPositionHash, view->CameraPosition);

	// TODO: Z Pre Pass

	// Draw the scene.
	BuildViewCommandBuffer_Meshes(view, buffer);
	BuildViewCommandBuffer_Lines(view, buffer);

	// pipeline sync here.

	// TODO: Light accumulation.

	// resolve gbuffer layouts to read access?
}

void Renderer::Present()
{
	ProfileScope scope(Color::Red, "Renderer::Present");

	m_statMeshesRendered = 0;
	m_statBatchesRendered = 0;
	m_statTrianglesRendered = 0;

	//printf("Present...\n");
	std::shared_ptr<IGraphicsCommandBuffer> commandBuffer = m_commandBuffers[m_frameCounter % m_commandBuffers.size()];
	BuildCommandBuffer(commandBuffer);
	m_graphics->Dispatch(commandBuffer);

	m_frameCounter++;
	m_frameIndex = m_frameCounter % m_swapChainViews.size();

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

bool Renderer::IsRenderingFrozen()
{
	return m_renderingIsFrozen;
}

bool Renderer::IsDrawBoundsEnabled()
{
	return m_drawBoundsEnabled;
}

std::shared_ptr<RenderView> Renderer::QueueView()
{
	std::lock_guard<std::mutex> lock(m_renderViewsMutex);

	if (m_freeRenderViews.size() > 0)
	{
		std::shared_ptr<RenderView> back = m_freeRenderViews.back();
		m_freeRenderViews.pop_back();
		m_renderViews.push_back(back);
		return back;
	}

	std::shared_ptr<RenderView> view = std::make_shared<RenderView>();
	m_renderViews.push_back(view);

	return view;
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