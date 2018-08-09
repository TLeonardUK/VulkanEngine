#include "Pch.h"

#include "Engine/ECS/World.h"
#include "Engine/ECS/AspectCollection.h"
#include "Engine/Threading/TaskManager.h"
#include "Engine/Threading/ParallelFor.h"
#include "Engine/Profiling/Profiling.h"

class SystemTickTask : public Task
{
private:
	FrameTime m_frameTime;
	World* m_world;
	SystemBase* m_system;
	String m_systemName;

public:
	SystemTickTask(const FrameTime& frameTime, World* world, const String& systemName, SystemBase* system)
		: m_system(system)
		, m_world(world)
		, m_frameTime(frameTime)
		, m_systemName(systemName)
	{
	}

	virtual void Run()
	{
		ProfileScope scope(Color::Red, GetName());
		m_system->Tick(*m_world, m_frameTime);
	}

	virtual String GetName()
	{
		return m_systemName + " Tick";
	}
};

World::World(std::shared_ptr<Logger> logger, std::shared_ptr<TaskManager> taskManager)
	: m_logger(logger)
	, m_taskManager(taskManager)
	, m_nextEntityId(1100000) // Arbitrary value, just ensure its greater than 1 (0 = no entity). Higher number tends to make it more obvious when things get corrupted.
	, m_tickActive(false)
{
}

World::~World()
{
}

bool World::Init()
{
	return true;
}

void World::Dispose()
{
	for (auto iter : m_systems)
	{
		delete iter.second;
	}
	for (auto iter : m_componentPools)
	{
		delete iter.second;
	}
	m_systems.clear();
	m_componentPools.clear();
	m_entities.clear();
}

void World::Tick(const FrameTime& frameTime)
{
	ProfileScope scope(Color::Blue, "World::Tick");

	Task::Id worldTickGroupTask;
	Array<Task::Id> tasks;

	{
		ProfileScope scope(Color::Blue, "Pump entity destruction");

		// Perform component destruction.
		for (PendingComponentRemove& removal : m_componentsPendingDestroy)
		{
			RemoveComponent(removal.entity, removal.componentType);
		}
		m_componentsPendingDestroy.clear();

		// Perform object destruction.
		for (Entity& entity : m_entitiesPendingDestroy)
		{
			DestroyEntity(entity);
		}
		m_entitiesPendingDestroy.clear();
	}

	{
		ProfileScope scope(Color::Blue, "Create system tasks");

		// Group task that will complete when all systems are ticked. Gives us something to wait for.
		worldTickGroupTask = m_taskManager->CreateTask();
		tasks.push_back(worldTickGroupTask);

		Dictionary<std::type_index, Task::Id> systemTaskIdMap;

		// Create task to tick each system and insert into task-id map.
		for (auto iter : m_systems)
		{
			Task::Id id = m_taskManager->CreateTask(std::make_shared<SystemTickTask>(frameTime, this, iter.first.name(), iter.second));
			systemTaskIdMap.emplace(iter.first, id);

			m_taskManager->AddDependency(id, worldTickGroupTask);

			tasks.push_back(id);
		}

		// Build dependency tree between each system-tick task.
		for (auto iter : m_systems)
		{
			Array<std::type_index>& predcessors = iter.second->GetPredecessors();
			Array<std::type_index>& successors = iter.second->GetSuccessors();

			for (std::type_index& depType : predcessors)
			{
				m_taskManager->AddDependency(systemTaskIdMap[depType], systemTaskIdMap[iter.first]);
			}

			for (std::type_index& depType : successors)
			{
				m_taskManager->AddDependency(systemTaskIdMap[iter.first], systemTaskIdMap[depType]);
			}
		}
	}

//	auto startTime = std::chrono::high_resolution_clock::now();

	// Dispatch all deleted/created messages to be handled.
	{
		ConsumeMessages<DeletedComponentMessage>();
		ConsumeMessages<DeletedEntityMessage>();
		ConsumeMessages<CreatedComponentMessage>();
		ConsumeMessages<CreatedEntityMessage>();

		for (auto& msg : m_pendingDeletedEntityMessages)
		{
			QueueMessage(msg);
		}
		for (auto& msg : m_pendingCreatedEntityMessages)
		{
			QueueMessage(msg);
		}
		for (auto& msg : m_pendingDeletedComponentMessages)
		{
			QueueMessage(msg);
		}
		for (auto& msg : m_pendingCreatedComponentMessages)
		{
			QueueMessage(msg);
		}

		m_pendingDeletedEntityMessages.clear();
		m_pendingCreatedEntityMessages.clear();
		m_pendingDeletedComponentMessages.clear();
		m_pendingCreatedComponentMessages.clear();
	}

	// Dispatch and wait.
	{
		m_tickActive = true;
		m_taskManager->Dispatch(tasks);
		m_taskManager->WaitForCompletion(worldTickGroupTask);
		m_tickActive = false;
	}

	{
		ProfileScope scope(Color::Blue, "Pump entity registration updates");

		// Perform object registrations.
		for (Entity& entity : m_entitiesPendingSystemRegistration)
		{
			EntityState* state = GetEntityState(entity);
			if (state == nullptr)
			{
				// Destroyed on same frame as creation?
				continue;
			}
			UpdateEntitySystemRegistrations(*state);
		}
		m_entitiesPendingSystemRegistration.clear();
	}

//	float duration = std::chrono::duration<float, std::chrono::milliseconds::period>(std::chrono::high_resolution_clock::now() - startTime).count();
//	m_logger->WriteInfo(LogCategory::Engine, "World Tick Took: %.4fms", duration);
}

