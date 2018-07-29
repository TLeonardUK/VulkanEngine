#include "Pch.h"

#include "Engine/Components/Mesh/MeshComponent.h"

#include "Engine/Rendering/Renderer.h"
#include "Engine/Rendering/RenderView.h"
#include "Engine/Rendering/RendererEnums.h"
#include "Engine/Profiling/Profiling.h"

#include "Engine/Engine/Logging.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/GraphicsResourceSet.h"
#include "Engine/Graphics/GraphicsResourceSetInstance.h"

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
	DrawFullScreenQuad(buffer, m_clearGBufferMaterial.Get(), &m_clearGBufferMaterialRenderData);

	// Render all views to gbuffer.
	for (auto& view : m_renderViews)
	{
		BuildViewCommandBuffer(view, buffer);
	}

	// Resolve to framebuffer.
	DrawFullScreenQuad(buffer, m_resolveToSwapchainMaterial.Get(), &m_resolveToSwapchainMaterialRenderData);

	RunQueuedCommands(RenderCommandStage::PostViewsRendered, buffer);

	buffer->End();

	// Clear frame-specific render views.
	for (auto& iter : m_renderViews)
	{
		m_freeRenderViews.push_back(iter);
	}
	m_renderViews.clear();
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

	buffer->TransitionResourceSets(&(*materialRenderData)->GetResourceSet(), 1);

	buffer->BeginPass(material->GetRenderPass(), material->GetFrameBuffer());
	buffer->BeginSubPass();

	buffer->SetPipeline(material->GetPipeline());
	buffer->SetViewport(0, 0, m_swapChainWidth, m_swapChainHeight);
	buffer->SetScissor(0, 0, m_swapChainWidth, m_swapChainHeight);

	buffer->SetIndexBuffer(m_fullscreenQuadIndexBuffer);
	buffer->SetVertexBuffer(m_fullscreenQuadVertexBuffer);

	buffer->SetResourceSetInstances(&(*materialRenderData)->GetResourceSet()->NewInstance(), 1);

	buffer->DrawIndexedElements(6, 1, 0, 0, 0);

	buffer->EndSubPass();
	buffer->EndPass();
}

