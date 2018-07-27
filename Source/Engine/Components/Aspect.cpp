#include "Pch.h"

#include "Engine/Components/AspectCollection.h"
#include "Engine/Components/World.h"

Aspect::Aspect(std::shared_ptr<World> world, Array<std::type_index> types)
	: m_componentTypes(types)
	, m_world(world)
{
}

const Array<std::type_index>& Aspect::GetComponentTypes()
{
	return m_componentTypes;
}

bool Aspect::MatchesEntity(Entity entity, const Dictionary<std::type_index, uint64_t>& componentMap)
{
	std::shared_ptr<World> world = m_world.lock();
	if (world == nullptr)
	{
		assert(false);
		return false;
	}

	for (std::type_index& index : m_componentTypes)
	{
		auto iter = componentMap.find(index);
		if (iter == componentMap.end())
		{
			return false;
		}
	}
	
	return true;
}

bool Aspect::EqualTo(const std::shared_ptr<Aspect>& other)
{
	if (m_componentTypes.size() != other->m_componentTypes.size())
	{
		return false;
	}

	for (int i = 0; i < m_componentTypes.size(); i++)
	{
		if (m_componentTypes[i] != other->m_componentTypes[i])
		{
			return false;
		}
	}

	return true;
}