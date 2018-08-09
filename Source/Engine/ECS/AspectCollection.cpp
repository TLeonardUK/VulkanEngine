#include "Pch.h"

#include "Engine/ECS/AspectCollection.h"
#include "Engine/ECS/World.h"

AspectCollection::AspectCollection(std::shared_ptr<World> world, std::shared_ptr<Aspect> aspect)
	: m_aspect(aspect)
	, m_world(world)
{
}

std::shared_ptr<Aspect> AspectCollection::GetAspect()
{
	return m_aspect;
}

const Array<Entity>& AspectCollection::GetEntities()
{
	return m_entities;
}

const Dictionary<Entity, int>& AspectCollection::GetEntitiesIndexMap()
{
	return m_entityIndexLookup;
}

bool AspectCollection::TryUpdate(Entity entity, const Dictionary<std::type_index, uint64_t>& componentMap)
{
	std::shared_ptr<World> world = m_world.lock();
	if (world == nullptr)
	{
		assert(false);
		return false;
	}

	if (!m_aspect->MatchesEntity(entity, componentMap))
	{
		TryRemove(entity);
		return false;
	}

	Array<std::type_index> componentTypes = m_aspect->GetComponentTypes();

	auto entityLookupIter = m_entityIndexLookup.find(entity);
	if (entityLookupIter == m_entityIndexLookup.end())
	{
		// Add entity.
		int entityIndex = (int)m_entities.size();

		m_entities.push_back(entity);
		m_entityIndexLookup.emplace(entity, entityIndex);

		for (std::type_index& index : componentTypes)
		{
			ComponentPoolBase* pool = world->GetComponentPool(index);
			void* component = pool->GetIndexPtr(componentMap.find(index)->second);

			Array<void*>& componentList = m_components[index];
			componentList.push_back(component);
		}
	}
	else
	{
		// Update components of entity.
		int entityIndex = entityLookupIter->second;

		for (std::type_index& index : componentTypes)
		{
			ComponentPoolBase* pool = world->GetComponentPool(index);
			void* component = pool->GetIndexPtr(componentMap.find(index)->second);

			Array<void*>& componentList = m_components[index];
			componentList[entityIndex] = component;
		}
	}

	// todo: sort in ascending memory address for spatial locality. Components should be
	// alocallated in same memory address range.

	return true;
}

bool AspectCollection::TryRemove(Entity entity)
{
	Array<std::type_index> componentTypes = m_aspect->GetComponentTypes();

	auto entityLookupIter = m_entityIndexLookup.find(entity);
	if (entityLookupIter != m_entityIndexLookup.end())
	{
		int entityIndex = entityLookupIter->second;

		// Remove entity.
		m_entities[entityIndex] = m_entities[m_entities.size() - 1];
		m_entityIndexLookup[m_entities[entityIndex]] = entityIndex;
		m_entities.resize(m_entities.size() - 1);

		for (std::type_index& index : componentTypes)
		{
			Array<void*>& componentList = m_components[index];

			componentList[entityIndex] = componentList[componentList.size() - 1];
			componentList.resize(componentList.size() - 1);
		}

		return true;
	}

	return false;
}
