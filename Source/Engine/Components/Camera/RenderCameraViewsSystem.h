#pragma once
#include "Pch.h"

#include "Engine/Components/Transform/TransformComponent.h"
#include "Engine/Components/Camera/CameraViewComponent.h"
#include "Engine/Components/System.h"

#include "Engine/Types/Frustum.h"
#include "Engine/Types/Color.h"
#include "Engine/Types/Bounds.h"
#include "Engine/Types/OrientedBounds.h"

class Renderer;
class RenderView;

// Renders a debug line to the camera view.
struct DrawDebugLineMessage
{
	Vector3 start;
	Vector3 end;
	Color color;
};

// Renders a debug axis-aligned bounds to the camera view.
struct DrawDebugBoundsMessage
{
	Bounds bounds;
	Color color;
};

// Renders a debug oriented bounds to the camera view.
struct DrawDebugOrientedBoundsMessage
{
	OrientedBounds bounds;
	Color color;
};

// Renders a debug frustum to the camera view.
struct DrawDebugFrustumMessage
{
	Frustum frustum;
	Color color;
};

// Generates command buffers to render each camera view.
class RenderCameraViewsSystem 
	: public System<CameraViewComponent, const TransformComponent>
{
private:
	std::shared_ptr<Renderer> m_renderer;

	AspectId m_meshComponentAspectId;

private:
	void TickView(
		World& world,
		const FrameTime& frameTime,
		CameraViewComponent* views,
		const TransformComponent* transforms,
		const std::shared_ptr<RenderView>& renderView);

	void GatherVisibleMeshes(
		World& world, 
		const std::shared_ptr<RenderView>& renderView);

public:
	RenderCameraViewsSystem(std::shared_ptr<World> world, std::shared_ptr<Renderer> renderer);

	virtual void Tick(
		World& world, 
		const FrameTime& frameTime, 
		const Array<Entity>& entities, 
		const Array<CameraViewComponent*>& views, 
		const Array<const TransformComponent*>& transforms);
};
