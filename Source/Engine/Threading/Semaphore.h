#pragma once

#include <mutex>
#include <condition_variable>

class Semaphore 
{
public:
	Semaphore(int count = 0)
		: m_count(count) 
	{
	}

	inline void Signal()
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		m_count++;
		m_conditionVariable.notify_one();
	}

	inline void Wait()
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		while (m_count == 0)
		{
			m_conditionVariable.wait(lock);
		}

		m_count--;
	}

private:
	std::mutex m_mutex;
	std::condition_variable m_conditionVariable;
	int m_count;

};