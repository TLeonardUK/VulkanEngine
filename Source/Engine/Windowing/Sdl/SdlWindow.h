#pragma once

#include <memory>

#include "Engine/Types/String.h"
#include "Engine/Windowing/Window.h"

class IGraphics;
class Logger;

struct SDL_Window;

class SdlWindow 
	: public IWindow
{
private:
	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<IGraphics> m_graphics;

	int m_width;
	int m_height;
	int m_rate;
	WindowMode m_mode;

	SDL_Window* m_window;

	bool SetupWindow();

public:
	SdlWindow(std::shared_ptr<Logger> logger, std::shared_ptr<IGraphics> graphics);
	virtual ~SdlWindow();
	virtual void Dispose();

	virtual int GetWidth();
	virtual int GetHeight();
	virtual int GetRate();
	virtual WindowMode GetMode();

	virtual void SetTitle(const String& newTitle);
	virtual bool Resize(int width, int height, int rate, WindowMode mode);

	SDL_Window* GetSdlHandle();

	static std::shared_ptr<IWindow> Create(
		std::shared_ptr<Logger> logger,
		std::shared_ptr<IGraphics> graphics,
		const String& name,
		int width,
		int height,
		int rate,
		WindowMode mode);
};
