#include "Pch.h"

#include "Engine/Systems/Transform/TransformSystem.h"
#include "Engine/Threading/ParallelFor.h"
#include "Engine/Threading/TaskManager.h"
#include "Engine/Types/Math.h"

#include "Engine/Profiling/Profiling.h"

TransformSystem::TransformSystem()
{
}

void TransformSystem::UpdateTransform(World& world, TransformComponent* transform, TransformComponent* parentTransform)
{
	Matrix4 scaleMatrix = Matrix4::Scale(transform->localScale);
	Matrix4 rotationMatrix = Matrix4::Rotation(transform->localRotation);
	Matrix4 translateMatrix = Matrix4::Translate(transform->localPosition);

	transform->localToWorld = translateMatrix * rotationMatrix * scaleMatrix;

	if (parentTransform != nullptr)
	{
		transform->localToWorld = transform->localToWorld * parentTransform->localToWorld;
	}

	transform->worldToLocal = transform->localToWorld.Inverse();
	transform->isDirty = false;
	transform->version++;

	// Update children.
	ParallelFor(static_cast<int>(transform->children.size()), [&](int i)
	{
		TransformComponent* child = transform->children[i].Get(world);
		UpdateTransform(world, child, transform);
	}, 16, "Child Transform Update");
}

void TransformSystem::Tick(
	World& world,
	const FrameTime& frameTime,
	const Array<Entity>& entities,
	const Array<TransformComponent*>& transforms)
{
	// todo: handle transforms being deleted.
	// todo: build dirty list from messages / add / remove events. Rather than iterating all entities.

	// Consume all relevant messages.
	{
		ProfileScope scope(Color::Black, "Consume Messages");

		for (auto& message : world.ConsumeMessages<SetTransformMessage>())
		{
			TransformComponent* component = message.component.Get(world);
			if (component != nullptr)
			{
				component->localPosition = message.localPosition;
				component->localRotation = message.localRotation;
				component->localScale = message.localScale;
				component->isDirty = true;
			}
		}
		for (auto& message : world.ConsumeMessages<SetTransformParentMessage>())
		{
			TransformComponent* component = message.component.Get(world);
			if (component != nullptr)
			{
				TransformComponent* parent = message.parent.Get(world);
				parent->children.push_back(message.component);

				component->parent = message.parent;
				component->isDirty = true;
			}
		}
	}

	// Find all roots of dirty transforms.
	int splitFactor = TaskManager::AsyncInstance->GetConcurrency() * 4;
	int chunkSize = (int)Math::Ceil(entities.size() / (float)splitFactor);

	m_asyncDirtyRoots.resize(splitFactor);
	m_asyncDirtyRootsList.resize(splitFactor);
	
	{
		ProfileScope scope(Color::Black, "Find Dirty Roots");

		ParallelFor(splitFactor, [&](int split)
		{
			int startIndex = (split * chunkSize);
			int endIndex = Math::Min((int)entities.size(), startIndex + chunkSize);

			m_asyncDirtyRoots[split].clear();
			m_asyncDirtyRootsList[split].clear();

			for (size_t i = startIndex; i < endIndex; i++)
			{
				TransformComponent* transform = transforms[i];
				Entity entity = entities[i];

				if (transform->isDirty)
				{
					TransformComponent* lastDirty = transform;

					while (transform != nullptr)
					{
						if (transform->isDirty)
						{
							lastDirty = transform;
						}
						transform = transform->parent.Get(world);
					}

					if (m_asyncDirtyRoots[split].emplace(lastDirty).second)
					{
						m_asyncDirtyRootsList[split].push_back(lastDirty);
					}
				}
			}
		}, 1, "Find Dirty Roots Task");
	}

	// Combine async lists.	
	{
		ProfileScope scope(Color::Black, "Combine Dirty Roots List");

		for (int i = 1; i < splitFactor; i++)
		{
			Array<TransformComponent*>& splitList = m_asyncDirtyRootsList[i];

			for (TransformComponent* component : splitList)
			{
				if (m_asyncDirtyRoots[0].emplace(component).second)
				{
					m_asyncDirtyRootsList[0].push_back(component);
				}
			}
		}
	}

	// Update all dirty transforms.
	{
		ProfileScope scope(Color::Black, "Update Dirty Roots");

		ParallelFor(static_cast<int>(m_asyncDirtyRootsList[0].size()), [&](int i)
		{
			TransformComponent* parent = m_asyncDirtyRootsList[0][i]->parent.Get(world);
			UpdateTransform(world, m_asyncDirtyRootsList[0][i], parent);
		}, 16, "Root Transform Update");
	}
}
