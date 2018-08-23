#pragma once
#include "Pch.h"

#include "Engine/Profiling/Profiling.h"

/*
struct NonRecursiveMutexTrait
{
	using MutexType = std::recursive_mutex;
};

struct RecursiveMutexTrait
{
	using MutexType = std::mutex;
};*/

//template <class TraitsClass = RecursiveMutexTrait>
struct Mutex
{
private:
	//typename TraitsClass::MutexType m_mutex;
	std::recursive_mutex m_mutex;

public:
	Mutex()
	{
	}

	void Lock()
	{
		m_mutex.lock();
	}

	void Unlock()
	{
		m_mutex.unlock();
	}

	bool TryLock()
	{
		return m_mutex.try_lock();
	}
};

struct ScopeLock
{
public:
	ScopeLock(Mutex& mutex)
		: m_mutex(mutex)
	{
		ProfileScope scope(ProfileColors::Synchronization, "Mutex Lock");

		m_mutex.Lock();
	}

	~ScopeLock()
	{
		m_mutex.Unlock();
	}

private:
	Mutex& m_mutex;

};
