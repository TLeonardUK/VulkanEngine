#include "Pch.h"

#include "Engine/Components/Mesh/MeshComponent.h"

#include "Engine/Systems/Render/RenderDirectionalShadowMapSystem.h"
#include "Engine/Systems/Render/RenderDebugSystem.h"
#include "Engine/Systems/Render/RenderCameraViewSystem.h"
#include "Engine/Systems/Mesh/MeshBoundsUpdateSystem.h"
#include "Engine/Systems/Mesh/MeshRenderStateUpdateSystem.h"
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

float CalculateCascadeDistance(float nearZ, float farZ, float percent, float blend)
{
	float uni = Math::Lerp(nearZ, farZ, percent);  
	float log = nearZ * Math::Pow((farZ / nearZ), percent);
	return Math::Lerp(uni, log, blend);
}

RenderDirectionalShadowMapSystem::RenderDirectionalShadowMapSystem(
	std::shared_ptr<World> world,
	std::shared_ptr<Logger> logger,
	std::shared_ptr<Renderer> renderer,
	std::shared_ptr<ResourceManager> resourceManager,
	std::shared_ptr<IGraphics> graphics
)
	: m_renderer(renderer)
	, m_graphics(graphics)
	, m_logger(logger)
{
	AddPredecessor<MeshRenderStateUpdateSystem>();
	AddPredecessor<MeshBoundsUpdateSystem>();
	AddPredecessor<TransformSystem>();

	// DEBUG DEBUG DEBUG
	// Remove when we figure out why shadow-map it flickers.
	//AddPredecessor<RenderCameraViewSystem>();
	// DEBUG DEBUG DEBUG

	m_meshComponentAspectId = world->GetAspectId({ typeid(TransformComponent), typeid(MeshComponent) });
	m_cameraViewAspectId = world->GetAspectId({ typeid(TransformComponent), typeid(CameraViewComponent) });
}

