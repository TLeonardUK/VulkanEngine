#pragma once
#include "Pch.h"

#include "Engine/Types/String.h"
#include "Engine/Input/Input.h"

#include <SDL.h>

class IWindow;
class Logger;
class SdlPlatform;
class IPlatform;

class SdlInput
	: public IInput
{
private:
	enum class KeyStateFlags : int
	{
		Down = 1,
		Pressed = 2,
		Released = 4,
	};

	std::shared_ptr<Logger> m_logger;
	std::shared_ptr<IWindow> m_window;
	std::shared_ptr<SdlPlatform> m_platform;

	int m_mouseX;
	int m_mouseY;
	Uint32 m_mouseState;
	const Uint8* m_keyboardState;
	String m_pendingInput;

	int m_mouseWheel;
	int m_mouseWheelH;

	int m_keyStates[(int)InputKey::COUNT];

	SDL_Cursor* m_mouseCursors[(int)InputCursor::COUNT];

	bool Setup();
	void UpdateKeyState(int keyIndex, bool down);

	bool IsWindowInFocus();

public:
	SdlInput(std::shared_ptr<Logger> logger, std::shared_ptr<IWindow> window, std::shared_ptr<IPlatform> platform);
	virtual ~SdlInput();

	virtual void Dispose();

	virtual void PollInput();

	virtual bool IsKeyDown(InputKey key);
	virtual bool WasKeyPressed(InputKey key);
	virtual bool WasKeyReleased(InputKey key);

	virtual String GetClipboardText();
	virtual void SetClipboardText(const String& value);

	virtual bool IsModifierActive(InputModifier modifier);

	virtual String GetInput();

	virtual Vector2 GetMousePosition();
	virtual void SetMousePosition(Vector2 position);
	virtual int GetMouseWheel(bool horizontal);
	virtual void SetMouseCursor(InputCursor cursor);
	virtual void SetMouseCapture(bool capture);
	virtual void SetMouseHidden(bool hidden);

	static std::shared_ptr<IInput> Create(
		std::shared_ptr<Logger> logger, 
		std::shared_ptr<IWindow> window, 
		std::shared_ptr<IPlatform> platform);
};
