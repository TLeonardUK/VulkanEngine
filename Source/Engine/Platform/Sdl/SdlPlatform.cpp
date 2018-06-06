#include "Engine/Platform/Sdl/SdlPlatform.h"
#include "Engine/Engine/Logging.h"

#include <SDL.h>
#include <SDL_video.h>

#if defined(_WIN32)
#include <windows.h>
#endif

SdlPlatform::SdlPlatform()
	: m_closeRequested(false)
{
}

SdlPlatform::~SdlPlatform()
{
}

void SdlPlatform::Dispose()
{
	SDL_EnableScreenSaver();
	SDL_Quit();
}

bool SdlPlatform::SetupPlatform()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		return false;
	}

	SDL_DisableScreenSaver();

#if defined(_WIN32)
	m_win32Console = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

	return true;
}

void SdlPlatform::PumpMessageQueue()
{
	SDL_Event sdlEvent;
	while (SDL_PollEvent(&sdlEvent))
	{
		switch (sdlEvent.type)
		{
		case SDL_QUIT:
			{
				m_closeRequested = true;
				break;
			}
		}
	}
}

bool SdlPlatform::WasCloseRequested()
{
	return m_closeRequested;
}

void SdlPlatform::SetLogger(std::shared_ptr<Logger> logger)
{
	m_logger = logger;

	m_logger->WriteInfo(LogCategory::SdlPlatform, "SDL Version: %i.%i.%i", 
		SDL_MAJOR_VERSION, 
		SDL_MINOR_VERSION, 
		SDL_PATCHLEVEL);
}

void SdlPlatform::WriteToConsole(ConsoleColor color, const String& data)
{
#if defined(_WIN32)
	WORD colorBits[(int)ConsoleColor::COUNT] =
	{
		/* White  */ FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
		/* Red    */ FOREGROUND_INTENSITY | FOREGROUND_RED,
		/* Green  */ FOREGROUND_INTENSITY | FOREGROUND_GREEN,
		/* Blue   */ FOREGROUND_INTENSITY | FOREGROUND_BLUE,
		/* Yellow */ FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN
	};

	SetConsoleTextAttribute(m_win32Console, colorBits[(int)color]);

	WriteConsole(m_win32Console, (char*)data.data(), (DWORD)data.size(), nullptr, nullptr);

	OutputDebugStringA(data.c_str());
#else
	printf("%s", data.c_str());
#endif
}

std::shared_ptr<IPlatform> SdlPlatform::Create()
{
	std::shared_ptr<SdlPlatform> handle = std::make_shared<SdlPlatform>();
	if (!handle->SetupPlatform())
	{
		handle->Dispose();
		return nullptr;
	}

	return handle;
}