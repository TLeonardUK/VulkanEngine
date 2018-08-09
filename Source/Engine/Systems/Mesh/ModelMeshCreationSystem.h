#pragma once
#include "Pch.h"

#include "Engine/Components/Transform/TransformComponent.h"
#include "Engine/Components/Transform/BoundsComponent.h"
#include "Engine/Components/Mesh/ModelComponent.h"
#include "Engine/Components/Mesh/MeshComponent.h"
#include "Engine/ECS/System.h"

// Whenever the model of a ModelComponent is updated, this system is
// responsible for creating and updating the sub MeshComponents that
// the model is made up from.
class ModelMeshCreationSystem
	: public System<ModelComponent>
{
public:
	ModelMeshCreationSystem();

	virtual void Tick(
		World& world,
		const FrameTime& frameTime,
		const Array<Entity>& entities,
		const Array<ModelComponent*>& models);
};
