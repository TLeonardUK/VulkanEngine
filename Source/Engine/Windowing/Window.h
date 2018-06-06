#pragma once

#include <memory>

#include "Engine/Types/String.h"

enum class WindowMode
{
	Windowed,
	Fullscreen,
	BorderlessWindowed
};

class Logger;

class IWindow
{
protected:
	IWindow() { };

public:
	virtual ~IWindow() { };
	virtual void Dispose() = 0;

	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;
	virtual int GetRate() = 0;
	virtual WindowMode GetMode() = 0;
	virtual void SetTitle(const String& newTitle) = 0;
	virtual bool Resize(int width, int height, int rate, WindowMode mode) = 0;
};
