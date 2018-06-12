#pragma once

#include <memory>

#include "Engine/Types/String.h"
#include "Engine/Input/Input.h"

#include <SDL.h>

class IWindow;
class Logger;

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

	int m_mouseX;
	int m_mouseY;
	Uint32 m_mouseState;
	const Uint8* m_keyboardState;

	int m_keyStates[(int)InputKey::COUNT];

	bool Setup();
	void UpdateKeyState(int keyIndex, bool down);

	bool IsWindowInFocus();

public:
	SdlInput(std::shared_ptr<Logger> logger, std::shared_ptr<IWindow> window);
	virtual ~SdlInput();

	virtual void Dispose();

	virtual void PollInput();

	virtual Vector2 GetMousePosition();
	virtual void SetMousePosition(Vector2 position);

	virtual bool IsKeyDown(InputKey key);
	virtual bool WasKeyPressed(InputKey key);
	virtual bool WasKeyReleased(InputKey key);

	static std::shared_ptr<IInput> Create(
		std::shared_ptr<Logger> logger, 
		std::shared_ptr<IWindow> window);
};