Entity World::CreateEntity()
{
	std::lock_guard<std::mutex> lock(m_entityMutex);

	EntityState state;
	state.entity = m_nextEntityId++;
	
	m_entities.emplace(state.entity, state);

	if (m_tickActive)
	{
		m_entitiesPendingSystemRegistration.push_back(state.entity);
	}
	else
	{
		UpdateEntitySystemRegistrations(state);
	}

	// Chuck out an entity-creation message.
	{
		std::lock_guard<std::mutex> lock(m_pendingEntityMessagesMutex);

		CreatedEntityMessage message;
		message.entity = state.entity;
		m_pendingCreatedEntityMessages.push_back(message);
	}

	return state.entity;
}

void World::DestroyEntity(Entity entity)
{
	std::lock_guard<std::mutex> lock(m_entityMutex);

	auto iter = m_entities.find(entity);
	if (iter == m_entities.end())
	{
		return;
	}

	if (m_tickActive)
	{
		m_entitiesPendingDestroy.push_back(entity);
	}
	else
	{
		EntityState& state = iter->second;

		// Erase all entity components.
		for (auto component : state.components)
		{
			// Chunk out an component-deletion message.
			{
				std::lock_guard<std::mutex> lock(m_pendingEntityMessagesMutex);

				DeletedComponentMessage message;
				message.componentType = component.first;
				message.entity = state.entity;

				m_pendingDeletedComponentMessages.push_back(message);
			}

			m_componentPools[component.first]->FreeIndex(component.second);
		}

		// Erase all message queues for this entity.
		for (auto& queueIter : m_messageQueues)
		{
			queueIter.second->RemoveEntity(entity);
		}

		// Deregister from systems.
		for (auto& aspectIter : m_aspectCollections)
		{
			aspectIter->TryRemove(entity);
		}

		// Chunk out an entity-deletion message.
		{
			std::lock_guard<std::mutex> lock(m_pendingEntityMessagesMutex);

			DeletedEntityMessage message;
			message.entity = state.entity;

			m_pendingDeletedEntityMessages.push_back(message);
		}

		m_entities.erase(iter);
	}
}

bool World::IsEntityAlive(Entity entity)
{
	std::lock_guard<std::mutex> lock(m_entityMutex);

	return (m_entities.find(entity) != m_entities.end());
}

void World::UpdateEntitySystemRegistrations(EntityState& state)
{
	for (auto aspectIter : m_aspectCollections)
	{
		aspectIter->TryUpdate(state.entity, state.components);
	}
}

ComponentPoolBase* World::GetComponentPool(std::type_index type)
{
	std::lock_guard<std::mutex> lock(m_componentPoolMutex);

	auto iter = m_componentPools.find(type);
	if (iter != m_componentPools.end())
	{
		return iter->second;
	}

	return nullptr;
}

