#include "Pch.h"

#include "Engine/Components/Mesh/ModelComponent.h"

#include "Engine/Systems/Mesh/MeshBoundsUpdateSystem.h"
#include "Engine/Systems/Transform/TransformSystem.h"
#include "Engine/Systems/Transform/SpatialIndexSystem.h"
#include "Engine/Systems/Render/RenderDebugSystem.h"

#include "Engine/Rendering/Renderer.h"

#include "Engine/Profiling/Profiling.h"
#include "Engine/Threading/ParallelFor.h"

MeshBoundsUpdateSystem::MeshBoundsUpdateSystem(std::shared_ptr<Renderer> renderer)
	: m_renderer(renderer)
{
	AddPredecessor<TransformSystem>();
}

void MeshBoundsUpdateSystem::Tick(
	World& world,
	const FrameTime& frameTime,
	const Array<Entity>& entities,
	const Array<BoundsComponent*>& bounds,
	const Array<const MeshComponent*>& meshes,
	const Array<const TransformComponent*>& transforms)
{
	// Remove any deleted components from spatial index.
	for (auto& message : world.PeekMessages<DeletedComponentMessage>())
	{
		// todo: this is kinda bleh, fix plz.
		if (message.componentType == typeid(BoundsComponent))
		{
			RemoveFromSpatialIndexMessage removeMessage;
			removeMessage.entity = message.entity;
			world.QueueMessage(removeMessage);
		}
	}

	// Update bounds.
	ParallelFor(static_cast<int>(entities.size()), [&](int i)
	{
		const TransformComponent* transform = transforms[i];
		const MeshComponent* mesh = meshes[i];
		BoundsComponent* bound = bounds[i];

		if (transform->version != bound->lastTransformVersion)
		{
			bound->bounds = OrientedBounds(mesh->mesh->GetBounds(), transform->localToWorld);
			bound->lastTransformVersion = transform->version;

			UpdateSpatialIndexMessage message;
			message.entity = entities[i];
			message.bounds = bound->bounds.GetAlignedBounds();
			world.QueueMessage(message);
		}

		// Draw bounds.
		if (false)//m_renderer->IsDrawBoundsEnabled())
		{
			DrawDebugOrientedBoundsMessage boundsDrawMsg;
			boundsDrawMsg.bounds = bound->bounds;
			boundsDrawMsg.color = Color::Blue;
			world.QueueMessage(boundsDrawMsg);
		}
	}, 16, "Bounds Update");
}