#pragma once
#include "Pch.h"

#include "Engine/Components/Transform/TransformComponent.h"
#include "Engine/Components/System.h"

// Processes updates for transform components and is responsible
// for updating dirty matrices, parent/child heirarchies, spatial indexing, etc.
class TransformSystem
	: public System<TransformComponent>
{
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
