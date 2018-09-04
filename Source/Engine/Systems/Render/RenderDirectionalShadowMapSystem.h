#pragma once
#include "Pch.h"

#include "Engine/Components/Transform/TransformComponent.h"
#include "Engine/Components/Camera/CameraViewComponent.h"
#include "Engine/Components/Lighting/DirectionalLightComponent.h"
#include "Engine/ECS/System.h"

#include "Engine/Rendering/MeshBatcher.h"

#include "Engine/Types/Frustum.h"
#include "Engine/Types/Color.h"
#include "Engine/Types/Bounds.h"
#include "Engine/Types/OctTree.h"
#include "Engine/Types/OrientedBounds.h"

class Renderer;
class RenderView;
class IGraphicsCommandBuffer;

// Generates command buffers to render a directional lights shadow map.
class RenderDirectionalShadowMapSystem
	: public System<DirectionalLightComponent, const TransformComponent>
{
private:
	std::shared_ptr<Renderer> m_renderer;
	std::shared_ptr<IGraphics> m_graphics;
	std::shared_ptr<Logger> m_logger;

	AspectId m_meshComponentAspectId;
	AspectId m_cameraViewAspectId;

private:
	void RenderLight(
		World& world,
		const FrameTime& frameTime,
		DirectionalLightComponent* light,
		CameraViewComponent* view,
		const TransformComponent* transforms,
		const TransformComponent* viewTransform);

public:
	RenderDirectionalShadowMapSystem(
		std::shared_ptr<World> world,
		std::shared_ptr<Logger> logger,
		std::shared_ptr<Renderer> renderer,
		std::shared_ptr<ResourceManager> resourceManager,
		std::shared_ptr<IGraphics> graphics);

	virtual void Tick(
		World& world,
		const FrameTime& frameTime,
		const Array<Entity>& entities,
		const Array<DirectionalLightComponent*>& lights,
		const Array<const TransformComponent*>& transforms);
};
