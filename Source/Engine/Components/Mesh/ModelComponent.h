#pragma once
#include "Pch.h"

#include "Engine/Resources/Types/Model.h"
#include "Engine/Resources/Types/MaterialRenderData.h"
#include "Engine/Types/OrientedBounds.h"

#include "Engine/ECS/Component.h"

// Describes an individual model. 
struct ModelComponent
{
	ResourcePtr<Model> model;
	Array<Entity> subMeshEntities;
	std::weak_ptr<Model> lastUpdateModel;	
};

// Changes the model of a mesh component.
/*
struct SetMeshModelMessage
{
ComponentRef<MeshComponent> component = NoEntity;

ResourcePtr<Model> model;
};
*/
