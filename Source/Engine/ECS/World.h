#pragma once
#include "Pch.h"

#include "Engine/Engine/FrameTime.h"
#include "Engine/Engine/Logging.h"
#include "Engine/Types/Dictionary.h"
#include "Engine/Types/Array.h"
#include "Engine/Types/Mutex.h"

#include "Engine/ECS/Component.h"
#include "Engine/ECS/ComponentPool.h"
#include "Engine/ECS/MessageQueue.h"
#include "Engine/ECS/System.h"
#include "Engine/ECS/Aspect.h"

class Logger;
class TaskManager;
class AspectCollection;
class SystemBase;

// Message queued when an entity is deleted.
struct DeletedEntityMessage
{
	Entity entity;
};

// Message queued when an entity is created.
struct CreatedEntityMessage
{
	Entity entity;
};

// Message queued when a component is deleted.
struct DeletedComponentMessage
{
	std::type_index componentType = typeid(void);
	Entity entity;
};

// Message queued when a component is created.
struct CreatedComponentMessage
{
	std::type_index componentType = typeid(void);
	Entity entity;
};

// ======================================================================================================================
// A world instance represents the highest level structure in the entity-component-system architecture.
// All entities, systems, components and messages are created, registered and accessed through this struct.
// ======================================================================================================================
// Notes on our ECS:
//
// Entities are a pure-id, that contain no logic or data themselves, but instead have Components associated with them.
// Components are purely data, they contain no logic.
// Aspects are sets of entities that contain a specific set of components.
// Systems are classes that work on a set of entities with a given aspect. 
// Messages are passed between systems to communicate.
//
// Each component should have a single System that is responsible for writing to it, all other systems that wish to modify
// said components should request the System do it via messages.
//
// All systems are ticked in parallel, but can define Predecessor/Successor dependencies to maintain ordering. 
//
// Even though systems are ticked in parallel, this has low-granularity and does not make best use of cpu cores. 
// As such systems should also, where able, try fo further parallelise their ticks. This granularity is left up to 
// systems to determine what is best for the components being worked on.
// ======================================================================================================================
class World 
	: public std::enable_shared_from_this<World>
{
private:
	static const int ComponentPoolBaseIndexOffset = 1000000;

	struct EntityState
	{
		Entity entity;
		Dictionary<std::type_index, uint64_t> components;
	};

	struct PendingComponentRemove
	{
		Entity entity;
		std::type_index componentType;
	};

	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<TaskManager> m_taskManager;

	Dictionary<Entity, EntityState> m_entities;
	Dictionary<std::type_index, ComponentPoolBase*> m_componentPools;
	Dictionary<std::type_index, SystemBase*> m_systems;
	Dictionary<std::type_index, MessageQueueBase*> m_messageQueues;

	Array<std::shared_ptr<AspectCollection>> m_aspectCollections;

	uint64_t m_nextEntityId;

	bool m_tickActive;

	Array<Entity> m_entitiesPendingSystemRegistration;
	Array<Entity> m_entitiesPendingDestroy;
	Array<PendingComponentRemove> m_componentsPendingDestroy;
 
	Mutex m_pendingEntityMessagesMutex;
	Array<DeletedEntityMessage> m_pendingDeletedEntityMessages;
	Array<CreatedEntityMessage> m_pendingCreatedEntityMessages;
	Array<DeletedComponentMessage> m_pendingDeletedComponentMessages;
	Array<CreatedComponentMessage> m_pendingCreatedComponentMessages;

	Mutex m_entityMutex;
	Mutex m_componentPoolMutex;
	Mutex m_aspectCollectionMutex;
	Mutex m_messagesMutex;

private:
	friend class AspectCollection;

	template<typename ComponentType>
	ComponentPool<ComponentType>& GetComponentPool()
	{
		auto iter = m_componentPools.find(typeid(ComponentType));
		if (iter != m_componentPools.end())
		{
			return *static_cast<ComponentPool<ComponentType>*>(iter->second);
		}

		ComponentPool<ComponentType>* pool = new ComponentPool<ComponentType>((m_componentPools.size() + 1) * ComponentPoolBaseIndexOffset);
		m_componentPools.emplace(typeid(ComponentType), pool);

		return *pool;
	}

	template<typename MessageType>
	MessageQueue<MessageType>& GetMessageQueue()
	{
		auto iter = m_messageQueues.find(typeid(MessageType));
		if (iter != m_messageQueues.end())
		{
			return *static_cast<MessageQueue<MessageType>*>(iter->second);
		}

		MessageQueue<MessageType>* queue = new MessageQueue<MessageType>();
		m_messageQueues.emplace(typeid(MessageType), queue);

		return *queue;
	}	

	ComponentPoolBase* GetComponentPool(std::type_index type);
	void* GetComponent(Entity entity, std::type_index type);

	EntityState* GetEntityState(Entity entity);
	void UpdateEntitySystemRegistrations(EntityState& state);

	void RemoveComponent(Entity entity, std::type_index type);
	void* AddComponent(Entity entity, std::type_index type, ComponentPoolBase* pool);

public:
	World(std::shared_ptr<Logger> logger, std::shared_ptr<TaskManager> taskManager);
	~World();

	bool Init();
	void Dispose();

	void Tick(const FrameTime& frameTime);

public:

	// [Thread-Safe] 
	// Creates an entity.
	Entity CreateEntity();

	// [Thread-Safe] 
	// Destroys an existing entity.
	// If called during a tick, will be deferred till end of tick.
	void DestroyEntity(Entity entity);

	// [Thread-Safe] 
	// Determines if the current entity is alive.
	bool IsEntityAlive(Entity entity);

	// [Thread-Safe] 
	// Get an dictionary containing a mapping between component-type and the component-index (in the associated pool) for a given entity.
	bool GetEntityComponentMap(Entity entity, Dictionary<std::type_index, uint64_t>& map);

	// [Thread-Safe] 
	// Gets an aspect-id, for the given aspect. 
	// This can be used with GetAspectCollection to retrieve a list of all entities (and associated components) that share a given aspect.
	AspectId GetAspectId(std::shared_ptr<Aspect> aspect);

	// [Thread-Safe] 
	// Gets an aspect-id, for an aspect represented as a given set of required components.
	// This can be used with GetAspectCollection to retrieve a list of all entities (and associated components) that share a given aspect.
	AspectId GetAspectId(Array<std::type_index> requiredComponents);

	// [Thread-Safe] 
	// Gets a collection of entities that share an aspect defined by the given AspectId.
	std::shared_ptr<AspectCollection> GetAspectCollection(AspectId id);

	// [Thread-Unsafe]
	// Adds a system to the world that executes logic on components that match its criteria.
	template <typename SystemType, typename... Args>
	void AddSystem(Args... args)
	{
		auto iter = m_systems.find(typeid(SystemType));
		if (iter != m_systems.end())
		{
			m_logger->WriteError(LogCategory::Engine, "Attempting to add pre-existing system");
			assert(false);
			return;
		}

		SystemType* type = new SystemType(args...);
		m_systems.emplace(typeid(SystemType), type);

		AspectId id = GetAspectId(std::make_shared<Aspect>(shared_from_this(), type->GetRequiredComponents()));
		type->SetPrimaryAspectId(id);
	}	

	// [Thread-Unsafe]
	// Removes a system that has previously been added. 
	template <typename SystemType>
	void RemoveSystem()
	{
		auto iter = m_systems.find(typeid(SystemType));
		if (iter == m_systems.end())
		{
			m_logger->WriteError(LogCategory::Engine, "Attempting to remove non-existant system");
			assert(false);
			return;
		}

		m_systems.erase(iter);
	}
	// [Thread-Unsafe]
	// Gets a previously registered systme.
	template <typename SystemType>
	SystemType* GetSystem()
	{
		auto iter = m_systems.find(typeid(SystemType));
		if (iter != m_systems.end())
		{
			return static_cast<SystemType*>(iter->second);
		}

		return nullptr;
	}

	// [Thread-Safe]
	// Adds a new component of the given type to Entity and returns it.
	// If called during a tick, will be deferred till end of tick.
	// Entities are not permitted to have multiple of the same component type. 
	template <typename ComponentType>
	ComponentType* AddComponent(Entity entity)
	{
		ComponentPoolBase* pool = &GetComponentPool<ComponentType>();
		return reinterpret_cast<ComponentType*>(AddComponent(entity, typeid(ComponentType), pool));
	}

	// [Thread-Safe]
	// Removes a previously added component of the given type to Entity and returns it.
	// If called during a tick, will be deferred till end of tick.
	template <typename ComponentType>
	void RemoveComponent(Entity entity)
	{
		RemoveComponent(entity, typeid(ComponentType));
	}

	// [Thread-Safe]
	// Gets the component of the given type from an Entity.
	template <typename ComponentType>
	ComponentType* GetComponent(Entity entity)
	{
		return reinterpret_cast<ComponentType*>(GetComponent(entity, typeid(ComponentType)));
	}

	// [Thread-Safe]
	// Queues a new message of the given type, and optionally targets it at the given entity. Systems
	// responsible for these messages will consume them the next time they are ticked. 
	template <typename MessageType>
	void QueueMessage(const MessageType& type)//, Entity entity = NoEntity)
	{
		ScopeLock lock(m_messagesMutex);

		MessageQueue<MessageType>& queue = GetMessageQueue<MessageType>();
		queue.Push(type, NoEntity);// queue.Push(type, entity);
	}

	// [Thread-Safe]
	// Consumes all messages of the given type. 
	// EntityFilter determines if the messages consumed should be those targeted at the given entity, or
	// if not, then messages targeted at no specific entity ("system-wide messages").
	template <typename MessageType>
	Array<MessageType> ConsumeMessages()//Entity entityFilter = NoEntity)
	{
		ScopeLock lock(m_messagesMutex);

		MessageQueue<MessageType>& queue = GetMessageQueue<MessageType>();
		return queue.Pop(NoEntity);// queue.Pop(entityFilter);
	}

	// [Thread-Safe]
	// Same as ConsumeMessages but messages stay in queue.
	template <typename MessageType>
	Array<MessageType> PeekMessages()//Entity entityFilter = NoEntity)
	{
		ScopeLock lock(m_messagesMutex);

		MessageQueue<MessageType>& queue = GetMessageQueue<MessageType>();
		return queue.Peek(NoEntity);// queue.Pop(entityFilter);
	}

};


