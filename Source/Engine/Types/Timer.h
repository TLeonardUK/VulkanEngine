#pragma once
#include "Pch.h"

struct Timer
{
private:
	float m_elapsed;
	bool m_hasStarted;
	std::chrono::high_resolution_clock::time_point m_startTime;

public:
	Timer();

	void Start();
	void Stop();
	void Reset();

	float GetElapsed();

};
