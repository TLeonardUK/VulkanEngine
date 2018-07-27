#pragma once
#include "Pch.h"

#include "Engine/Resources/Types/Model.h"
#include "Engine/Types/OrientedBounds.h"

// Describes an individual static mesh.
struct MeshComponent
{
	ResourcePtr<Model> model;
	Array<OrientedBounds> meshBounds;

	uint64_t lastBoundsUpdateTransformVersion;
	std::shared_ptr<Model> lastBoundsUpdateModel;

	MaterialPropertyCollection& meshProperties;
};

// Changes the model of a mesh component.
struct SetMeshModelMessage
{
	ComponentRef<MeshComponent> component;

	ResourcePtr<Model> model;
};
