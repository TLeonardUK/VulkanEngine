#include "Pch.h"

#include "Engine/Threading/TaskManager.h"
#include "Engine/Profiling/Profiling.h"
#include "Engine/Engine/Logging.h"

TaskManager* TaskManager::AsyncInstance = nullptr;

TaskManager::TaskManager(std::shared_ptr<Logger> logger)
	: m_active(false)
	, m_logger(logger)
	, m_nextTaskIndex(0)
{
}

TaskManager::~TaskManager()
{
	Dispose();
}

bool TaskManager::Init(Dictionary<TaskQueueFlags, int> queueWorkerCounts)
{
	m_active = true;
	m_queuedTaskCount.store(0);
	m_pendingTasks.store(0);

	for (int i = 0; i < MaxQueuedTasks; i++)
	{
		m_queuedTasks[i].available = true;
	}

	for (auto pair : queueWorkerCounts)
	{
		for (int i = 0; i < pair.second; i++)
		{
			m_workers.push_back(std::thread(&TaskManager::WorkerLoop, this, pair.first));
		}
	}

	return true;
}

void TaskManager::Dispose()
{
	m_active = false;

	// Wake all threads up so they exit.
	{
		std::unique_lock<std::mutex> lock(m_taskAvailableLock);
		m_taskAvailableCondVariable.notify_all();
	}

	for (std::thread& thread : m_workers)
	{
		thread.join();
	}
}

Task::Id TaskManager::CreateTask(Task::Ptr task)
{
	Task::Id id = AllocTaskId();

	QueuedTask* queuedTask;
	if (!GetTaskFromId(id, queuedTask))
	{
		assert(false);
	}

	queuedTask->task = task;

	return id;
}

void TaskManager::AddDependency(Task::Id predecessor, Task::Id successor)
{
	QueuedTask* successorTask;
	QueuedTask* predecessorTask;

	if (!GetTaskFromId(successor, successorTask))
	{
		assert(false);
	}
	if (!GetTaskFromId(predecessor, predecessorTask))
	{
		assert(false);
	}

	assert(!successorTask->executing);
	assert(!predecessorTask->executing);

	successorTask->predecessors.push_back(predecessor);
	predecessorTask->hasSuccessors = true;

	// todo: ensure predecessors are ahead of task in queue.
}

void TaskManager::Dispatch(Task::Id task, TaskQueueFlags queues)
{
	std::unique_lock<std::mutex> lock(m_taskAvailableLock);

	if ((static_cast<int>(queues) & static_cast<int>(TaskQueueFlags::Normal)) != 0)
	{
		Dispatch(task, m_taskQueues[static_cast<int>(TaskQueueIndex::Normal)]);
	}
	if ((static_cast<int>(queues) & static_cast<int>(TaskQueueFlags::Long)) != 0)
	{
		Dispatch(task, m_taskQueues[static_cast<int>(TaskQueueIndex::Long)]);
	}
	if ((static_cast<int>(queues) & static_cast<int>(TaskQueueFlags::TimeCritical)) != 0)
	{
		Dispatch(task, m_taskQueues[static_cast<int>(TaskQueueIndex::TimeCritical)]);
	}

	// Wake all threads up to consider tasks for execution.
	m_pendingTasks++;
	m_queuedTaskCount++;
	m_taskAvailableCondVariable.notify_all();
}

void TaskManager::Dispatch(Array<Task::Id> tasks, TaskQueueFlags queues)
{
	for (Task::Id& id : tasks)
	{
		Dispatch(id, queues);
	}
}

void TaskManager::Dispatch(Task::Id task, TaskQueue& queue)
{
	QueuedTask* queuedTask;
	if (!GetTaskFromId(task, queuedTask))
	{
		assert(false);
	}

	queuedTask->executing = false;

	queue.pendingTasks.push_back(task);
}

bool TaskManager::IsIdle()
{
	return m_queuedTaskCount.load() == 0;
}

int TaskManager::GetConcurrency()
{
	return static_cast<int>(m_workers.size() + 1);
}

bool TaskManager::IsComplete(Task::Id task)
{
	QueuedTask* queuedTask;
	if (!GetTaskFromId(task, queuedTask))
	{
		assert(false);
	}

	return queuedTask == nullptr || queuedTask->available;
}

bool TaskManager::WaitForCompletionEvent(Event& evnt, Timeout timeout)
{
	//ProfileScope scope(ProfileColors::Cpu, "Waiting for completion event");

	TimeoutCounter counter = timeout.Start();

	while (!evnt.IsSignaled())
	{
		if (counter.HasFinished())
		{
			return false;
		}

		if (!RunTask(TaskQueueFlags::All, false))
		{
			std::unique_lock<std::mutex> lock(m_taskAvailableLock);

			auto timeoutRemaining = std::chrono::milliseconds((int)counter.GetRemaining());
			m_taskAvailableCondVariable.wait_for(lock, timeoutRemaining);
		}
	}

	return true;
}

bool TaskManager::WaitForCompletion(Task::Id task, Timeout timeout)
{
	//ProfileScope scope(ProfileColors::Cpu, "Waiting for task completion");

	TimeoutCounter counter = timeout.Start();

	while (!IsComplete(task))
	{
		if (counter.HasFinished())
		{
			return false;
		}

		if (!RunTask(TaskQueueFlags::All, false))
		{
			std::unique_lock<std::mutex> lock(m_taskAvailableLock);

			if (IsComplete(task))
			{
				break;
			}
			else
			{
				auto timeoutRemaining = std::chrono::milliseconds((int)counter.GetRemaining());
				m_taskAvailableCondVariable.wait_for(lock, timeoutRemaining);
			}
		}
	}

	return true;
}

