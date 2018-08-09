#pragma once
#include "Pch.h"

#include "Engine/Types/Dictionary.h"
#include "Engine/Types/Array.h"

#include "Engine/ECS/Component.h"

class Logger;
class World;

class MessageQueueBase
{
public:
	virtual ~MessageQueueBase()
	{
	}

	virtual void RemoveEntity(Entity entity) = 0;

};

template<typename MessageType>
class MessageQueue : public MessageQueueBase
{
public:
	struct Queue
	{
		Array<MessageType> messages;
	};

	Dictionary<Entity, Queue*> m_queues;

private:
	Queue& GetOrCreateQueue(Entity entity)
	{
		auto iter = m_queues.find(entity);
		if (iter != m_queues.end())
		{
			return *iter->second;
		}

		Queue* queue = new Queue();
		m_queues.emplace(entity, queue);

		return *queue;
	}

public:
	MessageQueue()
	{
	}

	virtual ~MessageQueue()
	{
		for (std::pair<Entity, Queue*> pair : m_queues)
		{
			delete pair.second;
		}
	}

	void Push(const MessageType& type, Entity entity)
	{
		Queue& queue = GetOrCreateQueue(entity);
		queue.messages.push_back(type);
	}
	
	virtual void RemoveEntity(Entity entity)
	{
		auto iter = m_queues.find(entity);
		if (iter != m_queues.end())
		{
			m_queues.erase(iter);
		}
	}

	Array<MessageType> Pop(Entity entity)
	{
		Array<MessageType> result;
		
		Queue& queue = GetOrCreateQueue(entity);
		result = queue.messages;
		queue.messages.clear();

		return result;
	}

	Array<MessageType> Peek(Entity entity)
	{
		Array<MessageType> result;

		Queue& queue = GetOrCreateQueue(entity);
		result = queue.messages;

		return result;
	}
};

