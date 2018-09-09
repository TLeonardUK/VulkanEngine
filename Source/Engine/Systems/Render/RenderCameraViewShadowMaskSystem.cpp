#include "Pch.h"

#include "Engine/Components/Mesh/MeshComponent.h"
#include "Engine/Components/Lighting/DirectionalLightComponent.h"

#include "Engine/Systems/Render/RenderCameraViewSystem.h"
#include "Engine/Systems/Render/RenderCameraViewShadowMaskSystem.h"
#include "Engine/Systems/Render/RenderDirectionalShadowMapSystem.h"
#include "Engine/Systems/Render/RenderSpotLightShadowMapSystem.h"
#include "Engine/Systems/Render/RenderPointLightShadowMapSystem.h"
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

RenderCameraViewShadowMaskSystem::RenderCameraViewShadowMaskSystem(
	std::shared_ptr<World> world,
	std::shared_ptr<IGraphics> graphics,
	std::shared_ptr<Logger> logger,
	std::shared_ptr<Renderer> renderer,
	std::shared_ptr<ResourceManager> resourceManager)
	: m_renderer(renderer)
	, m_logger(logger)
	, m_graphics(graphics)
	, m_resourceManager(resourceManager)
{
	AddPredecessor<RenderCameraViewSystem>();
	AddPredecessor<RenderDirectionalShadowMapSystem>();
	AddPredecessor<RenderSpotLightShadowMapSystem>();
	AddPredecessor<RenderPointLightShadowMapSystem>();

	m_directionalComponentAspectId = world->GetAspectId({ typeid(DirectionalLightComponent) });
	m_spotComponentAspectId = world->GetAspectId({ typeid(SpotLightComponent) });
	m_pointComponentAspectId = world->GetAspectId({ typeid(PointLightComponent) });

	m_directionalShadowMaskMaterial = m_resourceManager->Load<Material>("Engine/Materials/directional_shadow_mask.json");
	m_spotlightShadowMaskMaterial = m_resourceManager->Load<Material>("Engine/Materials/spotlight_shadow_mask.json");
	m_pointShadowMaskMaterial = m_resourceManager->Load<Material>("Engine/Materials/point_light_shadow_mask.json");

	m_directionalShadowMaskMaterial.WaitUntilLoaded();
	m_spotlightShadowMaskMaterial.WaitUntilLoaded();
	m_pointShadowMaskMaterial.WaitUntilLoaded();

	m_renderer->CreateMeshRenderState(&m_directionalShadowMaskRenderState);
	m_renderer->CreateMeshRenderState(&m_spotlightShadowMaskRenderState);
	m_renderer->CreateMeshRenderState(&m_pointShadowMaskRenderState);
}

void RenderCameraViewShadowMaskSystem::TickView(
	World& world,
	const FrameTime& frameTime,
	CameraViewComponent* view)
{
	std::shared_ptr<AspectCollection> directionalLightCollection = world.GetAspectCollection(m_directionalComponentAspectId);
	const Array<DirectionalLightComponent*>& directionalLightComponents = directionalLightCollection->GetEntityComponents<DirectionalLightComponent>();

	std::shared_ptr<AspectCollection> spotLightCollection = world.GetAspectCollection(m_spotComponentAspectId);
	const Array<SpotLightComponent*>& spotLightComponents = spotLightCollection->GetEntityComponents<SpotLightComponent>();

	std::shared_ptr<AspectCollection> pointLightCollection = world.GetAspectCollection(m_pointComponentAspectId);
	const Array<PointLightComponent*>& pointLightComponents = pointLightCollection->GetEntityComponents<PointLightComponent>();

	// Render each directional light to shadow mask.
	for (int i = 0; i < directionalLightComponents.size(); i++)
	{
		DirectionalLightComponent* light = directionalLightComponents[i];
		if (light->isShadowCasting)
		{
			std::shared_ptr<IGraphicsCommandBuffer> drawBuffer = m_renderer->RequestPrimaryBuffer();

			drawBuffer->Reset();
			drawBuffer->Begin();

			m_renderer->DrawFullScreenQuad(
				drawBuffer,
				m_directionalShadowMaskMaterial.Get(),
				&m_directionalShadowMaskRenderState,
				&view->viewProperties,
				&light->shadowMaskProperties
			);

			drawBuffer->End();

			m_renderer->QueuePrimaryBuffer("Directional Light Shadow Mask Generation", RenderCommandStage::View_ShadowMask, drawBuffer, reinterpret_cast<uint64_t>(view));
		}
	}

	// Render each spot light to shadow mask.
	for (int i = 0; i < spotLightComponents.size(); i++)
	{
		SpotLightComponent* light = spotLightComponents[i];
		if (light->isShadowCasting)
		{
			std::shared_ptr<IGraphicsCommandBuffer> drawBuffer = m_renderer->RequestPrimaryBuffer();

			drawBuffer->Reset();
			drawBuffer->Begin();

			m_renderer->DrawFullScreenQuad(
				drawBuffer,
				m_spotlightShadowMaskMaterial.Get(),
				&m_spotlightShadowMaskRenderState,
				&view->viewProperties,
				&light->shadowInfo.shadowMaskProperties
			);

			drawBuffer->End();

			m_renderer->QueuePrimaryBuffer("Spot Light Shadow Mask Generation", RenderCommandStage::View_ShadowMask, drawBuffer, reinterpret_cast<uint64_t>(view));
		}
	}

	// Render each point light to shadow mask.
	for (int i = 0; i < pointLightComponents.size(); i++)
	{
		PointLightComponent* light = pointLightComponents[i];
		if (light->isShadowCasting)
		{
			std::shared_ptr<IGraphicsCommandBuffer> drawBuffer = m_renderer->RequestPrimaryBuffer();

			drawBuffer->Reset();
			drawBuffer->Begin();

			m_renderer->DrawFullScreenQuad(
				drawBuffer,
				m_pointShadowMaskMaterial.Get(),
				&m_pointShadowMaskRenderState,
				&view->viewProperties,
				&light->shadowInfo.shadowMaskProperties
			);

			drawBuffer->End();

			m_renderer->QueuePrimaryBuffer("Point Light Shadow Mask Generation", RenderCommandStage::View_ShadowMask, drawBuffer, reinterpret_cast<uint64_t>(view));
		}
	}
}

void RenderCameraViewShadowMaskSystem::Tick(
	World& world,
	const FrameTime& frameTime,
	const Array<Entity>& entities,
	const Array<CameraViewComponent*>& views)
{
	ProfileScope scope(ProfileColors::Draw, "Generate view shadow mask's");

	for (size_t i = 0; i < entities.size(); i++)
	{
		CameraViewComponent* view = views[i];

		TickView(world, frameTime, view);
	}
}
