#include "Pch.h"

#include "Engine/Types/Timer.h"

Timer::Timer()
{
	m_startTime = std::chrono::high_resolution_clock::now();
	m_elapsed = 0.0f;
	m_hasStarted = false;
}

float Timer::GetElapsed()
{
	return m_elapsed;
}

void Timer::Stop()
{
	assert(m_hasStarted);
	m_hasStarted = false;

	auto currentTime = std::chrono::high_resolution_clock::now();
	float elapsed = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - m_startTime).count();

	m_elapsed += elapsed;
}

void Timer::Start()
{
	assert(!m_hasStarted);
	m_hasStarted = true;

	m_startTime = std::chrono::high_resolution_clock::now();
}

void Timer::Reset()
{
	m_elapsed = 0.0f;
	m_hasStarted = false;
}
