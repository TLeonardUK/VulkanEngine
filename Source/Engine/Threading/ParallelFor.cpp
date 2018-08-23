#include "Pch.h"

#include "Engine/Threading/ParallelFor.h"
#include "Engine/Threading/TaskManager.h"
#include "Engine/Threading/Event.h"

#include "Engine/Profiling/Profiling.h"

#include "Engine/Types/Math.h"

struct ParallelForData
{
	ParallelForSignature_t function;

	String name;

	int granularity;

	int count;
	std::atomic<int> index;
	std::atomic<int> completedCount;

	Event completedEvent;
};

class ParallelForTask : public Task
{
private:
	std::shared_ptr<ParallelForData> m_data;

public:
	ParallelForTask(std::shared_ptr<ParallelForData> data)
		: m_data(data)
	{
	}

	virtual void Run()
	{
		ProfileScope scope(ProfileColors::SecondaryTask, GetName());

		while (true)
		{
			int startIndex = m_data->index.fetch_add(m_data->granularity);
			if (startIndex >= m_data->count)
			{
				break;
			}

			int endIndex = startIndex + m_data->granularity;
			if (endIndex >= m_data->count)
			{
				endIndex = m_data->count;
			}

			int total = endIndex - startIndex;
			for (int i = startIndex; i < endIndex; i++)
			{
				m_data->function(i);
			}

			int totalCompleted = m_data->completedCount.fetch_add(total) + total;
			if (totalCompleted == m_data->count)
			{
				m_data->completedEvent.Signal();
			}
		}
	}

	virtual String GetName()
	{
		return m_data->name;
	}
};

void ParallelFor(int count, ParallelForSignature_t function, int granularity, const String& name)
{
	assert(count >= 0);

	if (count == 0)
	{
		return;
	}

	// If lower than granularity, just do the whole thing single-threaded, less overhead
	// and more cache coherent, for small loops.
	if (count < granularity)
	{
		for (int i = 0; i < count; i++)
		{
			function(i);
		}
		return;
	}

	ProfileScope scope(ProfileColors::SecondaryTask, "Parallel For");

	TaskManager* manager = TaskManager::AsyncInstance;
	int concurrency = manager->GetConcurrency();

	// todo: grab data in chunks of granularity, rather than individually, more cache coherent.

	std::shared_ptr<ParallelForData> data = std::make_shared<ParallelForData>();
	data->granularity = (count < granularity * concurrency) ? (int)Math::Ceil(count / (float)concurrency) : granularity;
	data->count = count;
	data->index = 0;
	data->completedCount = 0;
	data->completedEvent.Reset();
	data->function = function;
	data->name = name;

	Array<Task::Id> tasks;
	tasks.resize(concurrency);

	for (int i = 0; i < concurrency; i++)
	{
		tasks[i] = manager->CreateTask(std::make_shared<ParallelForTask>(data));
	}

	manager->Dispatch(tasks);
	manager->WaitForCompletionEvent(data->completedEvent);
}