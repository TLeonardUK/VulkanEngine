#pragma once
#include "Pch.h"

struct TimeoutCounter
{
private:
	float m_duration;
	std::chrono::high_resolution_clock::time_point m_startTime;

public:
	TimeoutCounter(float durationMilliseconds);

	bool HasFinished();
	float GetRemaining();

};

struct Timeout
{
private:
	float m_duration;

public:
	static Timeout Infinite;

public:
	Timeout(float durationMilliseconds);

	TimeoutCounter Start();
	float GetDuration();

};
