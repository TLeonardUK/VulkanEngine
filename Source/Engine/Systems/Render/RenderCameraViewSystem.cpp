#include "Pch.h"

#include "Engine/Components/Mesh/MeshComponent.h"

#include "Engine/Systems/Render/RenderCameraViewSystem.h"
#include "Engine/Systems/Render/RenderDebugSystem.h"
#include "Engine/Systems/Mesh/MeshBoundsUpdateSystem.h"
#include "Engine/Systems/Transform/TransformSystem.h"
#include "Engine/Systems/Transform/TransformUtils.h"
#include "Engine/Systems/Transform/SpatialIndexSystem.h"

#include "Engine/Rendering/Renderer.h"
#include "Engine/Rendering/RenderView.h"

#include "Engine/Graphics/GraphicsCommandBuffer.h"

#include "Engine/Profiling/Profiling.h"
#include "Engine/Threading/ParallelFor.h"

#include "Engine/Types/OctTree.h"

RenderCameraViewSystem::RenderCameraViewSystem(std::shared_ptr<World> world, std::shared_ptr<Renderer> renderer)
	: m_renderer(renderer)
{
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
		view->viewport.x * swapWidth,
		view->viewport.y * swapHeight,
		view->viewport.width * swapWidth,
		view->viewport.height * swapHeight
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
		DrawDebugFrustumMessage frusumDrawMsg;
		frusumDrawMsg.frustum = view->frustum;
		frusumDrawMsg.color = Color::Green;
		world.QueueMessage(frusumDrawMsg);
	}

	// Update global properties.
	m_renderer->GetGlobalMaterialProperties().Set(ViewMatrixHash, view->viewMatrix);
	m_renderer->GetGlobalMaterialProperties().Set(ProjectionMatrixHash, view->projectionMatrix);
	m_renderer->GetGlobalMaterialProperties().Set(CameraPositionHash, cameraPosition);

	// Update global buffers.
	m_renderer->UpdateGlobalResources();

	// Grab all visible entities from the oct-tree.
	{
		ProfileScope scope(Color::Blue, "Search OctTree");
		spatialSystem->GetTree().Get(view->frustum, m_visibleEntitiesResult, false);
	}

	// Batch up all meshes.
	{
		ProfileScope scope(Color::Blue, "Batch Meshes");
		m_meshBatcher.Batch(world, m_renderer, m_visibleEntitiesResult.entries, MaterialVariant::Normal);
	}

	// Generate command buffers for each batch.
	Array<MaterialRenderBatch>& renderBatches = m_meshBatcher.GetBatches();

	m_batchBuffers.resize(renderBatches.size());

	ParallelFor(renderBatches.size(), [&](int index)
	{
		std::shared_ptr<IGraphicsCommandBuffer> drawBuffer = m_renderer->RequestSecondaryBuffer();
		m_batchBuffers[index] = drawBuffer;

		MaterialRenderBatch& batch = renderBatches[index];

		ProfileScope scope(Color::Red, "Render material batch: " + batch.material->GetName());

		// Generate drawing buffer.
		drawBuffer->Begin(batch.material->GetRenderPass(), batch.material->GetFrameBuffer());

		if (m_renderer->IsWireframeEnabled())
		{
			drawBuffer->SetPipeline(batch.material->GetWireframePipeline());
		}
		else
		{
			drawBuffer->SetPipeline(batch.material->GetPipeline());
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
			ProfileScope scope(Color::Black, "Draw Meshes");
			for (int i = 0; i < batch.meshInstances.size(); i++)
			{
				MeshInstance& instance = *batch.meshInstances[i];
				drawBuffer->SetIndexBuffer(*instance.indexBuffer);
				drawBuffer->SetVertexBuffer(*instance.vertexBuffer);
				drawBuffer->SetResourceSets(instance.resourceSets->data(), instance.resourceSets->size());
				drawBuffer->DrawIndexedElements(instance.indexCount, 1, 0, 0, 0);
			}
		}

		drawBuffer->End();

	}, 1, "Command Buffer Creation");

	// Dispatch each buffer.
	if (m_batchBuffers.size() > 0)
	{
		std::shared_ptr<IGraphicsCommandBuffer> buffer = m_renderer->RequestPrimaryBuffer();
		buffer->Reset();
		buffer->Begin();

		{
			ProfileScope scope(Color::Red, "Dispatch Buffers");
			for (int i = 0; i < m_batchBuffers.size(); i++)
			{
				MaterialRenderBatch& batch = renderBatches[i];

				ProfileScope scope(Color::Black, StringFormat("Batch: %s", batch.material->GetName().c_str()));

				buffer->BeginPass(batch.material->GetRenderPass(), batch.material->GetFrameBuffer(), false);
				buffer->BeginSubPass();

				buffer->Dispatch(m_batchBuffers[i]);

				buffer->EndSubPass();
				buffer->EndPass();
			}
		}

		buffer->End();
		m_renderer->QueuePrimaryBuffer(RenderCommandStage::Render, buffer);
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
		ProfileScope scope(Color::Blue, "Tick Messages");

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
		ProfileScope scope(Color::Blue, "Tick Views");

		for (size_t i = 0; i < entities.size(); i++)
		{
			CameraViewComponent* view = views[i];
			const TransformComponent* transform = transforms[i];

			TickView(world, frameTime, view, transform);
		}
	}
}
