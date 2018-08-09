#pragma once
#include "Pch.h"

#include "Engine/Engine/FrameTime.h"
#include "Engine/Types/Dictionary.h"
#include "Engine/Types/Array.h"

#include "Engine/ECS/Component.h"
#include "Engine/ECS/Aspect.h"
#include "Engine/ECS/AspectCollection.h"
#include "Engine/ECS/World.h"

class World;

class SystemBase
{
protected:
	Array<std::type_index> m_componentTypes;
	Array<std::type_index> m_successors;
	Array<std::type_index> m_predecessors;

	AspectId m_primaryAspectId;

private:
	friend class SystemTickTask;
	friend class World;

	virtual void Tick(World& world, const FrameTime& frameTime) = 0;

public:
	SystemBase();

protected:
	void SetPrimaryAspectId(AspectId aspectId);

	Array<std::type_index>& GetRequiredComponents();
	void AddRequiredComponents(Array<std::type_index> indices);

	Array<std::type_index>& GetSuccessors();
	Array<std::type_index>& GetPredecessors();
	void AddSuccessors(Array<std::type_index> indices);
	void AddPredecessors(Array<std::type_index> indices);

	template<typename ComponentType>
	void AddRequiredComponent()
	{
		AddRequiredComponents({ typeid(ComponentType) });
	}
	
	template<typename DependentType>
	void AddPredecessor()
	{
		AddPredecessors({ typeid(DependentType) });
	}

	template<typename DependentType>
	void AddSuccessor()
	{
		AddSuccessors({ typeid(DependentType) });
	}
};

template<typename ...Args>
class System 
	: public SystemBase
{
public:
	System()
		: SystemBase()
	{
		Array<std::type_index> componentTypes = { typeid(Args)... };
		AddRequiredComponents(componentTypes);
	}

	virtual ~System()
	{
	}
	 
private:
	virtual void Tick(World& world, const FrameTime& frameTime)
	{
		std::shared_ptr<AspectCollection> collection = world.GetAspectCollection(m_primaryAspectId);
		Tick(world, frameTime, collection->GetEntities(), collection->GetEntityComponents<Args>()...);
	}

public:
	virtual void Tick(World& world, const FrameTime& frameTime, const Array<Entity>& entities, const Array<Args*>&...) = 0;

};

