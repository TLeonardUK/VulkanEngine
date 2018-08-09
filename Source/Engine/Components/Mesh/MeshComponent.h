#pragma once
#include "Pch.h"

#include "Engine/Resources/Types/Model.h"
#include "Engine/Resources/Types/MaterialRenderData.h"
#include "Engine/Types/OrientedBounds.h"

#include "Engine/ECS/Component.h"

struct ModelComponent;

// Describes a sub-mesh of a model component.
struct MeshComponent
{
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<MaterialRenderData> renderData[(int)MaterialVariant::Count];
	MaterialPropertyCollection properties;
};
