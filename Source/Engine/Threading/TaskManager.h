#pragma once
#include "Pch.h"

#include "Engine/Types/Array.h"
#include "Engine/Types/String.h"
#include "Engine/Types/Timeout.h"
#include "Engine/Types/Dictionary.h"
#include "Engine/Types/Mutex.h"
#include "Engine/Threading/Event.h"

class Logger;

enum class TaskQueueFlags : int
{
	Normal = 1,			// General short-running tasks, normally per-frame things.
	Long = 2,			// Long-running tasks, such as resource loads.
	TimeCritical = 4,	// Time-critical things, these will be picked up by workers above all other queues.

	All = Normal | Long | TimeCritical,

	Count = 3
};

enum class TaskQueueIndex : int
{
	Normal = 0,
	Long = 1,
	TimeCritical = 2,

	COUNT = 3
};

class Task
{
public:
	typedef int Id;
	typedef std::shared_ptr<Task> Ptr;

public:
	virtual void Run() = 0;
	virtual String GetName() = 0;
};

class TaskManager 
{
private:
	static const int MaxQueuedTasks = 100000;

	struct QueuedTask
	{
		Task::Id			id;
		std::atomic<bool>	available;
		bool				executing;
		Task::Ptr			task;
		int					hasSuccessors;
		Array<Task::Id>		predecessors;
	};

	struct TaskQueue
	{
		Array<Task::Id> pendingTasks;
	};

	bool m_active;
	Array<std::thread> m_workers;
	std::shared_ptr<Logger> m_logger;

	QueuedTask m_queuedTasks[MaxQueuedTasks];
	std::atomic<int> m_nextTaskIndex;

	std::mutex m_taskAvailableLock;
	std::condition_variable m_taskAvailableCondVariable;

	std::atomic<int> m_queuedTaskCount;
	std::atomic<int> m_pendingTasks;

	TaskQueue m_taskQueues[(int)TaskQueueFlags::Count];

	Mutex m_availableTaskIdsLock; // todo: lockless queue.
	std::queue<Task::Id> m_availableTaskIds;

private:
	void WorkerLoop(TaskQueueFlags workQueues);
	QueuedTask* FindWork(TaskQueueFlags queues, bool bCanBlock);
	QueuedTask* FindWork(TaskQueue& queue);
	bool RunTask(TaskQueueFlags queues, bool bCanBlock);

	void CompleteTask(QueuedTask* task);

	Task::Id AllocTaskId();
	void FreeTaskId(Task::Id id);
	bool GetTaskFromId(Task::Id id, QueuedTask*& task);

	void Dispatch(Task::Id task, TaskQueue& queue);

public:
	static TaskManager* AsyncInstance;

public:
	TaskManager(std::shared_ptr<Logger> logger);
	~TaskManager();

	bool Init(Dictionary<TaskQueueFlags, int> queueWorkerCounts);
	void Dispose();

	int GetConcurrency();

	Task::Id CreateTask(Task::Ptr task = nullptr);
	void AddDependency(Task::Id predecessor, Task::Id successor);
	void Dispatch(Task::Id task, TaskQueueFlags queues = TaskQueueFlags::Normal);
	void Dispatch(Array<Task::Id> tasks, TaskQueueFlags queues = TaskQueueFlags::Normal);

	bool IsIdle();
	bool IsComplete(Task::Id task);

	bool WaitForCompletionEvent(Event& evnt, Timeout timeout = Timeout::Infinite);
	bool WaitForCompletion(Task::Id task, Timeout timeout = Timeout::Infinite);
	bool WaitForIdle(Timeout timeout = Timeout::Infinite);

	void Assist(TaskQueueFlags queues);

};
