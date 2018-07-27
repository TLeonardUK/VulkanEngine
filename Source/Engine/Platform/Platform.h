#pragma once
#include "Pch.h"

enum class ConsoleColor
{
	White,
	Red,
	Green,
	Blue,
	Yellow,

	COUNT
};

class Logger;

class IPlatform
{
protected:
	IPlatform() { };

public:
	virtual ~IPlatform() { };
	virtual void Dispose() = 0;

	virtual void PumpMessageQueue() = 0;
	virtual bool WasCloseRequested() = 0;

	virtual void SetLogger(std::shared_ptr<Logger> logger) = 0;

	virtual void WriteToConsole(ConsoleColor color, const String& data) = 0;
};