bool TaskManager::WaitForIdle(Timeout timeout)
{
	//ProfileScope scope(ProfileColors::Cpu, "Waiting for idle");

	TimeoutCounter counter = timeout.Start();

	while (!IsIdle())
	{
		if (counter.HasFinished())
		{
			return false;
		}

		if (!RunTask(TaskQueueFlags::All, false))
		{
			std::unique_lock<std::mutex> lock(m_taskAvailableLock);

			if (IsIdle())
			{
				break;
			}
			else
			{
				auto timeoutRemaining = std::chrono::milliseconds((int)counter.GetRemaining());
				m_taskAvailableCondVariable.wait_for(lock, timeoutRemaining);
			}
		}
	}

	return true;
}

Task::Id TaskManager::AllocTaskId()
{
	for (int i = 0; i < MaxQueuedTasks; i++)
	{
		int nextIndex = (m_nextTaskIndex++);
		int realIndex = nextIndex % MaxQueuedTasks;

		QueuedTask& task = m_queuedTasks[realIndex];
		if (task.available)
		{
			bool expectedValue = true;
			if (task.available.compare_exchange_strong(expectedValue, false) == true)
			{
				task.id = nextIndex;
				task.executing = false;
				task.predecessors.clear();
				task.hasSuccessors = false;
				task.task = nullptr;
				return task.id;
			}
		}
	}	

	assert(false); // Out of task-indices.
	return -1;
}

void TaskManager::FreeTaskId(Task::Id id)
{	
	QueuedTask* queuedTask = nullptr;
	if (!GetTaskFromId(id, queuedTask))
	{
		// We should only try to free task-id's that are valid!
		assert(false);
	}

	queuedTask->available = true;
	queuedTask->task = nullptr;
}

bool TaskManager::GetTaskFromId(Task::Id id, QueuedTask*& task)
{
	int index = static_cast<int>(id) % MaxQueuedTasks;
	if (m_queuedTasks[index].id == id)
	{
		task = &m_queuedTasks[index];
		return true;
	}
	return false;
}

TaskManager::QueuedTask* TaskManager::FindWork(TaskQueueFlags queues, bool bCanBlock)
{
	std::unique_lock<std::mutex> lock(m_taskAvailableLock);
	while (m_active)
	{
		if ((static_cast<int>(queues) & static_cast<int>(TaskQueueFlags::TimeCritical)) != 0)
		{
			QueuedTask* task = FindWork(m_taskQueues[static_cast<int>(TaskQueueIndex::TimeCritical)]);
			if (task != nullptr)
			{
				return task;
			}
		}
		if ((static_cast<int>(queues) & static_cast<int>(TaskQueueFlags::Normal)) != 0)
		{
			QueuedTask*task = FindWork(m_taskQueues[static_cast<int>(TaskQueueIndex::Normal)]);
			if (task != nullptr)
			{
				return task;
			}
		}
		if ((static_cast<int>(queues) & static_cast<int>(TaskQueueFlags::Long)) != 0)
		{
			QueuedTask* task = FindWork(m_taskQueues[static_cast<int>(TaskQueueIndex::Long)]);
			if (task != nullptr)
			{
				return task;
			}
		}

		if (!bCanBlock)
		{
			break;
		}

		{
			//ProfileScope scope(ProfileColors::Cpu, "Waiting for task");

			m_taskAvailableCondVariable.wait(lock);
		}
	}

	return nullptr;
}

TaskManager::QueuedTask* TaskManager::FindWork(TaskQueue& queue)
{
	for (int i = 0; i < queue.pendingTasks.size(); i++)
	{
		Task::Id id = queue.pendingTasks[i];

		QueuedTask* queuedTask = nullptr;
		bool isTaskRunnable = false;

		// Task still exists.
		if (GetTaskFromId(id, queuedTask))
		{
			// Task hasn't been picked up by another worker (it can be in multiple queues).
			if (!queuedTask->executing)
			{
				bool allPredecssorsCompleted = true;

				// All of the tasks predecessors has completed.
				for (Task::Id& predecessor : queuedTask->predecessors)
				{
					if (!IsComplete(predecessor))
					{
						allPredecssorsCompleted = false;
						break;
					}
				}

				if (allPredecssorsCompleted)
				{
					isTaskRunnable = true;
				}
			}
		}

		// If runnable, then we have work to return!
		if (isTaskRunnable)
		{
			queue.pendingTasks[i] = queue.pendingTasks.back();
			queue.pendingTasks.pop_back();
			queuedTask->executing = true;

			m_pendingTasks--;

			return queuedTask;
		}
	}

	return nullptr;
}

void TaskManager::CompleteTask(QueuedTask* task)
{
	bool hadSuccessors = task->hasSuccessors;

	m_queuedTaskCount--;
	FreeTaskId(task->id);

	// Task may have been a dependency, so notify workers to see if they 
	// have any further work to do.
	if (hadSuccessors)
	{
		std::unique_lock<std::mutex> lock(m_taskAvailableLock);
		m_taskAvailableCondVariable.notify_all();
	}
}

void TaskManager::Assist(TaskQueueFlags queues)
{
	RunTask(queues, false);
}

bool TaskManager::RunTask(TaskQueueFlags queues, bool bCanBlock)
{			
	TaskManager::QueuedTask* task = FindWork(queues, bCanBlock);
	if (task != nullptr)
	{
		if (task->task != nullptr)
		{
			task->task->Run();
		}
		CompleteTask(task);
		return true;
	}

	return false;
}

void TaskManager::WorkerLoop(TaskQueueFlags workQueues)
{
	while (m_active)
	{
		RunTask(workQueues, true);
	}
}
