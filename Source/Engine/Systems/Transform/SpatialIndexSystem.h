#pragma once
#include "Pch.h"

#include "Engine/Components/Transform/TransformComponent.h"
#include "Engine/Components/Transform/BoundsComponent.h"
#include "Engine/ECS/System.h"
#include "Engine/Types/Set.h"
#include "Engine/Types/Array.h"
#include "Engine/Types/OctTree.h"

// Stores entities with bounding boxes into a spatial heirarchy for 
// fast querying.
class SpatialIndexSystem
	: public System<>
{
private:
	OctTree<Entity> m_tree;
	Dictionary<Entity, OctTree<Entity>::Token> m_entityTokens;

public:
	SpatialIndexSystem();

	const OctTree<Entity>& GetTree();

	virtual void Tick(
		World& world,
		const FrameTime& frameTime,
		const Array<Entity>& entities);
};

// Inserts a bounds component into spatial index.
struct UpdateSpatialIndexMessage
{
	Entity entity;
	Bounds bounds;
};

// Removes a bounds component into spatial index.
struct RemoveFromSpatialIndexMessage
{
	Entity entity;
};