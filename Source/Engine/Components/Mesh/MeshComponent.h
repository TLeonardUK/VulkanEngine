#pragma once
#include "Pch.h"

#include "Engine/Resources/Types/Model.h"
#include "Engine/Types/OrientedBounds.h"

#include "Engine/Components/Component.h"

// Describes an individual static mesh.
struct MeshComponent
{
	ResourcePtr<Model> model;
	Array<OrientedBounds> bounds;
	MaterialPropertyCollection properties;

	Array<std::shared_ptr<MaterialRenderData>> renderData;

	uint64_t lastBoundsUpdateTransformVersion;
	std::shared_ptr<Model> lastBoundsUpdateModel;
};

// Changes the model of a mesh component.
struct SetMeshModelMessage
{
	ComponentRef<MeshComponent> component;

	ResourcePtr<Model> model;
};