void Renderer::BuildViewCommandBuffer_Meshes(std::shared_ptr<RenderView> view, std::shared_ptr<IGraphicsCommandBuffer> buffer)
{
	ProfileScope scope(Color::Red, "Renderer::BuildViewCommandBuffer_Meshes");

	// Batch meshes by material.
	int concurrency = TaskManager::AsyncInstance->GetConcurrency();
	int chunkSize = (int)Math::Ceil(view->Meshes.size() / (float)concurrency);

	// Resize the batch lists so they have enough space for each material.
	{
		ProfileScope scope(Color::Red, "Resize Batch Lists");

		m_asyncMaterialBatches.resize(concurrency);
		m_materialRenderBatches.clear();
	}

	// Batch all meshes in parallel.
	ParallelFor(concurrency, [&](int index)
	{
		ProfileScope scope(Color::Red, "Batch meshes");

		Dictionary<Material*, MaterialBatch>& batches = m_asyncMaterialBatches[index];

		for (auto& iter : batches)
		{
			iter.second.meshInstanceCount = 0;
			if (iter.second.meshInstances.size() < view->Meshes.size())
			{
				iter.second.meshInstances.resize(view->Meshes.size());
			}
		}

		// todo: batch in parallel, each thread makes its own list, which is then combined.
		int startIndex = (index * chunkSize);
		int endIndex = Math::Min((int)view->Meshes.size(), startIndex + chunkSize);

		for (int i = startIndex; i < endIndex; i++)
		{
			ProfileScope scope(Color::Red, "Mesh");
			auto& visibleMesh = view->Meshes[i];

			Material* material = &*visibleMesh.Mesh->GetMaterial().Get();
			MaterialBatch* batch = nullptr;

			auto batchIter = batches.find(material);
			if (batchIter != batches.end())
			{
				batch = &batchIter->second;
			}
			else
			{
				batches.emplace(material, MaterialBatch());

				batch = &batches[material];
				batch->material = material;
				batch->meshInstanceCount = 0;
				batch->meshInstances.resize(view->Meshes.size());
			}

			// Update mesh state.
			visibleMesh.Mesh->UpdateResources();

			UpdateMaterialRenderData(
				&visibleMesh.MeshComponent->renderData[visibleMesh.MeshIndex],
				visibleMesh.Mesh->GetMaterial().Get(),
				&visibleMesh.MeshComponent->properties);

			std::shared_ptr<MaterialRenderData>& renderData = visibleMesh.MeshComponent->renderData[visibleMesh.MeshIndex];

			// Create new instance.
			MeshInstance& instance = batch->meshInstances[batch->meshInstanceCount++];
			instance.resourceSet = renderData->GetResourceSet();
			// todo: don't request new descriptor set if binding has not changed.
			{
				ProfileScope scope(Color::Red, "Resource Set Request");
				instance.resourceSetInstance = renderData->GetResourceSetInstance();
			}
			instance.indexBuffer = visibleMesh.Mesh->GetIndexBuffer();
			instance.vertexBuffer = visibleMesh.Mesh->GetVertexBuffer();
			instance.indexCount = visibleMesh.Mesh->GetIndexCount();

			Stat_Rendering_Budgets_MeshesRendered.Add(1);
			Stat_Rendering_Budgets_TrianglesRendered.Add(instance.indexCount / 3);
		}
	}, 1, "Mesh Batching");

	// Make list of all batch types available.
	int totalMeshInstances = 0;
	{
		ProfileScope scope(Color::Red, "Combine Batches");

		for (int i = 0; i < concurrency; i++)
		{
			for (auto& iter : m_asyncMaterialBatches[i])
			{
				if (iter.second.meshInstanceCount > 0)
				{
					int instanceOffset = 0;
					while (instanceOffset < iter.second.meshInstanceCount)
					{
						MaterialRenderBatch* batch = nullptr;

						for (int j = 0; j < m_materialRenderBatches.size(); j++)
						{
							if (m_materialRenderBatches[j].material == iter.first &&
								m_materialRenderBatches[j].meshInstances.size() < MaxMeshesPerBatch)
							{
								batch = &m_materialRenderBatches[j];
								break;
							}
						}

						if (batch == nullptr)
						{
							m_materialRenderBatches.push_back(MaterialRenderBatch());
							batch = &m_materialRenderBatches[m_materialRenderBatches.size() - 1];

							batch->material = iter.first;
						}

						int spaceAvailable = MaxMeshesPerBatch - batch->meshInstances.size();
						int itemsLeft = iter.second.meshInstanceCount - instanceOffset;
						int itemsToAdd = Math::Min(spaceAvailable, itemsLeft);

						batch->meshInstances.reserve(batch->meshInstances.size() + itemsToAdd);
						for (int k = 0; k < itemsToAdd; k++)
						{
							batch->meshInstances.push_back(&iter.second.meshInstances[instanceOffset + k]);
						}
						instanceOffset += itemsToAdd;
						totalMeshInstances += itemsToAdd;
					}
				}
			}
		}
	}

	// Draw each batch.
	m_batchBuffers.resize(m_materialRenderBatches.size());
	m_batchTransitionBuffers.resize(m_materialRenderBatches.size());

	// Generate command buffers for each batch.
	ParallelFor(m_materialRenderBatches.size(), [&](int index)
	{
		std::shared_ptr<IGraphicsCommandBuffer> drawBuffer;
		std::shared_ptr<IGraphicsCommandBuffer> transitionBuffer;
		{
			ProfileScope scope2(Color::White, "Allocate Buffer");
			drawBuffer = RequestSecondaryBuffer();
			transitionBuffer = RequestSecondaryBuffer();
		}
		m_batchBuffers[index] = drawBuffer;
		m_batchTransitionBuffers[index] = transitionBuffer;

		MaterialRenderBatch& batch = m_materialRenderBatches[index];

		ProfileScope scope(Color::Red, "Render material batch: " + batch.material->m_name);

		// Generate transition buffer.
		for (int i = 0; i < batch.meshInstances.size(); i++)
		{
			transitionBuffer->TransitionResourceSets(&batch.meshInstances[i]->resourceSet, 1);
		}

		// Generate drawing buffer.
		drawBuffer->Begin(batch.material->GetRenderPass(), batch.material->GetFrameBuffer());

		if (m_drawWireframeEnabled)
		{
			drawBuffer->SetPipeline(batch.material->GetWireframePipeline());
		}
		else
		{
			drawBuffer->SetPipeline(batch.material->GetPipeline());
		}

		drawBuffer->SetScissor(
			static_cast<int>(view->Viewport.x),
			static_cast<int>(view->Viewport.y),
			static_cast<int>(view->Viewport.width),
			static_cast<int>(view->Viewport.height)
		);
		drawBuffer->SetViewport(
			static_cast<int>(view->Viewport.x),
			static_cast<int>(view->Viewport.y),
			static_cast<int>(view->Viewport.width),
			static_cast<int>(view->Viewport.height)
		);

		{
			ProfileScope scope(Color::Black, "Draw Meshes");
			for (int i = 0; i < batch.meshInstances.size(); i++)
			{
				MeshInstance& instance = *batch.meshInstances[i];
				drawBuffer->SetIndexBuffer(instance.indexBuffer);
				drawBuffer->SetVertexBuffer(instance.vertexBuffer);
				drawBuffer->SetResourceSetInstances(&instance.resourceSetInstance, 1);
				drawBuffer->DrawIndexedElements(instance.indexCount, 1, 0, 0, 0);
			}
		}

		drawBuffer->End();

	}, 1, "Command Buffer Creation");

	// Dispatch all buffers.
	{
		ProfileScope scope(Color::Red, "Dispatch Buffers");
		for (int i = 0; i < m_batchBuffers.size(); i++)
		{
			MaterialRenderBatch& batch = m_materialRenderBatches[i];

			buffer->Dispatch(m_batchTransitionBuffers[i]);

			buffer->BeginPass(batch.material->GetRenderPass(), batch.material->GetFrameBuffer(), false);
			buffer->BeginSubPass();

			buffer->Dispatch(m_batchBuffers[i]);

			buffer->EndSubPass();
			buffer->EndPass();
		}
	}

	Stat_Rendering_Budgets_BatchesRendered.Add(m_batchBuffers.size());
}

