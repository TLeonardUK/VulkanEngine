#include "Pch.h"

#include "Engine/Types/Timeout.h"

Timeout Timeout::Infinite(-1.0f);

TimeoutCounter::TimeoutCounter(float durationMilliseconds)
{
	m_startTime = std::chrono::high_resolution_clock::now();
	m_duration = durationMilliseconds;
}

float TimeoutCounter::GetRemaining()
{
	auto currentTime = std::chrono::high_resolution_clock::now();
	float elapsed = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - m_startTime).count();

	if (m_duration < 0.0f)
	{
		return std::numeric_limits<float>::infinity();
	}

	return std::max(0.0f, m_duration - elapsed);
}

bool TimeoutCounter::HasFinished()
{
	// Anything below 0 is considered infinite.
	if (m_duration <= 0.0f)
	{
		return false;
	}

	auto currentTime = std::chrono::high_resolution_clock::now();
	float elapsed = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - m_startTime).count();

	return elapsed >= m_duration;
}

Timeout::Timeout(float durationMilliseconds)
	: m_duration(durationMilliseconds)
{
}

TimeoutCounter Timeout::Start()
{
	return TimeoutCounter(m_duration);
}

float Timeout::GetDuration()
{
	return m_duration;
}