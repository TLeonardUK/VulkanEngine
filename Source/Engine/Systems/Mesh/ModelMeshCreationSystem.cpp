#include "Pch.h"

#include "Engine/Components/Transform/BoundsComponent.h"
#include "Engine/Components/Transform/TransformComponent.h"
#include "Engine/Components/Mesh/MeshComponent.h"

#include "Engine/Systems/Mesh/ModelMeshCreationSystem.h"
#include "Engine/Systems/Mesh/MeshBoundsUpdateSystem.h"
#include "Engine/Systems/Transform/TransformSystem.h"

#include "Engine/Profiling/Profiling.h"
#include "Engine/Threading/ParallelFor.h"

ModelMeshCreationSystem::ModelMeshCreationSystem()
{
}

void ModelMeshCreationSystem::Tick(
	World& world,
	const FrameTime& frameTime,
	const Array<Entity>& entities,
	const Array<ModelComponent*>& models)
{
	for (int i = 0; i < entities.size(); i++)
	{
		ModelComponent* model = models[i];

		std::shared_ptr<Model> currentModel = model->model.Get();
	
		if (model->lastUpdateModel.lock() != currentModel)
		{
			// Clean up old sub-meshes.
			for (Entity& entity : model->subMeshEntities)
			{
				world.DestroyEntity(entity);
			}
			model->subMeshEntities.clear();

			// Create new sub-meshes.
			for (auto& mesh : currentModel->GetMeshes())
			{
				Entity subMesh = world.CreateEntity();				
				world.AddComponent<BoundsComponent>(subMesh);

				MeshComponent* meshComponent = world.AddComponent<MeshComponent>(subMesh);
				meshComponent->mesh = mesh;

				TransformComponent* transform = world.AddComponent<TransformComponent>(subMesh);
				transform->parent = entities[i];

				model->subMeshEntities.push_back(subMesh);
			}

			model->lastUpdateModel = currentModel;
		}
	}
}

