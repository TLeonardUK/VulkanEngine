#pragma once
#include "Pch.h"

class Event
{
public:
	Event()
	{
	}

	inline void Signal()
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		m_signaled = true;
		m_conditionVariable.notify_all();
	}

	inline void Wait()
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		while (!m_signaled)
		{
			m_conditionVariable.wait(lock);
		}
	}

	inline bool IsSignaled()
	{
		return m_signaled;
	}

	inline void Reset()
	{
		m_signaled = false;
	}

private:
	std::mutex m_mutex;
	std::condition_variable m_conditionVariable;
	bool m_signaled;

};