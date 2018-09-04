#include "Pch.h"

#include "Engine/Components/Mesh/MeshComponent.h"
#include "Engine/Components/Lighting/DirectionalLightComponent.h"

#include "Engine/Systems/Render/RenderCameraViewSystem.h"
#include "Engine/Systems/Render/RenderCameraViewShadowMaskSystem.h"
#include "Engine/Systems/Render/RenderDirectionalShadowMapSystem.h"
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

	m_directionalComponentAspectId = world->GetAspectId({ typeid(DirectionalLightComponent) });

	m_directionalShadowMaskMaterial = m_resourceManager->Load<Material>("Engine/Materials/directional_shadow_mask.json");
	m_directionalShadowMaskMaterial.WaitUntilLoaded();

	m_renderer->CreateMeshRenderState(&m_directionalShadowMaskRenderState);
}

void RenderCameraViewShadowMaskSystem::TickView(
	World& world,
	const FrameTime& frameTime,
	CameraViewComponent* view)
{
	std::shared_ptr<AspectCollection> directionalLights = world.GetAspectCollection(m_directionalComponentAspectId);

	const Array<DirectionalLightComponent*>& components = directionalLights->GetEntityComponents<DirectionalLightComponent>();

	// Render each directional light to shadow mask.
	for (int i = 0; i < components.size(); i++)
	{
		DirectionalLightComponent* light = components[i];

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

		m_renderer->QueuePrimaryBuffer("Shadow Mask Generation", RenderCommandStage::View_ShadowMask, drawBuffer, reinterpret_cast<uint64_t>(view));			
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
