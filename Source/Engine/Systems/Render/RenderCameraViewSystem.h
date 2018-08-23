#pragma once
#include "Pch.h"

#include "Engine/Components/Transform/TransformComponent.h"
#include "Engine/Components/Camera/CameraViewComponent.h"
#include "Engine/ECS/System.h"

#include "Engine/Rendering/MeshBatcher.h"

#include "Engine/Types/Frustum.h"
#include "Engine/Types/Color.h"
#include "Engine/Types/Bounds.h"
#include "Engine/Types/OctTree.h"
#include "Engine/Types/OrientedBounds.h"

class Renderer;
class RenderView;
class IGraphics;
class IGraphicsCommandBuffer;

// Generates command buffers to render each camera view.
class RenderCameraViewSystem 
	: public System<CameraViewComponent, const TransformComponent>
{
private:
	std::shared_ptr<Renderer> m_renderer;
	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<IGraphics> m_graphics;

	AspectId m_meshComponentAspectId;

	OctTree<Entity>::Result m_visibleEntitiesResult;
	MeshBatcher m_meshBatcher;

	Array<std::shared_ptr<IGraphicsCommandBuffer>> m_batchBuffers;

private:
	void TickView(
		World& world,
		const FrameTime& frameTime,
		CameraViewComponent* views,
		const TransformComponent* transforms);

public:
	RenderCameraViewSystem(
		std::shared_ptr<World> world,
		std::shared_ptr<IGraphics> graphics,
		std::shared_ptr<Logger> logger,
		std::shared_ptr<Renderer> renderer);

	virtual void Tick(
		World& world, 
		const FrameTime& frameTime, 
		const Array<Entity>& entities, 
		const Array<CameraViewComponent*>& views, 
		const Array<const TransformComponent*>& transforms);
};
