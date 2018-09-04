#pragma once
#include "Pch.h"

#include "Engine/Components/Transform/TransformComponent.h"
#include "Engine/Components/Transform/BoundsComponent.h"
#include "Engine/Components/Mesh/MeshComponent.h"
#include "Engine/ECS/System.h"

class Graphics;
class Logger;

// Updates the transform based render state of each mesh.
class MeshRenderStateUpdateSystem
	: public System<MeshComponent, const TransformComponent>
{
private:
	std::shared_ptr<IGraphics> m_graphics;
	std::shared_ptr<Logger> m_logger;

public:
	MeshRenderStateUpdateSystem(
		std::shared_ptr<IGraphics> graphics,
		std::shared_ptr<Logger> logger);

	virtual void Tick(
		World& world,
		const FrameTime& frameTime,
		const Array<Entity>& entities,
		const Array<MeshComponent*>& meshes,
		const Array<const TransformComponent*>& transforms);
};
