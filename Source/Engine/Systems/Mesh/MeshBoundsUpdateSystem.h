#pragma once
#include "Pch.h"

#include "Engine/Components/Transform/TransformComponent.h"
#include "Engine/Components/Transform/BoundsComponent.h"
#include "Engine/Components/Mesh/MeshComponent.h"
#include "Engine/ECS/System.h"

// Updates the bounds of each mesh component.
class MeshBoundsUpdateSystem
	: public System<BoundsComponent, const MeshComponent, const TransformComponent>
{
private:
	std::shared_ptr<Renderer> m_renderer;

public:
	MeshBoundsUpdateSystem(std::shared_ptr<Renderer> renderer);

	virtual void Tick(
		World& world,
		const FrameTime& frameTime,
		const Array<Entity>& entities,
		const Array<BoundsComponent*>& bounds,
		const Array<const MeshComponent*>& meshes,
		const Array<const TransformComponent*>& transforms);
};
