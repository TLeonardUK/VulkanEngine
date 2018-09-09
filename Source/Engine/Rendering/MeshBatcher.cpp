#include "Pch.h"

#include "Engine/Components/Mesh/MeshComponent.h"
#include "Engine/Components/Transform/TransformComponent.h"

#include "Engine/Rendering/MeshBatcher.h"
#include "Engine/Rendering/Renderer.h"
#include "Engine/Rendering/RenderPropertyHeirarchy.h"

#include "Engine/Profiling/Profiling.h"
#include "Engine/Threading/ParallelFor.h"
#include "Engine/Threading/TaskManager.h"

#include "Engine/Utilities/Statistic.h"

#include "Engine/Types/Math.h"

extern Statistic Stat_Rendering_Budgets_MeshesRendered;
extern Statistic Stat_Rendering_Budgets_BatchesRendered;
extern Statistic Stat_Rendering_Budgets_TrianglesRendered;

extern const RenderPropertyHash ModelMatrixHash;

MeshBatcher::~MeshBatcher()
{
	for (MaterialRenderBatch* batch : m_materialRenderBatches)
	{
		delete batch;
	}
	m_materialRenderBatches.clear();
}

Array<MaterialRenderBatch*>& MeshBatcher::GetBatches()
{
	return m_finalRenderBatches;
}

