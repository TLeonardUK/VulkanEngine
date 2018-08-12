#include "Pch.h"

#include "Engine/Components/Mesh/MeshComponent.h"

#include "Engine/Systems/Render/RenderDirectionalShadowMapSystem.h"
#include "Engine/Systems/Render/RenderDebugSystem.h"
#include "Engine/Systems/Render/RenderCameraViewSystem.h"
#include "Engine/Systems/Mesh/MeshBoundsUpdateSystem.h"
#include "Engine/Systems/Transform/TransformSystem.h"
#include "Engine/Systems/Transform/TransformUtils.h"
#include "Engine/Systems/Transform/SpatialIndexSystem.h"

#include "Engine/Rendering/Renderer.h"
#include "Engine/Rendering/RenderView.h"

#include "Engine/Graphics/Graphics.h"
#include "Engine/Graphics/GraphicsCommandBuffer.h"

#include "Engine/Profiling/Profiling.h"
#include "Engine/Threading/ParallelFor.h"

#include "Engine/Types/OctTree.h"

RenderDirectionalShadowMapSystem::RenderDirectionalShadowMapSystem(
	std::shared_ptr<World> world,
	std::shared_ptr<Renderer> renderer,
	std::shared_ptr<ResourceManager> resourceManager,
	std::shared_ptr<IGraphics> graphics
)
	: m_renderer(renderer)
	, m_graphics(graphics)
{
	AddPredecessor<MeshBoundsUpdateSystem>();
	AddPredecessor<TransformSystem>();

	// todo: remove when we make this safe to run in parallel (remove global properties / resource updates / etc)
	AddPredecessor<RenderCameraViewSystem>();

	m_meshComponentAspectId = world->GetAspectId({ typeid(TransformComponent), typeid(MeshComponent) });
}

void RenderDirectionalShadowMapSystem::RenderLight(
	World& world,
	const FrameTime& frameTime,
	DirectionalLightComponent* light,
	const TransformComponent* transform)
{
	SpatialIndexSystem* spatialSystem = world.GetSystem<SpatialIndexSystem>();

	// Create resources if required.
	if (light->shadowMapImage == nullptr)
	{
		light->shadowMapImage = m_graphics->CreateImage("Shadow Map", light->shadowMapSize, light->shadowMapSize, 1, GraphicsFormat::UNORM_D16, false, GraphicsUsage::DepthAttachment);
		light->shadowMapImageView = m_graphics->CreateImageView("Shadow Map View", light->shadowMapImage);

		// Create framebuffer.
		GraphicsFramebufferSettings settings;
		settings.width = light->shadowMapSize;
		settings.height = light->shadowMapSize;
		settings.renderPass = m_renderer->GetRenderPassForTarget(FrameBufferTarget::DepthBuffer);
		settings.attachments.push_back(light->shadowMapImageView);

		light->shadowMapFramebuffer = m_graphics->CreateFramebuffer("Shadow Map Framebuffer", settings);
	}
	
	Vector3 worldLocation = TransformUtils::GetWorldLocation(transform);
	Vector3 forward = TransformUtils::GetForwardVector(transform);

	Matrix4 viewMatrix = Matrix4::LookAt(worldLocation, worldLocation + TransformUtils::GetForwardVector(transform), Vector3::Up);
	Matrix4 projectionMatrix = Matrix4::Perspective(Math::Radians(90.0f), 1.0f, 1.0f, 10000.0f);
	Vector3 cameraPosition = worldLocation;

	if (!m_renderer->IsRenderingFrozen())
	{
		light->frustum = Frustum(projectionMatrix * viewMatrix);
	}

	if (m_renderer->IsDrawBoundsEnabled())
	{
		DrawDebugFrustumMessage frusumDrawMsg;
		frusumDrawMsg.frustum = light->frustum;
		frusumDrawMsg.color = Color::Green;
		world.QueueMessage(frusumDrawMsg);
	}

	// Update global properties.
	m_renderer->GetGlobalMaterialProperties().Set(ViewMatrixHash, viewMatrix);
	m_renderer->GetGlobalMaterialProperties().Set(ProjectionMatrixHash, projectionMatrix);
	m_renderer->GetGlobalMaterialProperties().Set(CameraPositionHash, cameraPosition);

	// Update global buffers.
	m_renderer->UpdateGlobalResources();

	// Grab all visible entities from the oct-tree.
	{
		ProfileScope scope(Color::Blue, "Search OctTree");
		spatialSystem->GetTree().Get(light->frustum, m_visibleEntitiesResult, false);
	}

	// Batch up all meshes.
	{
		ProfileScope scope(Color::Blue, "Batch Meshes");
		m_meshBatcher.Batch(world, m_renderer, m_visibleEntitiesResult.entries, MaterialVariant::DepthOnly);
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
		drawBuffer->Begin(batch.material->GetRenderPass(), light->shadowMapFramebuffer);
		drawBuffer->SetPipeline(batch.material->GetPipeline());

		drawBuffer->SetScissor(0, 0, light->shadowMapSize, light->shadowMapSize);
		drawBuffer->SetViewport(0, 0, light->shadowMapSize, light->shadowMapSize);

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

		buffer->TransitionResource(light->shadowMapImage, GraphicsAccessMask::ReadWrite);
		
		buffer->Clear(light->shadowMapImage, Color::Black, 1.0f, 0.0f);

		{
			ProfileScope scope(Color::Red, "Dispatch Buffers");
			for (int i = 0; i < m_batchBuffers.size(); i++)
			{
				MaterialRenderBatch& batch = renderBatches[i];

				ProfileScope scope(Color::Black, StringFormat("Batch: %s", batch.material->GetName().c_str()));

				buffer->BeginPass(batch.material->GetRenderPass(), light->shadowMapFramebuffer, false);
				buffer->BeginSubPass();

				buffer->Dispatch(m_batchBuffers[i]);

				buffer->EndSubPass();
				buffer->EndPass();
			}
		}

		buffer->End();
		m_renderer->QueuePrimaryBuffer("Shadow Map Generation", RenderCommandStage::Shadow, buffer);
	}
}

void RenderDirectionalShadowMapSystem::Tick(
	World& world,
	const FrameTime& frameTime,
	const Array<Entity>& entities,
	const Array<DirectionalLightComponent*>& lights,
	const Array<const TransformComponent*>& transforms)
{
	// Draw shadow map for each light.
	{
		ProfileScope scope(Color::Blue, "Update Shadow Maps");

		// todo: modify to support parallel.

		for (size_t i = 0; i < entities.size(); i++)
		{
			DirectionalLightComponent* light = lights[i];
			const TransformComponent* transform = transforms[i];

			if (light->isShadowCasting)
			{
				RenderLight(world, frameTime, light, transform);
			}
		}
	}
}
