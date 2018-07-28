#include "Pch.h"

#include "Engine/Components/Transform/TransformSystem.h"
#include "Engine/Types/Set.h"
#include "Engine/Threading/ParallelFor.h"

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
	for (auto& childRef : transform->children)
	{
		TransformComponent* child = childRef.Get(world);
		UpdateTransform(world, child, transform);
	}
}

void TransformSystem::Tick(
	World& world,
	const FrameTime& frameTime,
	const Array<Entity>& entities,
	const Array<TransformComponent*>& transforms)
{
	// todo: handle transforms being deleted.

	// Consume all relevant messages.
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

	// Find all roots of dirty transforms.
	Set<TransformComponent*> dirtyRoots;
	Array<TransformComponent*> dirtyRootsList;

	for (size_t i = 0; i < entities.size(); i++)
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

			if (dirtyRoots.emplace(lastDirty).second)
			{
				dirtyRootsList.push_back(lastDirty);
			}
		}
	}

	// Update all dirty transforms.
	ParallelFor(static_cast<int>(dirtyRootsList.size()), [&](int i)
	{
		TransformComponent* parent = dirtyRootsList[i]->parent.Get(world);
		UpdateTransform(world, dirtyRootsList[i], parent);
	}, 16, "Transform Update");
}
