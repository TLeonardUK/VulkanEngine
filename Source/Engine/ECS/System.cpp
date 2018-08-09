#include "Pch.h"

#include "Engine/ECS/System.h"
#include "Engine/ECS/World.h"

SystemBase::SystemBase()
{
}

void SystemBase::SetPrimaryAspectId(AspectId aspectId)
{
	m_primaryAspectId = aspectId;
}

Array<std::type_index>& SystemBase::GetRequiredComponents()
{
	return m_componentTypes;
}

Array<std::type_index>& SystemBase::GetSuccessors()
{
	return m_successors;
}

Array<std::type_index>& SystemBase::GetPredecessors()
{
	return m_predecessors;
}

void SystemBase::AddRequiredComponents(Array<std::type_index> indices)
{
	for (std::type_index id : indices)
	{
		if (std::find(m_componentTypes.begin(), m_componentTypes.end(), id) == m_componentTypes.end())
		{
			m_componentTypes.push_back(id);
		}
	}
}

void SystemBase::AddSuccessors(Array<std::type_index> indices)
{
	for (std::type_index id : indices)
	{
		if (std::find(m_successors.begin(), m_successors.end(), id) == m_successors.end())
		{
			m_successors.push_back(id);
		}
	}
}

void SystemBase::AddPredecessors(Array<std::type_index> indices)
{
	for (std::type_index id : indices)
	{
		if (std::find(m_predecessors.begin(), m_predecessors.end(), id) == m_predecessors.end())
		{
			m_predecessors.push_back(id);
		}
	}
}
