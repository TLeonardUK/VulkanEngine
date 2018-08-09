#pragma once
#include "Pch.h"

#include "Engine/Components/Transform/TransformComponent.h"
#include "Engine/ECS/System.h"
#include "Engine/Types/Set.h"
#include "Engine/Types/Array.h"

// Processes updates for transform components and is responsible
// for updating dirty matrices, parent/child heirarchies, spatial indexing, etc.
class TransformSystem
	: public System<TransformComponent>
{
private:
	Array<Set<TransformComponent*>> m_asyncDirtyRoots;
	Array<Array<TransformComponent*>> m_asyncDirtyRootsList;

private:
	void UpdateTransform(
		World& world, 
		TransformComponent* transform,
		TransformComponent* parentTransform);

public:
	TransformSystem();

	virtual void Tick(
		World& world, 
		const FrameTime& frameTime, 
		const Array<Entity>& entities, 
		const Array<TransformComponent*>& transforms);
};