void Renderer::BuildViewCommandBuffer_Lines(std::shared_ptr<RenderView> view, std::shared_ptr<IGraphicsCommandBuffer> buffer)
{
	ProfileScope scope(Color::Red, "Renderer::BuildViewCommandBuffer_Lines");

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

		lineIndices[(i * 2) + 0] = static_cast<uint16_t>((i * 2) + 0);
		lineIndices[(i * 2) + 1] = static_cast<uint16_t>((i * 2) + 1);
	}

	if (lineVerts.size() == 0)
	{
		return;
	}

	if (view->LineVertexBuffer == nullptr || lineVerts.size() > view->LineVertexBuffer->GetCapacity())
	{
		VertexBufferBindingDescription description;
		m_debugLineMaterial.Get()->GetVertexBufferFormat(description);

		view->LineVertexBuffer = m_graphics->CreateVertexBuffer("Debug Line Vertex Buffer", description, static_cast<int>(lineVerts.size()));
	}

	if (view->LineIndexBuffer == nullptr || lineIndices.size() > view->LineIndexBuffer->GetCapacity())
	{
		view->LineIndexBuffer = m_graphics->CreateIndexBuffer("Debug Line Index Buffer", sizeof(uint16_t), static_cast<int>(lineIndices.size()));
	}

	view->LineVertexBuffer->Stage(lineVerts.data(), 0, static_cast<int>(lineVerts.size()) * sizeof(DebugLineVertex));
	view->LineIndexBuffer->Stage(lineIndices.data(), 0, static_cast<int>(lineIndices.size()) * sizeof(uint16_t));

	std::shared_ptr<Material> material = m_debugLineMaterial.Get();
	material->UpdateResources();
	UpdateMaterialRenderData(&m_debugLineMaterialRenderData, material, &m_globalMaterialProperties);

	buffer->TransitionResourceSets(&m_debugLineMaterialRenderData->GetResourceSet(), 1);

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
	buffer->SetResourceSetInstances(&m_debugLineMaterialRenderData->GetResourceSet()->NewInstance(), 1);

	buffer->DrawIndexedElements(static_cast<int>(view->Lines.size() * 2), 1, 0, 0, 0);

	buffer->EndSubPass();
	buffer->EndPass();
}

void Renderer::BuildViewCommandBuffer(std::shared_ptr<RenderView> view, std::shared_ptr<IGraphicsCommandBuffer> buffer)
{
	ProfileScope scope(Color::Red, "Renderer::BuildViewCommandBuffer");

	m_globalMaterialProperties.Set(ViewMatrixHash, view->ViewMatrix);
	m_globalMaterialProperties.Set(ProjectionMatrixHash, view->ProjectionMatrix);
	m_globalMaterialProperties.Set(CameraPositionHash, view->CameraPosition);

	// Update global buffers.
	UpdateGlobalUniformBuffers();

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

	// Reset command buffers for this frame.
	UpdateCommandBufferPools();

	// Allocate and build a primary command buffer.
	std::shared_ptr<IGraphicsCommandBuffer> commandBuffer = RequestPrimaryBuffer();
	BuildCommandBuffer(commandBuffer);
	m_graphics->Dispatch(commandBuffer);

	m_frameCounter++;
	m_frameIndex = m_frameCounter % m_swapChainViews.size();

	if (m_graphics->Present())
	{
		SwapChainModified();
	}
}

void Renderer::RegisterGlobalUniformBuffer(const UniformBufferLayout& layout)
{
	// Already exists?
	if (GetGlobalUniformBuffer(layout.HashCode) != nullptr)
	{
		return;
	}

	int dataSize = layout.GetSize();

	GlobalUniformBuffer buffer;
	buffer.layout = layout;
	buffer.buffer = m_graphics->CreateUniformBuffer(StringFormat("Global UBO (%s)", layout.Name.c_str()), dataSize);

	m_globalUniformBuffers.emplace(layout.HashCode, buffer);
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
		std::lock_guard<std::mutex> lock(pool->mutex);

		int frameIndex = m_frameCounter % pool->frameData.size();
		pool->frameData[frameIndex].primaryBuffersAllocated = 0;
		pool->frameData[frameIndex].secondaryBuffersAllocated = 0;
	}
}