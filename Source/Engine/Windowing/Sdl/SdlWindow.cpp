#include "Pch.h"

#include "Engine/Windowing/Sdl/SdlWindow.h"
#include "Engine/Engine/Logging.h"

#include "Engine/Graphics/Vulkan/VulkanGraphics.h"

#include <SDL.h>
#include <SDL_video.h>

SdlWindow::SdlWindow(std::shared_ptr<Logger> logger, std::shared_ptr<IGraphics> graphics)
	: m_window(nullptr)
	, m_logger(logger)
	, m_graphics(graphics)
{
}

SdlWindow::~SdlWindow()
{
	Dispose();
}

void SdlWindow::Dispose()
{
	if (m_window)
	{
		m_logger->WriteInfo(LogCategory::Windowing, "Destroying sdl window.");

		SDL_DestroyWindow(m_window);
		m_window = nullptr;
	}
}

bool SdlWindow::SetupWindow()
{
	m_logger->WriteInfo(LogCategory::Windowing, "Creating sdl window.");

	Uint32 windowFlags = SDL_WINDOW_RESIZABLE;

	std::shared_ptr<VulkanGraphics> vulkanGraphics = std::static_pointer_cast<VulkanGraphics>(m_graphics);
	if (vulkanGraphics)
	{
		windowFlags |= SDL_WINDOW_VULKAN;
	}
	else
	{
		m_logger->WriteError(LogCategory::Windowing, "Failed to create sdl window, support for current graphics driver not available or unimplemented.");
		return false;
	}

	m_window = SDL_CreateWindow(
		"",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		640,
		480,
		windowFlags);

	if (m_window == nullptr)
	{
		m_logger->WriteError(LogCategory::Windowing, "Failed to create sdl window: %s", SDL_GetError());
		return false;
	}

	return true;
}

void SdlWindow::SetTitle(const String& newTitle)
{
	SDL_SetWindowTitle(m_window, newTitle.c_str());
}

bool SdlWindow::Resize(int width, int height, int rate, WindowMode mode)
{
	SDL_bool bHasBorder = SDL_TRUE;
	Uint32 fullscreenMode = 0;
	SDL_DisplayMode displayMode;

	switch (mode)
	{
	case WindowMode::Fullscreen:
		{
			m_logger->WriteInfo(LogCategory::Windowing, "Changing to fullscreen window: width=%i height=%i rate=%i",
				width, height, rate, mode);

			bHasBorder = SDL_FALSE;
			fullscreenMode = SDL_WINDOW_FULLSCREEN;

			SDL_DisplayMode targetMode;
			targetMode.w = width;
			targetMode.h = height;
			targetMode.format = 0;
			targetMode.refresh_rate = rate;
			targetMode.driverdata = 0;

			if (SDL_GetClosestDisplayMode(0, &targetMode, &displayMode) == NULL)
			{
				m_logger->WriteError(LogCategory::Windowing, "Failed to find suitable display mode: %s", SDL_GetError());
				return false;
			}

			break;
		}
	case WindowMode::Windowed:
		{
			m_logger->WriteInfo(LogCategory::Windowing, "Changing to windowed window: width=%i height=%i rate=%i",
				width, height, rate, mode);

			bHasBorder = SDL_TRUE;
			fullscreenMode = 0;

			if (SDL_GetDesktopDisplayMode(0, &displayMode) != 0)
			{
				m_logger->WriteError(LogCategory::Windowing, "Failed to find suitable display mode: %s", SDL_GetError());
				return false;
			}

			break;
		}
	case WindowMode::BorderlessWindowed:
		{			
			m_logger->WriteInfo(LogCategory::Windowing, "Changing to borderless windowed: width=%i height=%i rate=%i",
				width, height, rate, mode);

			bHasBorder = SDL_FALSE;
			fullscreenMode = SDL_WINDOW_FULLSCREEN_DESKTOP;

			if (SDL_GetDesktopDisplayMode(0, &displayMode) != 0)
			{
				m_logger->WriteError(LogCategory::Windowing, "Failed to find suitable display mode: %s", SDL_GetError());
				return false;
			}

			break;
		}
	}

	m_logger->WriteInfo(LogCategory::Windowing, "Display mode selected for use: width=%i height=%i rate=%i format=0x%08x",
		displayMode.w,
		displayMode.h,
		displayMode.refresh_rate,
		displayMode.format);

	SDL_SetWindowBordered(m_window, bHasBorder);
	SDL_SetWindowSize(m_window, width, height);
	SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

	int result = SDL_SetWindowDisplayMode(m_window, &displayMode);
	if (result != 0)
	{
		m_logger->WriteError(LogCategory::Windowing, "Failed to set sdl display mode: %s", SDL_GetError());
		return false;
	}

	result = SDL_SetWindowFullscreen(m_window, fullscreenMode);
	if (result != 0)
	{
		m_logger->WriteError(LogCategory::Windowing, "Failed to set sdl window fullscreen mode: %s", SDL_GetError());
		return false;
	}

	//SDL_SetWindowBordered();
	//SDL_SetWindowSize(m_window, width, height);
	//SDL_SetWindowFullscreen();
	//SDL_SetWindowDisplayMode();

	m_width = width;
	m_height = height;
	m_rate = rate;
	m_mode = mode;

	return true;
}

SDL_Window* SdlWindow::GetSdlHandle()
{
	return m_window;
}

int SdlWindow::GetWidth()
{
	int width, height;
	SDL_GetWindowSize(m_window, &width, &height);
	return width;
}

int SdlWindow::GetHeight()
{
	int width, height;
	SDL_GetWindowSize(m_window, &width, &height);
	return height;
}

int SdlWindow::GetRate()
{
	return m_rate;
}

WindowMode SdlWindow::GetMode()
{
	return m_mode;
}

std::shared_ptr<IWindow> SdlWindow::Create(
	std::shared_ptr<Logger> logger,
	std::shared_ptr<IGraphics> graphics,
	const String& name,
	int width,
	int height,
	int rate,
	WindowMode mode)
{
	std::shared_ptr<SdlWindow> handle = std::make_shared<SdlWindow>(logger, graphics);

	if (!handle->SetupWindow())
	{
		handle->Dispose();
		return nullptr;
	}

	handle->SetTitle(name);

	if (!handle->Resize(width, height, rate, mode))
	{
		handle->Dispose();
		return nullptr;
	}

	return handle;
}