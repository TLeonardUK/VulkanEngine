#pragma once
#include "Pch.h"

#include "Engine/Resources/Types/Model.h"
#include "Engine/Rendering/MeshRenderState.h"
#include "Engine/Types/OrientedBounds.h"

#include "Engine/ECS/Component.h"

struct ModelComponent;

// Describes a sub-mesh of a model component.
struct MeshComponent
{
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<MeshRenderState> renderData[(int)MaterialVariant::Count];
	RenderPropertyCollection properties;
};
