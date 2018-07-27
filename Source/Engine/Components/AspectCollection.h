#pragma once
#include "Pch.h"

#include "Engine/Types/Array.h"
#include "Engine/Components/Component.h"
#include "Engine/Components/Aspect.h"

class World;

// Maintains a collection of entities that have a given aspect. Maintains internal lists
// of each entities components that are relevant to the aspect for performant access.
class AspectCollection
{
private:
	std::shared_ptr<Aspect> m_aspect;

	Array<Entity> m_entities;
	Dictionary<Entity, int> m_entityIndexLookup;
	Dictionary<std::type_index, Array<void*>> m_components;

	std::weak_ptr<World> m_world;

public:
	AspectCollection(std::shared_ptr<World> world, std::shared_ptr<Aspect> aspect);

	template <typename ComponentType>
	const Array<ComponentType*>& GetEntityComponents()
	{
		static Array<ComponentType*> defaultValue;

		auto iter = m_components.find(typeid(ComponentType));
		if (iter != m_components.end())
		{
			return *reinterpret_cast<Array<ComponentType*>*>(&iter->second);
		}

		return defaultValue;
	}

	const Array<Entity>& GetEntities();

	std::shared_ptr<Aspect> GetAspect();

	bool TryUpdate(Entity entity, const Dictionary<std::type_index, uint64_t>& componentMap);
	bool TryRemove(Entity entity);

};