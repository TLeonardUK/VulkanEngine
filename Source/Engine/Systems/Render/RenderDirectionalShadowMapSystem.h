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

	AspectId m_meshComponentAspectId;

	OctTree<Entity>::Result m_visibleEntitiesResult;

	MeshBatcher m_meshBatcher;
	Array<std::shared_ptr<IGraphicsCommandBuffer>> m_batchBuffers;

private:
	void RenderLight(
		World& world,
		const FrameTime& frameTime,
		DirectionalLightComponent* light,
		const TransformComponent* transforms);

public:
	RenderDirectionalShadowMapSystem(
		std::shared_ptr<World> world,
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