void RenderDirectionalShadowMapSystem::RenderLight(
	World& world,
	const FrameTime& frameTime,
	DirectionalLightComponent* light,
	CameraViewComponent* view,
	const TransformComponent* transform,
	const TransformComponent* viewTransform)
{
	SpatialIndexSystem* spatialSystem = world.GetSystem<SpatialIndexSystem>();

	Vector3 viewWorldLocation = TransformUtils::GetWorldLocation(viewTransform);
	Vector3 viewForward = TransformUtils::GetForwardVector(viewTransform);

	Matrix4 viewMatrix = Matrix4::LookAt(Vector3::Zero, TransformUtils::GetForwardVector(transform), Vector3::Up);

	// Calculate cascades based on view frustum.
	float cascadeNearZ = view->depthMin;
	float cascadeFarZ = Math::Min(view->depthMax, light->shadowDistance);

	light->shadowMapCascadeInfo.resize(light->shadowMapCascades);
	for (int i = 0; i < light->shadowMapCascades; i++)
	{
		DirectionalLightCascadeInfo& cascade = light->shadowMapCascadeInfo[i];

		// Create resources if required.
		if (cascade.shadowMapImage == nullptr)
		{
			cascade.shadowMapImage = m_graphics->CreateImage(StringFormat("Directional Shadow Map (Cascade %i)", i), light->shadowMapSize, light->shadowMapSize, 1, GraphicsFormat::UNORM_D16, false, GraphicsUsage::DepthAttachment);
			cascade.shadowMapImageView = m_graphics->CreateImageView(StringFormat("Directional Shadow Map View (Cascade %i)", i), cascade.shadowMapImage);

			// Create framebuffer.
			GraphicsFramebufferSettings settings;
			settings.width = light->shadowMapSize;
			settings.height = light->shadowMapSize;
			settings.renderPass = m_renderer->GetRenderPassForTarget(FrameBufferTarget::DepthBuffer);
			settings.attachments.push_back(cascade.shadowMapImageView);

			cascade.shadowMapFramebuffer = m_graphics->CreateFramebuffer(StringFormat("Directional Shadow Map Framebuffer (Cascade %i)", i), settings);

			SamplerDescription samplerDescription;
			cascade.shadowMapSampler = m_graphics->CreateSampler(StringFormat("Directional Shadow Map Sampler (Cascade %i)", i), samplerDescription);
		}

		float minDelta = (1.0f / light->shadowMapCascades) * (i);
		float maxDelta = (1.0f / light->shadowMapCascades) * (i + 1);
		double minDistance = CalculateCascadeDistance(cascadeNearZ, cascadeFarZ, minDelta, light->shadowMapSplitExponent);
		double maxDistance = CalculateCascadeDistance(cascadeNearZ, cascadeFarZ, maxDelta, light->shadowMapSplitExponent);

		cascade.viewFrustum = view->frustum.GetCascade(minDistance, maxDistance);

		// Convert frustum corners to light-space.
		Vector3 corners[(int)FrustumCorners::Count];
		cascade.viewFrustum.GetCorners(corners);
		cascade.splitMinDistance = minDistance;
		cascade.splitMaxDistance = maxDistance;

		for (int i = 0; i < (int)FrustumCorners::Count; i++)
		{
			corners[i] = corners[i] * viewMatrix;
		}

		// Calculate sphere that encompases all corners (we do this as a sphere 
		// is rotationally invariant so we don't get artifacts as the view area changes).
		Sphere sphere(corners, (int)FrustumCorners::Count);

		// Calculate bounding box in light-space.
		Bounds lightSpaceBounds = sphere.GetBounds();

		// Calculate projection matrix that encompases the light space bounds.
		Vector3 originLightSpace = view->frustum.GetOrigin() * viewMatrix;

		float minZ = -originLightSpace.z - 10000.0f; // todo: Shouldn't attach this to frustum. Calculate scene bounds?
		float maxZ = -originLightSpace.z + 10000.0f;

		// Keep it fixed in x and y.
		float maxSize = Math::Max(
			(lightSpaceBounds.max.x - lightSpaceBounds.min.x), 
			(lightSpaceBounds.max.y - lightSpaceBounds.min.y));
		
		cascade.projectionMatrix = Matrix4::Orthographic(
			lightSpaceBounds.min.x,
			lightSpaceBounds.max.x,//lightSpaceBounds.min.x + maxSize,
			lightSpaceBounds.min.y,
			lightSpaceBounds.max.y,//lightSpaceBounds.min.y + maxSize,
			minZ, 
			maxZ);

		// Calculate frustum from projection matrix.
		cascade.frustum = Frustum(cascade.projectionMatrix * viewMatrix);

		if (m_renderer->IsDrawBoundsEnabled())
		{
			/*{
				DrawDebugSphereMessage frusumDrawMsg;
				frusumDrawMsg.sphere = sphere;
				frusumDrawMsg.color = Color::Red;
				world.QueueMessage(frusumDrawMsg);
			}
			{
				DrawDebugBoundsMessage frusumDrawMsg;
				frusumDrawMsg.bounds = lightSpaceBounds;
				frusumDrawMsg.color = Color::Green;
				world.QueueMessage(frusumDrawMsg);
			}*/
			{
				DrawDebugFrustumMessage frusumDrawMsg;
				frusumDrawMsg.frustum = cascade.viewFrustum;
				frusumDrawMsg.color = Color::Red;
				world.QueueMessage(frusumDrawMsg);
			}
			{
				DrawDebugFrustumMessage frusumDrawMsg;
				frusumDrawMsg.frustum = cascade.frustum;
				frusumDrawMsg.color = Color::Amber;
				world.QueueMessage(frusumDrawMsg);
			}
		}

		m_renderer->DisplayDebugFrameBuffer(cascade.shadowMapImageView);
	}

	// Batch and render each cascade in parallel.
	ParallelFor(light->shadowMapCascades, [&](int index) {

		DirectionalLightCascadeInfo& cascade = light->shadowMapCascadeInfo[index];

		cascade.viewProperties.Set(ViewMatrixHash, viewMatrix);
		cascade.viewProperties.Set(ProjectionMatrixHash, cascade.projectionMatrix);
		cascade.viewProperties.UpdateResources(m_graphics, m_logger);

		// Grab all visible entities from the oct-tree.
		{
			ProfileScope scope(ProfileColors::Draw, "Search OctTree");
			spatialSystem->GetTree().Get(cascade.frustum, cascade.visibleEntitiesResult, false);
		}
		// Batch up all meshes.
		{
			ProfileScope scope(ProfileColors::Draw, "Batch Meshes");
			cascade.meshBatcher.Batch(world, m_renderer, m_logger, m_graphics, cascade.visibleEntitiesResult.entries, MaterialVariant::DepthOnly, &cascade.viewProperties,
				[=](Entity entity, const MeshComponent* mesh, const TransformComponent* transform) -> bool
				{
					std::shared_ptr<Material> material = mesh->mesh->GetMaterial().Get();
					std::shared_ptr<Shader> shader = material->GetShader().Get();
					return (shader->GetProperties().ShadowCaster);
				}
			);
		}

		// Generate command buffers for each batch.
		Array<MaterialRenderBatch*>& renderBatches = cascade.meshBatcher.GetBatches();
		cascade.batchBuffers.resize(renderBatches.size());
		
		ParallelFor((int)renderBatches.size(), [&](int index)
		{
			std::shared_ptr<IGraphicsCommandBuffer> drawBuffer = m_renderer->RequestPrimaryBuffer();
			cascade.batchBuffers[index] = drawBuffer;

			MaterialRenderBatch*& batch = renderBatches[index];

			ProfileScope scope(ProfileColors::Draw, "Render material batch: " + batch->material->GetName());

			// Generate drawing buffer.
			drawBuffer->Reset();
			drawBuffer->Begin();

			drawBuffer->BeginPass(batch->material->GetRenderPass(), cascade.shadowMapFramebuffer, true);
			drawBuffer->BeginSubPass();

			drawBuffer->SetPipeline(batch->material->GetPipeline());

			drawBuffer->SetScissor(0, 0, light->shadowMapSize, light->shadowMapSize);
			drawBuffer->SetViewport(0, 0, light->shadowMapSize, light->shadowMapSize);

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

		// Transition and clear the shadow map.
		{
			std::shared_ptr<IGraphicsCommandBuffer> clearBuffer = m_renderer->RequestPrimaryBuffer();
			clearBuffer->Reset();
			clearBuffer->Begin();

			clearBuffer->TransitionResource(cascade.shadowMapImage, GraphicsAccessMask::ReadWrite);
			clearBuffer->Clear(cascade.shadowMapImage, Color::Black, 1.0f, 0.0f);

			clearBuffer->End();

			m_renderer->QueuePrimaryBuffer("Shadow Map Clear", RenderCommandStage::View_PreRender, clearBuffer, reinterpret_cast<uint64_t>(view));
		}

		// Queue all shadow-map generation buffers.
		for (int i = 0; i < cascade.batchBuffers.size(); i++)
		{
			m_renderer->QueuePrimaryBuffer("Shadow Map Generation", RenderCommandStage::View_ShadowMap, cascade.batchBuffers[i], reinterpret_cast<uint64_t>(view));
		}

		// Transition back to reading format.
		{
			std::shared_ptr<IGraphicsCommandBuffer> drawBuffer = m_renderer->RequestPrimaryBuffer();
			drawBuffer->Reset();
			drawBuffer->Begin();

			drawBuffer->TransitionResource(cascade.shadowMapImage, GraphicsAccessMask::Read);

			drawBuffer->End();

			m_renderer->QueuePrimaryBuffer("Shadow Map Transition", RenderCommandStage::View_GBuffer, drawBuffer, reinterpret_cast<uint64_t>(view));
		}
	}, 1, "Shadow Map Cascade Building");

	// Update shadow mesh rendering properties.
	Array<Matrix4> cascadeViewProjArray;
	Array<float> cascadeSplitDistanceArray;
	Array<RenderPropertyImageSamplerValue> cascadeShadowMapsArray;

	for (int i = 0; i < Renderer::MaxShadowCascades; i++)
	{
		if (i < light->shadowMapCascades)
		{
			DirectionalLightCascadeInfo& cascade = light->shadowMapCascadeInfo[i];

			cascadeViewProjArray.push_back(cascade.projectionMatrix * viewMatrix);
			cascadeSplitDistanceArray.push_back(cascade.splitMaxDistance);
			cascadeShadowMapsArray.push_back(RenderPropertyImageSamplerValue(cascade.shadowMapImageView, cascade.shadowMapSampler));
		}
		else
		{
			DirectionalLightCascadeInfo& cascade = light->shadowMapCascadeInfo[0];

			cascadeViewProjArray.push_back(Matrix4::Identity);
			cascadeSplitDistanceArray.push_back(0.0f);
			cascadeShadowMapsArray.push_back(RenderPropertyImageSamplerValue(cascade.shadowMapImageView, cascade.shadowMapSampler));
		}
	}

	light->shadowMaskProperties.Set(LightCascadeCountHash, light->shadowMapCascades);
	light->shadowMaskProperties.Set(LightViewPositionHash, Vector4(viewWorldLocation, 1.0f));
	light->shadowMaskProperties.Set(LightCascadeViewProjHash, cascadeViewProjArray);
	light->shadowMaskProperties.Set(LightCascadeSplitDistancesHash, cascadeSplitDistanceArray);
	light->shadowMaskProperties.Set(LightCascadeShadowMapsHash, cascadeShadowMapsArray);
	light->shadowMaskProperties.UpdateResources(m_graphics, m_logger);
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

		std::shared_ptr<AspectCollection> viewCollection = world.GetAspectCollection(m_cameraViewAspectId);
		const Array<CameraViewComponent*>& viewComponents = viewCollection->GetEntityComponents<CameraViewComponent>();
		const Array<TransformComponent*>& viewTransformComponents = viewCollection->GetEntityComponents<TransformComponent>();

		for (int i = 0; i < viewComponents.size(); i++)
		{
			CameraViewComponent* view = viewComponents[i];

			for (size_t i = 0; i < entities.size(); i++)
			{
				DirectionalLightComponent* light = lights[i];
				const TransformComponent* transform = transforms[i];
				const TransformComponent* viewTransform = viewTransformComponents[i];

				if (light->isShadowCasting)
				{
					RenderLight(world, frameTime, light, view, transform, viewTransform);
				}
			}
		}
	}
}
