#include "Pch.h"

#include "Engine/Components/Mesh/MeshComponent.h"

#include "Engine/Systems/Render/RenderCameraViewSystem.h"
#include "Engine/Systems/Render/RenderDebugSystem.h"
#include "Engine/Systems/Mesh/MeshBoundsUpdateSystem.h"
#include "Engine/Systems/Mesh/MeshRenderStateUpdateSystem.h"
#include "Engine/Systems/Transform/TransformSystem.h"
#include "Engine/Systems/Transform/TransformUtils.h"
#include "Engine/Systems/Transform/SpatialIndexSystem.h"

#include "Engine/Rendering/Renderer.h"
#include "Engine/Rendering/RenderView.h"

#include "Engine/Graphics/GraphicsCommandBuffer.h"

#include "Engine/Profiling/Profiling.h"
#include "Engine/Threading/ParallelFor.h"

#include "Engine/Types/OctTree.h"

RenderCameraViewSystem::RenderCameraViewSystem(
	std::shared_ptr<World> world,
	std::shared_ptr<IGraphics> graphics,
	std::shared_ptr<Logger> logger,
	std::shared_ptr<Renderer> renderer)
	: m_renderer(renderer)
	, m_logger(logger)
	, m_graphics(graphics)
{
	AddPredecessor<MeshRenderStateUpdateSystem>();
	AddPredecessor<MeshBoundsUpdateSystem>();
	AddPredecessor<TransformSystem>();

	m_meshComponentAspectId = world->GetAspectId({ typeid(TransformComponent), typeid(MeshComponent) });
}