void* World::GetComponent(Entity entity, std::type_index type)
{
	std::lock_guard<std::mutex> lock(m_entityMutex);

	ComponentPoolBase* pool = GetComponentPool(type);

	EntityState* state = GetEntityState(entity);
	if (state == nullptr)
	{
		return false;
	}

	auto componentIter = state->components.find(type);
	if (componentIter == state->components.end())
	{
		return nullptr;
	}

	return pool->GetIndexPtr(componentIter->second);
}

bool World::GetEntityComponentMap(Entity entity, Dictionary<std::type_index, uint64_t>& map)
{
	std::lock_guard<std::mutex> lock(m_entityMutex);

	EntityState* state = GetEntityState(entity);
	if (state == nullptr)
	{
		return false;
	}

	map = state->components;

	return true;
}

AspectId World::GetAspectId(std::shared_ptr<Aspect> aspect)
{
	std::lock_guard<std::mutex> lock(m_aspectCollectionMutex);

	for (int i = 0; i < m_aspectCollections.size(); i++)
	{
		if (m_aspectCollections[i]->GetAspect()->EqualTo(aspect))
		{
			return static_cast<AspectId>(i);
		}
	}

	m_aspectCollections.push_back(std::make_shared<AspectCollection>(shared_from_this(), aspect));
	return static_cast<AspectId>(m_aspectCollections.size() - 1);
}

AspectId World::GetAspectId(Array<std::type_index> requiredComponents)
{
	return GetAspectId(std::make_shared<Aspect>(shared_from_this(), requiredComponents));
}

std::shared_ptr<AspectCollection> World::GetAspectCollection(AspectId id)
{
	std::lock_guard<std::mutex> lock(m_aspectCollectionMutex);

	return m_aspectCollections[static_cast<int>(id)];
}

World::EntityState* World::GetEntityState(Entity entity)
{
	auto iter = m_entities.find(entity);
	if (iter == m_entities.end())
	{
		return nullptr;
	}

	return &iter->second;
}

void World::RemoveComponent(Entity entity, std::type_index type)
{
	std::lock_guard<std::mutex> lock(m_entityMutex);

	ComponentPoolBase* pool = GetComponentPool(type);

	auto iter = m_entities.find(entity);
	if (iter == m_entities.end())
	{
		m_logger->WriteError(LogCategory::Engine, "Attempting to remove component from non-existant entity.");
		assert(false);
		return;
	}

	if (m_tickActive)
	{
		PendingComponentRemove remove = { entity, type };
		m_componentsPendingDestroy.push_back(remove);
	}
	else
	{
		EntityState& state = iter->second;
		auto componentIter = state.components.find(type);
		if (componentIter == state.components.end())
		{
			m_logger->WriteError(LogCategory::Engine, "Attempting to remove non-existant component from entity.");
			assert(false);
			return;
		}

		state.components.erase(componentIter);

		UpdateEntitySystemRegistrations(state);
	
		// Chunk out an component-deletion message.
		{
			std::lock_guard<std::mutex> lock(m_pendingEntityMessagesMutex);

			DeletedComponentMessage message;
			message.componentType = type;
			message.entity = entity;
			m_pendingDeletedComponentMessages.push_back(message);
		}

		pool->FreeIndex(componentIter->second);
	}
}

void* World::AddComponent(Entity entity, std::type_index type, ComponentPoolBase* pool)
{
	std::lock_guard<std::mutex> lock(m_entityMutex);

	uint64_t index = pool->AllocateIndex();

	auto iter = m_entities.find(entity);
	if (iter == m_entities.end())
	{
		m_logger->WriteError(LogCategory::Engine, "Attempting to add component to non-existant entity.");
		assert(false);
		return nullptr;
	}

	EntityState& state = iter->second;
	auto componentIter = state.components.find(type);
	if (componentIter != state.components.end())
	{
		m_logger->WriteError(LogCategory::Engine, "Attempting to add duplicate component to entity.");
		assert(false);
		return nullptr;
	}

	state.components.emplace(type, index);

	if (m_tickActive)
	{
		m_entitiesPendingSystemRegistration.push_back(state.entity);
	}
	else
	{
		UpdateEntitySystemRegistrations(state);

		// Chunk out an component-creation message.
		{
			std::lock_guard<std::mutex> lock(m_pendingEntityMessagesMutex);

			CreatedComponentMessage message;
			message.componentType = type;
			message.entity = entity;
			m_pendingCreatedComponentMessages.push_back(message);
		}
	}

	return pool->GetIndexPtr(index);
}