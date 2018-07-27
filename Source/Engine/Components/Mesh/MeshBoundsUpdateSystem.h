#pragma once
#include "Pch.h"

#include "Engine/Components/Transform/TransformComponent.h"
#include "Engine/Components/Mesh/MeshComponent.h"
#include "Engine/Components/System.h"

// Updates the world bounds of each mesh component.
class MeshBoundsUpdateSystem
	: public System<MeshComponent, const TransformComponent>
{
public:
	MeshBoundsUpdateSystem();

	virtual void Tick(
		World& world,
		const FrameTime& frameTime,
		const Array<Entity>& entities,
		const Array<MeshComponent*>& meshes,
		const Array<const TransformComponent*>& transforms);
};
