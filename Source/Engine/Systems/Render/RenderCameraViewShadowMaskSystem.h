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
class Material;
class IGraphics;
class IGraphicsCommandBuffer;

// Generates command buffers to render each camera-views shadow mask.
class RenderCameraViewShadowMaskSystem
	: public System<CameraViewComponent>
{
private:
	std::shared_ptr<Renderer> m_renderer;
	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<IGraphics> m_graphics;
	std::shared_ptr<ResourceManager> m_resourceManager;

	ResourcePtr<Material> m_directionalShadowMaskMaterial;
	std::shared_ptr<MeshRenderState> m_directionalShadowMaskRenderState;

	ResourcePtr<Material> m_spotlightShadowMaskMaterial;
	std::shared_ptr<MeshRenderState> m_spotlightShadowMaskRenderState;

	ResourcePtr<Material> m_pointShadowMaskMaterial;
	std::shared_ptr<MeshRenderState> m_pointShadowMaskRenderState;
	
	AspectId m_directionalComponentAspectId;
	AspectId m_spotComponentAspectId;
	AspectId m_pointComponentAspectId;

private:
	void TickView(
		World& world,
		const FrameTime& frameTime,
		CameraViewComponent* views);

public:
	RenderCameraViewShadowMaskSystem(
		std::shared_ptr<World> world,
		std::shared_ptr<IGraphics> graphics,
		std::shared_ptr<Logger> logger,
		std::shared_ptr<Renderer> renderer,
		std::shared_ptr<ResourceManager> resourceManager);

	virtual void Tick(
		World& world,
		const FrameTime& frameTime,
		const Array<Entity>& entities,
		const Array<CameraViewComponent*>& views);
};