void RenderCameraViewSystem::TickView(
	World& world,
	const FrameTime& frameTime,
	CameraViewComponent* view,
	const TransformComponent* transform)
{
	SpatialIndexSystem* spatialSystem = world.GetSystem<SpatialIndexSystem>();

	Vector3 worldLocation = TransformUtils::GetWorldLocation(transform);
	Vector3 forward = TransformUtils::GetForwardVector(transform);

	int swapWidth = m_renderer->GetSwapChainWidth();
	int swapHeight = m_renderer->GetSwapChainHeight();

	Rect viewport(
		0.0f,
		0.0f,
		swapWidth,
		swapHeight
	);

	view->viewMatrix = Matrix4::LookAt(worldLocation, worldLocation + TransformUtils::GetForwardVector(transform), Vector3::Up);
	view->projectionMatrix = Matrix4::Perspective(Math::Radians(view->fov), viewport.width / viewport.height, view->depthMin, view->depthMax);
	Vector3 cameraPosition = worldLocation;

	if (!m_renderer->IsRenderingFrozen())
	{
		view->frustum = Frustum(view->projectionMatrix * view->viewMatrix);
	}

	if (m_renderer->IsDrawBoundsEnabled())
	{
	/*	DrawDebugFrustumMessage frusumDrawMsg;
		frusumDrawMsg.frustum = view->frustum;
		frusumDrawMsg.color = Color::Green;
		world.QueueMessage(frusumDrawMsg);

		DrawDebugBoundsMessage boundsMsg;
		boundsMsg.color = Color::PureRed;
		boundsMsg.bounds = Bounds::FromCenterAndExtents(view->frustum.GetOrigin(), Vector3(5.0f, 5.0f, 5.0f));
		world.QueueMessage(boundsMsg);*/
	}

	// Update global properties.
	view->viewProperties.Set(ViewMatrixHash, view->viewMatrix);
	view->viewProperties.Set(ProjectionMatrixHash, view->projectionMatrix);
	view->viewProperties.Set(CameraPositionHash, cameraPosition);
	view->viewProperties.UpdateResources(m_graphics, m_logger);

	// Grab all visible entities from the oct-tree.
	{
		ProfileScope scope(ProfileColors::Draw, "Search OctTree");
		spatialSystem->GetTree().Get(view->frustum, m_visibleEntitiesResult, false);
	}

	// Batch up all meshes.
	{
		ProfileScope scope(ProfileColors::Draw, "Batch Meshes");
		m_meshBatcher.Batch(world, m_renderer, m_logger, m_graphics, m_visibleEntitiesResult.entries, MaterialVariant::Normal, &view->viewProperties);
	}

	// Generate command buffers for each batch.
	Array<MaterialRenderBatch*>& renderBatches = m_meshBatcher.GetBatches();

	m_batchBuffers.resize(renderBatches.size());

	ParallelFor((int)renderBatches.size(), [&](int index)
	{
		std::shared_ptr<IGraphicsCommandBuffer> drawBuffer = m_renderer->RequestPrimaryBuffer();
		m_batchBuffers[index] = drawBuffer;

		MaterialRenderBatch*& batch = renderBatches[index];

		ProfileScope scope(ProfileColors::Draw, "Render material batch: " + batch->material->GetName());

		// Generate drawing buffer.
		drawBuffer->Reset();
		drawBuffer->Begin();

		drawBuffer->BeginPass(batch->material->GetRenderPass(), batch->material->GetFrameBuffer(), true);
		drawBuffer->BeginSubPass();

		if (m_renderer->IsWireframeEnabled())
		{
			drawBuffer->SetPipeline(batch->material->GetWireframePipeline());
		}
		else
		{
			drawBuffer->SetPipeline(batch->material->GetPipeline());
		}

		drawBuffer->SetScissor(
			static_cast<int>(viewport.x),
			static_cast<int>(viewport.y),
			static_cast<int>(viewport.width),
			static_cast<int>(viewport.height)
		);
		drawBuffer->SetViewport(
			static_cast<int>(viewport.x),
			static_cast<int>(viewport.y),
			static_cast<int>(viewport.width),
			static_cast<int>(viewport.height)
		);

		{
			ProfileScope scope(ProfileColors::Draw, "Draw Meshes");
			for (int i = 0; i < batch->meshInstances.size(); i++)
			{
				MeshInstance& instance = *batch->meshInstances[i];
				drawBuffer->SetIndexBuffer(*instance.indexBuffer);
				drawBuffer->SetVertexBuffer(*instance.vertexBuffer);
				drawBuffer->SetResourceSets(instance.resourceSets->data(), (int)instance.resourceSets->size());
				drawBuffer->DrawIndexedElements(instance.indexCount, 1, 0, 0, 0);
			}
		}

		drawBuffer->EndSubPass();
		drawBuffer->EndPass();

		drawBuffer->End();

	}, 1, "Command Buffer Creation");
	
	for (int i = 0; i < m_batchBuffers.size(); i++)
	{
		m_renderer->QueuePrimaryBuffer("Camera View Render", RenderCommandStage::View_GBuffer, m_batchBuffers[i], reinterpret_cast<uint64_t>(view));
	}

	// Transition buffer at start of rendering.
	{
		std::shared_ptr<IGraphicsCommandBuffer> buffer = m_renderer->RequestPrimaryBuffer();

		buffer->Reset();
		buffer->Begin();

		// Transition all images so they can be written to.
		buffer->TransitionResource(m_renderer->GetDepthImage(), GraphicsAccessMask::ReadWrite);
		buffer->TransitionResource(m_renderer->GetShadowMaskImage(), GraphicsAccessMask::Write);
		buffer->TransitionResource(m_renderer->GetLightAccumulationImage(), GraphicsAccessMask::Write);
		for (int i = 0; i < Renderer::GBufferImageCount; i++)
		{
			buffer->TransitionResource(m_renderer->GetGBufferImage(i), GraphicsAccessMask::Write);
		}

		// Clear all buffers we will be rendering to.
		buffer->Clear(m_renderer->GetDepthImage(), Color(0.1f, 0.1f, 0.1f, 1.0f), 1.0f, 0.0f);
		buffer->Clear(m_renderer->GetShadowMaskImage(), Color(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0.0f);
		buffer->Clear(m_renderer->GetLightAccumulationImage(), Color(0.0f, 0.0f, 0.0f, 0.0f), 1.0f, 0.0f);

		// DBEUG DEBUG DEBUG DEBUG
		buffer->Clear(m_renderer->GetLightAccumulationImage(), Color(1.0f, 1.0f, 1.0f, 1.0f), 1.0f, 0.0f);
		// DBEUG DEBUG DEBUG DEBUG

		// Clear G-Buffer
		m_renderer->DrawFullScreenQuad(buffer, m_renderer->GetClearGBufferMaterial().Get(), &m_renderer->GetClearGBufferRenderState(), nullptr, nullptr);

		buffer->End();

		m_renderer->QueuePrimaryBuffer("Camera View Pre Transitions", RenderCommandStage::View_PreRender, buffer, reinterpret_cast<uint64_t>(view));
	}

	// Transition/resolve buffer at end of rendering.
	{
		std::shared_ptr<IGraphicsCommandBuffer> buffer = m_renderer->RequestPrimaryBuffer();

		buffer->Reset();
		buffer->Begin();

		// Transition all images so they can be read from.
		buffer->TransitionResource(m_renderer->GetShadowMaskImage(), GraphicsAccessMask::Read);
		buffer->TransitionResource(m_renderer->GetLightAccumulationImage(), GraphicsAccessMask::Read);
		for (int i = 0; i < Renderer::GBufferImageCount; i++)
		{
			buffer->TransitionResource(m_renderer->GetGBufferImage(i), GraphicsAccessMask::Read);
		}

		// Resolve to framebuffer.
		// todo: support resolving to texture etc.
		m_renderer->DrawFullScreenQuad(
			buffer, 
			m_renderer->GetResolveToSwapChainMaterial().Get(), 
			&m_renderer->GetResolveToSwapChainRenderState(), 
			nullptr, 
			nullptr,
			view->viewport,
			view->viewport);

		buffer->End();

		m_renderer->QueuePrimaryBuffer("Camera View Post Transitions", RenderCommandStage::View_PostRender, buffer, reinterpret_cast<uint64_t>(view));
	}
}

void RenderCameraViewSystem::Tick(
	World& world,
	const FrameTime& frameTime,
	const Array<Entity>& entities,
	const Array<CameraViewComponent*>& views,
	const Array<const TransformComponent*>& transforms)
{
	// Consume all relevant messages.
	{
		ProfileScope scope(ProfileColors::Draw, "Tick Messages");

		for (auto& message : world.ConsumeMessages<SetCameraViewProjectionMessage>())
		{
			CameraViewComponent* component = message.component.Get(world);
			if (component != nullptr)
			{
				component->viewport = message.viewport;
				component->depthMin = message.depthMin;
				component->depthMax = message.depthMax;
				component->fov = message.fov;
			}
		}
	}

	// Draw visible meshes for each view.
	{
		ProfileScope scope(ProfileColors::Draw, "Tick Views");

		for (size_t i = 0; i < entities.size(); i++)
		{
			CameraViewComponent* view = views[i];
			const TransformComponent* transform = transforms[i];

			TickView(world, frameTime, view, transform);
		}
	}
}
