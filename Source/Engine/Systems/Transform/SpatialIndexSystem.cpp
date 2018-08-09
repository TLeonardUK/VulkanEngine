#include "Pch.h"

#include "Engine/Components/Transform/BoundsComponent.h"
#include "Engine/Components/Transform/TransformComponent.h"
#include "Engine/Components/Mesh/MeshComponent.h"

#include "Engine/Systems/Mesh/ModelMeshCreationSystem.h"
#include "Engine/Systems/Mesh/MeshBoundsUpdateSystem.h"
#include "Engine/Systems/Transform/TransformSystem.h"
#include "Engine/Systems/Transform/SpatialIndexSystem.h"

#include "Engine/Profiling/Profiling.h"
#include "Engine/Threading/ParallelFor.h"

SpatialIndexSystem::SpatialIndexSystem()
	: m_tree(Vector3(30000.0f, 30000.0f, 30000.0f), 7) // (10000 / (2 ^ 10)) = ~9.765 per leaf cell.
{
	AddSuccessor<TransformSystem>();
	AddSuccessor<MeshBoundsUpdateSystem>();
}

const OctTree<Entity>& SpatialIndexSystem::GetTree()
{
	return m_tree;
}

void SpatialIndexSystem::Tick(
	World& world,
	const FrameTime& frameTime,
	const Array<Entity>& entities)
{
	// Remove tree entries.
	for (auto& message : world.ConsumeMessages<RemoveFromSpatialIndexMessage>())
	{
		auto iter = m_entityTokens.find(message.entity);
		if (iter != m_entityTokens.end())
		{
			m_tree.Remove(m_entityTokens[message.entity]);
			m_entityTokens.erase(iter);
		}
	}

	// Update tree entries.
	for (auto& message : world.ConsumeMessages<UpdateSpatialIndexMessage>())
	{
		auto iter = m_entityTokens.find(message.entity);
		if (iter != m_entityTokens.end())
		{
			m_tree.Remove(m_entityTokens[message.entity]);
			m_entityTokens.erase(iter);
		}

		OctTree<Entity>::Token token = m_tree.Add(message.bounds, message.entity);
		m_entityTokens.emplace(message.entity, token);
	}

	// Draw bounds.
	//Array<const OctTree<Entity>::Node*> nodes;
	//m_tree.GetNodes(nodes);
	/*for (auto& cell : nodes)
	{
		if (cell->level == 0 || cell->level == 1)
		{
			DrawDebugBoundsMessage boundsDrawMsg;
			boundsDrawMsg.bounds = cell->bounds;
			boundsDrawMsg.color = Color(1.0f, 0.0f, 0.0f, 1.0f);
			world.QueueMessage(boundsDrawMsg);
		}
		if (cell->level == 0)
		{
			for (auto& entry : cell->entries)
			{
				DrawDebugBoundsMessage boundsDrawMsg;
				boundsDrawMsg.bounds = entry.second.bounds;
				boundsDrawMsg.color = Color(0.0f, 1.0f, 0.0f, 1.0f);
				world.QueueMessage(boundsDrawMsg);
			}
		}
	}*/
}