void MeshBatcher::Batch(
	World& world,
	const std::shared_ptr<Renderer>& renderer,
	const std::shared_ptr<Logger>& logger,
	const std::shared_ptr<IGraphics>& graphics,
	const Array<Entity>& entities, 
	MaterialVariant variant,
	RenderPropertyCollection* viewProperties,
	RenderFlags requiredFlags,
	RenderFlags excludedFlags)
{
	ProfileScope scope(ProfileColors::Draw, "Renderer::BuildViewCommandBuffer_Meshes");

	if (m_meshComponentAspectId == -1)
	{
		m_meshComponentAspectId = world.GetAspectId({ typeid(TransformComponent), typeid(MeshComponent) });
	}

	std::shared_ptr<AspectCollection> collection = world.GetAspectCollection(m_meshComponentAspectId);
	const Dictionary<Entity, int>& entityIndexMap = collection->GetEntitiesIndexMap();
	const Array<MeshComponent*>& meshComponents = collection->GetEntityComponents<MeshComponent>();
	const Array<TransformComponent*>& meshTransformComponents = collection->GetEntityComponents<TransformComponent>();

	// Reset stack allocator.
	m_meshInstanceAllocator.Reset(entities.size());

	// Batch meshes by material.
	int splitFactor = TaskManager::AsyncInstance->GetConcurrency() * 4;
	int chunkSize = (int)Math::Ceil(entities.size() / (float)splitFactor);

	// Resize the batch lists so they have enough space for each material.
	{
		ProfileScope scope(ProfileColors::Draw, "Resize Batch Lists");

		m_asyncMaterialBatches.resize(splitFactor);
		for (MaterialRenderBatch* batch : m_materialRenderBatches)
		{
			batch->firstMesh = nullptr;
			batch->meshInstanceCount = 0;
			batch->inUse = false;
		}
		m_finalRenderBatches.clear();
	}

	// Batch all meshes in parallel.
	{
		ProfileScope scope(ProfileColors::Draw, "Batch Meshes");

		ParallelFor(splitFactor, [&](int index)
		{
			Dictionary<Material*, MaterialBatch>& batches = m_asyncMaterialBatches[index];

			for (auto& iter : batches)
			{
				iter.second.firstMesh = nullptr;
				iter.second.meshInstanceCount = 0;
			}

			int startIndex = (index * chunkSize);
			int endIndex = Math::Min((int)entities.size(), startIndex + chunkSize);

			for (int i = startIndex; i < endIndex; i++)
			{
				//ProfileScope scope(ProfileColors::Draw, "Mesh");
				Entity entity = entities[i];

				auto iter = entityIndexMap.find(entity);
				if (iter == entityIndexMap.end())
				{
					continue;
				}

				int entityArrayIndex = iter->second;
				MeshComponent* meshComponent = meshComponents[entityArrayIndex];
				TransformComponent* transformComponent = meshTransformComponents[entityArrayIndex];

				// Filter based on render flags if required.
				if (requiredFlags != RenderFlags::None || excludedFlags != RenderFlags::None)
				{
					std::shared_ptr<Material> baseMaterial = meshComponent->mesh->GetMaterial().Get();
					std::shared_ptr<Shader> baseShader = baseMaterial->GetShader().Get();

					RenderFlags flags = baseShader->GetProperties().Flags;

					if (requiredFlags != RenderFlags::None)
					{
						if ((static_cast<int>(flags) & static_cast<int>(requiredFlags)) != static_cast<int>(requiredFlags))
						{
							continue;
						}
					}
					if (excludedFlags != RenderFlags::None)
					{
						if ((static_cast<int>(flags) & static_cast<int>(requiredFlags)) != 0)
						{
							continue;
						}
					}
				}

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
					batch->firstMesh = nullptr;
				}

				std::shared_ptr<MeshRenderState>& renderData = meshComponent->renderData[(int)variant];

				// Update mesh state.
				//{
				//	ProfileScope scope(ProfileColors::Draw, "UpdateResources");
				meshComponent->mesh->UpdateResources();
				//}

				// Create new instance.
				MeshInstance* instance = m_meshInstanceAllocator.New();
				instance->indexBuffer = &meshComponent->mesh->GetIndexBuffer();
				instance->vertexBuffer = &meshComponent->mesh->GetVertexBuffer();
				instance->indexCount = meshComponent->mesh->GetIndexCount();

				instance->nextMesh = batch->firstMesh;
				batch->firstMesh = instance;
				batch->meshInstanceCount++;

				//{
				//	ProfileScope scope(ProfileColors::Draw, "UpdateMeshRenderState");

				// todo: this needs to go elsewhere, but where?
				material->GetProperties().UpdateResources(graphics, logger);

				// Create render property heirarchy.
				RenderPropertyHeirarchy renderHeirarchy;
				renderHeirarchy.Set(GraphicsBindingFrequency::Global, &renderer->GetGlobalRenderProperties());
				renderHeirarchy.Set(GraphicsBindingFrequency::View, viewProperties);
				renderHeirarchy.Set(GraphicsBindingFrequency::Material, &material->GetProperties());
				renderHeirarchy.Set(GraphicsBindingFrequency::Mesh, &meshComponent->properties);

				if (renderData == nullptr)
				{
					renderer->CreateMeshRenderState(&renderData);
				}
				instance->resourceSets = &renderData->UpdateAndGetResourceSets(material, &renderHeirarchy);
				//}

				//{
				//	ProfileScope scope(ProfileColors::Draw, "InstanceData");
				Stat_Rendering_Budgets_MeshesRendered.Add(1);
				Stat_Rendering_Budgets_TrianglesRendered.Add(instance->indexCount / 3);
				//}
			}
		}, 1, "Mesh Batching");
	}

	// Make list of all batch types available.
	// todo: should be able to do this in parallel?
	int totalMeshInstances = 0;
	{
		ProfileScope scope(ProfileColors::Draw, "Combine Batches");

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
							if (m_materialRenderBatches[j]->material == iter.first &&
								m_materialRenderBatches[j]->meshInstanceCount < MaxMeshesPerBatch)
							{
								batch = m_materialRenderBatches[j];
								break;
							}
						}

						if (batch == nullptr)
						{
							m_materialRenderBatches.push_back(new MaterialRenderBatch());
							batch = m_materialRenderBatches[m_materialRenderBatches.size() - 1];

							batch->material = iter.first;
						}

						if (batch->inUse == false)
						{
							m_finalRenderBatches.push_back(batch);
							batch->inUse = true;
						}

						int spaceAvailable = MaxMeshesPerBatch - (int)batch->meshInstanceCount;
						int itemsLeft = iter.second.meshInstanceCount - instanceOffset;
						int itemsToAdd = Math::Min(spaceAvailable, itemsLeft);

						for (int k = 0; k < itemsToAdd; k++)
						{
							MeshInstance* tmpNext = iter.second.firstMesh->nextMesh;

							iter.second.firstMesh->nextMesh = batch->firstMesh;
							batch->firstMesh = iter.second.firstMesh;
							iter.second.firstMesh = tmpNext;

							batch->meshInstanceCount++;
						}
						instanceOffset += itemsToAdd;
						totalMeshInstances += itemsToAdd;
					}
				}
			}
		}
	}

	/*
	size_t capacity = 0;
	size_t size = 0;
	for (auto& asyncBatches : m_asyncMaterialBatches)
	{
		for (auto& batchIter : asyncBatches)
		{
			capacity += batchIter.second.meshInstances.capacity();
			size += batchIter.second.meshInstances.size();
		}
	}
	
	if (size > 16)
	{
		printf("===================================\n");
		printf("MeshInstance = %i\n", sizeof(MeshInstance));

		printf("Async: capacity=%i size=%i mb=%.2f\n", capacity, size, ((capacity * sizeof(MeshInstance)) / 1024.0f) / 1024.0f);

		for (auto& asyncBatches : m_asyncMaterialBatches)
		{
			printf("Async Material Batch: Count=%i\n", asyncBatches.size());
		}

		capacity = 0;
		size = 0;
		for (MaterialRenderBatch* batch : m_materialRenderBatches)
		{
			capacity += batch->meshInstances.capacity();
			size += batch->meshInstances.size();
		}
		printf("Batches: capacity=%i size=%i\n", capacity, size);
	}*/

	Stat_Rendering_Budgets_BatchesRendered.Add((double)m_finalRenderBatches.size());
}