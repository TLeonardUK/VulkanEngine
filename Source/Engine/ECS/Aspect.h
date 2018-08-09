#pragma once
#include "Pch.h"

#include "Engine/Types/Array.h"
#include "Engine/ECS/Component.h"

class World;

// Represents an aspect that has been registered with the entity world. Used to lookup
// collections at runtime.
typedef int AspectId;

// An aspect represents a set of component types, if an entity has all the component types, 
// they are defined as having this aspect.
class Aspect
{
private:
	Array<std::type_index> m_componentTypes;
	std::weak_ptr<World> m_world;

public:
	Aspect(std::shared_ptr<World> world, Array<std::type_index> types);

	const Array<std::type_index>& GetComponentTypes();

	bool MatchesEntity(Entity entity, const Dictionary<std::type_index, uint64_t>& componentMap);
	bool EqualTo(const std::shared_ptr<Aspect>& other);

};