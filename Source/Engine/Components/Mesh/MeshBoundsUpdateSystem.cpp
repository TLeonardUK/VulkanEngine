#include "Pch.h"

#include "Engine/Components/Mesh/MeshBoundsUpdateSystem.h"
#include "Engine/Components/Transform/TransformSystem.h"

#include "Engine/Profiling/Profiling.h"
#include "Engine/Threading/ParallelFor.h"

MeshBoundsUpdateSystem::MeshBoundsUpdateSystem()
{
	AddPredecessor<TransformSystem>();
}

void MeshBoundsUpdateSystem::Tick(
	World& world,
	const FrameTime& frameTime,
	const Array<Entity>& entities, 
	const Array<MeshComponent*>& meshes,
	const Array<const TransformComponent*>& transforms)
{
	// Consume all relevant messages.
	for (auto& message : world.ConsumeMessages<SetMeshModelMessage>())
	{
		MeshComponent* component = message.component.Get(world);
		if (component != nullptr)
		{
			component->model = message.model;
		}
	}

	// Update bounds.
	ParallelFor(entities.size(), [&](int i) 
	{
		const TransformComponent* transform = transforms[i];
		MeshComponent* mesh = meshes[i];

		std::shared_ptr<Model> model = mesh->model.Get();

		if (transform->version != mesh->lastBoundsUpdateTransformVersion ||
			model != mesh->lastBoundsUpdateModel)
		{
			if (model != nullptr)
			{
				const Array<std::shared_ptr<Mesh>>& meshes = model->GetMeshes();
				mesh->meshBounds.resize(meshes.size());

				ParallelFor(mesh->meshBounds.size(), [&](int j)
				{
					mesh->meshBounds[j] = OrientedBounds(meshes[j]->GetBounds(), transform->localToWorld);
				}, 16, "Sub-Mesh Bounds Update");
			}
		}
		
		mesh->lastBoundsUpdateModel = model;
		mesh->lastBoundsUpdateTransformVersion = transform->version;
	}, 16, "Bounds Update");
}
