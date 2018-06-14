#pragma once

#include <memory>
#include <functional>

#include "Engine/Types/String.h"
#include "Engine/Types/Array.h"
#include "Engine/Platform/Platform.h"

#include <SDL.h>

class Logger;

#if defined(_WIN32)
#include <Windows.h>
#endif

class SdlPlatform : public IPlatform
{
public:
	typedef std::function<bool(SDL_Event& event)> SdlEventCallback_t;

private:
#if defined(_WIN32)
	HANDLE m_win32Console;
#endif

	std::shared_ptr<Logger> m_logger;

	Array<SdlEventCallback_t> m_eventCallbacks;

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

	void RegisterSdlEventCallback(SdlEventCallback_t callback);

	static std::shared_ptr<IPlatform> Create();
};