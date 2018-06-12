#include "Engine/Rendering/Renderer.h"
#include "Engine/Rendering/RenderView.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/GraphicsResourceSet.h"

// batch everything using same material

static const MaterialPropertyHash ModelMatrixHash = CalculateMaterialPropertyHash("ModelMatrix");
static const MaterialPropertyHash ViewMatrixHash = CalculateMaterialPropertyHash("ViewMatrix");
static const MaterialPropertyHash ProjectionMatrixHash = CalculateMaterialPropertyHash("ProjectionMatrix");

Renderer::Renderer(std::shared_ptr<IGraphics> graphics)
	: m_graphics(graphics)
	, m_frameIndex(0)
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

bool Renderer::Init()
{
	CreateResources();

	return true;
}

void Renderer::Dispose()
{
	FreeResources();
}

void Renderer::CreateResources()
{
	m_commandBufferPool = m_graphics->CreateCommandBufferPool("Main Command Buffer Pool");
	m_resourceSetPool = m_graphics->CreateResourceSetPool("Main Resource Set Pool");

	CreateSwapChainDependentResources();
}

void Renderer::FreeResources()
{
	m_resourceSetPool = nullptr;
	m_commandBufferPool = nullptr;

	FreeSwapChainDependentResources();
}

void Renderer::FreeSwapChainDependentResources()
{
	m_depthBufferImage = nullptr;
	m_depthBufferView = nullptr;
	m_renderPass = nullptr;
	m_swapChainFramebuffers.clear();
	m_commandBuffers.clear();
}

void Renderer::CreateSwapChainDependentResources()
{
	// Create render pass.
	GraphicsRenderPassSettings renderPassSettings;
	renderPassSettings.AddColorAttachment(m_graphics->GetSwapChainFormat(), true);
	renderPassSettings.AddDepthAttachment(GraphicsFormat::UNORM_D24_UINT_S8);

	GraphicsSubPassIndex subPass1 = renderPassSettings.AddSubPass();
	renderPassSettings.AddSubPassDependency(GraphicsExternalPassIndex, GraphicsAccessMask::None, subPass1, GraphicsAccessMask::ReadWrite);

	m_renderPass = m_graphics->CreateRenderPass("Main Render Pass", renderPassSettings);

	m_swapChainViews = m_graphics->GetSwapChainViews();
	m_swapChainWidth = m_swapChainViews[0]->GetWidth();
	m_swapChainHeight = m_swapChainViews[0]->GetHeight();

	// Create depth buffer.
	m_depthBufferImage = m_graphics->CreateImage("Depth Buffer", m_swapChainWidth, m_swapChainHeight, GraphicsFormat::UNORM_D24_UINT_S8, false);
	m_depthBufferView = m_graphics->CreateImageView("Depth Buffer View", m_depthBufferImage);

	// Create frame buffers for each swap chain image.
	m_swapChainFramebuffers.resize(m_swapChainViews.size());
	for (int i = 0; i < m_swapChainViews.size(); i++)
	{
		std::shared_ptr<IGraphicsImageView> imageView = m_swapChainViews[i];

		GraphicsFramebufferSettings frameBufferSettings;
		frameBufferSettings.width = imageView->GetWidth();
		frameBufferSettings.height = imageView->GetHeight();
		frameBufferSettings.renderPass = m_renderPass;
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

	std::shared_ptr<IGraphicsFramebuffer> framebuffer = m_swapChainFramebuffers[m_frameIndex];

	// Draw each view.
	for (auto& view : m_renderViews)
	{
		BuildViewCommandBuffer(view, framebuffer, buffer);
	}

	buffer->End();
}

void Renderer::BuildViewCommandBuffer(std::shared_ptr<RenderView> view, std::shared_ptr<IGraphicsFramebuffer> framebuffer, std::shared_ptr<IGraphicsCommandBuffer> buffer)
{
	Matrix4 identityMatrix = Matrix4(1.0);

	m_globalMaterialProperties.Set(ModelMatrixHash, identityMatrix);
	m_globalMaterialProperties.Set(ViewMatrixHash, view->ViewMatrix);
	m_globalMaterialProperties.Set(ProjectionMatrixHash, view->ProjectionMatrix);

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

				buffer->BeginPass(material->GetRenderPass(), framebuffer);
				buffer->BeginSubPass();

				buffer->SetPipeline(material->GetPipeline());
				buffer->SetScissor(view->Viewport.x, view->Viewport.y, view->Viewport.width, view->Viewport.height);
				buffer->SetViewport(view->Viewport.x, view->Viewport.y, view->Viewport.width, view->Viewport.height);

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
