#pragma once
#include "Pch.h"

#include "Engine/Types/Dictionary.h"
#include "Engine/Types/Array.h"

class Logger;
class World;

typedef uint64_t Entity;
const uint64_t NoEntity = 0;

template <typename ComponentType>
struct ComponentRef
{
private:
	Entity m_entity;

public:
	ComponentRef()
		: m_entity(NoEntity)
	{
	}

	ComponentRef(Entity entity)
		: m_entity(entity)
	{
	}

	Entity GetEntity()
	{
		return m_entity;
	}

	bool IsValid(World& world)
	{
		return Get(world) != nullptr;
	}

	ComponentType* Get(World& world)
	{
		return world.GetComponent<ComponentType>(m_entity);
	}

	void Set(Entity entity)
	{
		m_entity = entity;
	}
};
