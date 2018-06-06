#pragma once

#include <memory>

#include "Engine/Types/String.h"
#include "Engine/Platform/Platform.h"

class Logger;

#if defined(_WIN32)
#include <Windows.h>
#endif

class SdlPlatform : public IPlatform
{
private:
#if defined(_WIN32)
	HANDLE m_win32Console;
#endif

	std::shared_ptr<Logger> m_logger;

	bool m_closeRequested;

	bool SetupPlatform();

public:
	SdlPlatform();
	virtual ~SdlPlatform();
	virtual void Dispose();

	virtual void PumpMessageQueue();
	virtual bool WasCloseRequested();

	virtual void SetLogger(std::shared_ptr<Logger> logger);

	virtual void WriteToConsole(ConsoleColor color, const String& data);

	static std::shared_ptr<IPlatform> Create();
};