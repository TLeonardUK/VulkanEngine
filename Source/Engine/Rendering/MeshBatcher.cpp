#include "Pch.h"

#include "Engine/Components/Mesh/MeshComponent.h"
#include "Engine/Components/Transform/TransformComponent.h"

#include "Engine/Rendering/MeshBatcher.h"
#include "Engine/Rendering/Renderer.h"

#include "Engine/Profiling/Profiling.h"
#include "Engine/Threading/ParallelFor.h"
#include "Engine/Threading/TaskManager.h"

#include "Engine/Utilities/Statistic.h"

#include "Engine/Types/Math.h"

extern Statistic Stat_Rendering_Budgets_MeshesRendered;
extern Statistic Stat_Rendering_Budgets_BatchesRendered;
extern Statistic Stat_Rendering_Budgets_TrianglesRendered;

extern const MaterialPropertyHash ModelMatrixHash;

Array<MaterialRenderBatch>& MeshBatcher::GetBatches()
{
	return m_materialRenderBatches;
}

void MeshBatcher::Batch(World& world, std::shared_ptr<Renderer>& renderer, const Array<Entity>& entities, MaterialVariant variant)
{
	ProfileScope scope(Color::Red, "Renderer::BuildViewCommandBuffer_Meshes");

	if (m_meshComponentAspectId == -1)
	{
		m_meshComponentAspectId = world.GetAspectId({ typeid(TransformComponent), typeid(MeshComponent) });
	}

	std::shared_ptr<AspectCollection> collection = world.GetAspectCollection(m_meshComponentAspectId);
	const Dictionary<Entity, int>& entityIndexMap = collection->GetEntitiesIndexMap();
	const Array<MeshComponent*>& meshComponents = collection->GetEntityComponents<MeshComponent>();
	const Array<TransformComponent*>& meshTransformComponents = collection->GetEntityComponents<TransformComponent>();

	// Batch meshes by material.
	int splitFactor = TaskManager::AsyncInstance->GetConcurrency() * 4;
	int chunkSize = (int)Math::Ceil(entities.size() / (float)splitFactor);

	// Resize the batch lists so they have enough space for each material.
	{
		ProfileScope scope(Color::Red, "Resize Batch Lists");

		m_asyncMaterialBatches.resize(splitFactor);
		m_materialRenderBatches.clear();
	}

	// Batch all meshes in parallel.
	ParallelFor(splitFactor, [&](int index)
	{
		ProfileScope scope(Color::Red, "Batch meshes");

		Dictionary<Material*, MaterialBatch>& batches = m_asyncMaterialBatches[index];

		for (auto& iter : batches)
		{
			iter.second.meshInstanceCount = 0;
			if (iter.second.meshInstances.size() < entities.size())
			{
				iter.second.meshInstances.resize(entities.size());
			}
		}

		int startIndex = (index * chunkSize);
		int endIndex = Math::Min((int)entities.size(), startIndex + chunkSize);

		for (int i = startIndex; i < endIndex; i++)
		{
			//ProfileScope scope(Color::Red, "Mesh");
			Entity entity = entities[i];

			auto iter = entityIndexMap.find(entity);
			if (iter == entityIndexMap.end())
			{
				return;
			}

			int entityArrayIndex = iter->second;
			MeshComponent* meshComponent = meshComponents[entityArrayIndex];
			TransformComponent* transformComponent = meshTransformComponents[entityArrayIndex];

			std::shared_ptr<Material> material = nullptr;
			material = meshComponent->mesh->GetMaterial().Get()->GetVariant(variant);
	
			Material* materialPtr = &*material;			

			MaterialBatch* batch = nullptr;

			auto batchIter = batches.find(materialPtr);
			if (batchIter != batches.end())
			{
				batch = &batchIter->second;
			}
			else
			{
				batches.emplace(materialPtr, MaterialBatch());

				batch = &batches[materialPtr];
				batch->material = materialPtr;
				batch->meshInstanceCount = 0;
				batch->meshInstances.resize(entities.size());
			}

			std::shared_ptr<MaterialRenderData>& renderData = meshComponent->renderData[(int)variant];

			// Update mesh state.
			//{
			//	ProfileScope scope(Color::Red, "UpdateResources");
				meshComponent->mesh->UpdateResources();
			//}

			//{
			//	ProfileScope scope(Color::Red, "UpdateMaterialRenderData");
				meshComponent->properties.Set(ModelMatrixHash, transformComponent->localToWorld);

				renderer->UpdateMaterialRenderData(
					&renderData,
					material,
					&meshComponent->properties);
			//}

			//{
			//	ProfileScope scope(Color::Red, "InstanceData");

				// Create new instance.
				MeshInstance& instance = batch->meshInstances[batch->meshInstanceCount++];
				instance.resourceSets = &renderData->GetResourceSets();
				instance.indexBuffer = &meshComponent->mesh->GetIndexBuffer();
				instance.vertexBuffer = &meshComponent->mesh->GetVertexBuffer();
				instance.indexCount = meshComponent->mesh->GetIndexCount();

				Stat_Rendering_Budgets_MeshesRendered.Add(1);
				Stat_Rendering_Budgets_TrianglesRendered.Add(instance.indexCount / 3);
			//}
		}
	}, 1, "Mesh Batching");

	// Make list of all batch types available.
	// todo: should be able to do this in parallel?
	int totalMeshInstances = 0;
	{
		ProfileScope scope(Color::Red, "Combine Batches");

		for (int i = 0; i < splitFactor; i++)
		{
			for (auto& iter : m_asyncMaterialBatches[i])
			{
				if (iter.second.meshInstanceCount > 0)
				{
					int instanceOffset = 0;
					while (instanceOffset < iter.second.meshInstanceCount)
					{
						MaterialRenderBatch* batch = nullptr;

						for (int j = 0; j < m_materialRenderBatches.size(); j++)
						{
							if (m_materialRenderBatches[j].material == iter.first &&
								m_materialRenderBatches[j].meshInstances.size() < MaxMeshesPerBatch)
							{
								batch = &m_materialRenderBatches[j];
								break;
							}
						}

						if (batch == nullptr)
						{
							m_materialRenderBatches.push_back(MaterialRenderBatch());
							batch = &m_materialRenderBatches[m_materialRenderBatches.size() - 1];

							batch->material = iter.first;
						}

						int spaceAvailable = MaxMeshesPerBatch - batch->meshInstances.size();
						int itemsLeft = iter.second.meshInstanceCount - instanceOffset;
						int itemsToAdd = Math::Min(spaceAvailable, itemsLeft);

						int startIndex = batch->meshInstances.size();
						batch->meshInstances.resize(batch->meshInstances.size() + itemsToAdd);
						for (int k = 0; k < itemsToAdd; k++)
						{
							batch->meshInstances[startIndex + k] = &iter.second.meshInstances[instanceOffset + k];
						}
						instanceOffset += itemsToAdd;
						totalMeshInstances += itemsToAdd;
					}
				}
			}
		}
	}

	Stat_Rendering_Budgets_BatchesRendered.Set(m_materialRenderBatches.size());
}