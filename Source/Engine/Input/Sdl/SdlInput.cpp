#include "Engine/Input/Sdl/SdlInput.h"
#include "Engine/Windowing/Sdl/SdlWindow.h"
#include "Engine/Engine/Logging.h"

#include <memory>

#include <SDL.h>
#include <SDL_video.h>

int InputKeyToSdlScancode[(int)InputKey::COUNT] = {

	SDL_SCANCODE_A,
	SDL_SCANCODE_B,
	SDL_SCANCODE_C,
	SDL_SCANCODE_D,
	SDL_SCANCODE_E,
	SDL_SCANCODE_F,
	SDL_SCANCODE_G,
	SDL_SCANCODE_H,
	SDL_SCANCODE_I,
	SDL_SCANCODE_J,
	SDL_SCANCODE_K,
	SDL_SCANCODE_L,
	SDL_SCANCODE_M,
	SDL_SCANCODE_N,
	SDL_SCANCODE_O,
	SDL_SCANCODE_P,
	SDL_SCANCODE_Q,
	SDL_SCANCODE_R,
	SDL_SCANCODE_S,
	SDL_SCANCODE_T,
	SDL_SCANCODE_U,
	SDL_SCANCODE_V,
	SDL_SCANCODE_W,
	SDL_SCANCODE_X,
	SDL_SCANCODE_Y,
	SDL_SCANCODE_Z,

	SDL_SCANCODE_1,
	SDL_SCANCODE_2,
	SDL_SCANCODE_3,
	SDL_SCANCODE_4,
	SDL_SCANCODE_5,
	SDL_SCANCODE_6,
	SDL_SCANCODE_7,
	SDL_SCANCODE_8,
	SDL_SCANCODE_9,
	SDL_SCANCODE_0,

	SDL_SCANCODE_RETURN,
	SDL_SCANCODE_ESCAPE,
	SDL_SCANCODE_BACKSPACE,
	SDL_SCANCODE_TAB,
	SDL_SCANCODE_SPACE,
	SDL_SCANCODE_MINUS,
	SDL_SCANCODE_EQUALS,
	SDL_SCANCODE_LEFTBRACKET,
	SDL_SCANCODE_RIGHTBRACKET,
	SDL_SCANCODE_BACKSLASH,
	SDL_SCANCODE_NONUSHASH,

	SDL_SCANCODE_SEMICOLON,
	SDL_SCANCODE_APOSTROPHE,
	SDL_SCANCODE_GRAVE,
	SDL_SCANCODE_COMMA,
	SDL_SCANCODE_PERIOD,
	SDL_SCANCODE_SLASH,

	SDL_SCANCODE_CAPSLOCK,

	SDL_SCANCODE_F1,
	SDL_SCANCODE_F2,
	SDL_SCANCODE_F3,
	SDL_SCANCODE_F4,
	SDL_SCANCODE_F5,
	SDL_SCANCODE_F6,
	SDL_SCANCODE_F7,
	SDL_SCANCODE_F8,
	SDL_SCANCODE_F9,
	SDL_SCANCODE_F10,
	SDL_SCANCODE_F11,
	SDL_SCANCODE_F12,
	SDL_SCANCODE_F13,
	SDL_SCANCODE_F14,
	SDL_SCANCODE_F15,
	SDL_SCANCODE_F16,
	SDL_SCANCODE_F17,
	SDL_SCANCODE_F18,
	SDL_SCANCODE_F19,
	SDL_SCANCODE_F20,
	SDL_SCANCODE_F21,
	SDL_SCANCODE_F22,
	SDL_SCANCODE_F23,
	SDL_SCANCODE_F24,

	SDL_SCANCODE_PRINTSCREEN,
	SDL_SCANCODE_SCROLLLOCK,
	SDL_SCANCODE_PAUSE,
	SDL_SCANCODE_INSERT,
	SDL_SCANCODE_HOME,
	SDL_SCANCODE_PAGEUP,
	SDL_SCANCODE_DELETE,
	SDL_SCANCODE_END,
	SDL_SCANCODE_PAGEDOWN,

	SDL_SCANCODE_RIGHT,
	SDL_SCANCODE_LEFT,
	SDL_SCANCODE_DOWN,
	SDL_SCANCODE_UP,

	SDL_SCANCODE_KP_DIVIDE,
	SDL_SCANCODE_KP_MULTIPLY,
	SDL_SCANCODE_KP_MINUS,
	SDL_SCANCODE_KP_PLUS,
	SDL_SCANCODE_KP_ENTER,
	SDL_SCANCODE_KP_1,
	SDL_SCANCODE_KP_2,
	SDL_SCANCODE_KP_3,
	SDL_SCANCODE_KP_4,
	SDL_SCANCODE_KP_5,
	SDL_SCANCODE_KP_6,
	SDL_SCANCODE_KP_7,
	SDL_SCANCODE_KP_8,
	SDL_SCANCODE_KP_9,
	SDL_SCANCODE_KP_0,
	SDL_SCANCODE_KP_PERIOD,

	SDL_SCANCODE_LCTRL,
	SDL_SCANCODE_LSHIFT,
	SDL_SCANCODE_LALT,
	SDL_SCANCODE_LGUI,
	SDL_SCANCODE_RCTRL,
	SDL_SCANCODE_RSHIFT,
	SDL_SCANCODE_RALT,
	SDL_SCANCODE_RGUI,

	0,
	0,
	0,
	0,
	0
};

SdlInput::SdlInput(std::shared_ptr<Logger> logger, std::shared_ptr<IWindow> window)
	: m_logger(logger)
	, m_window(window)
{
}

SdlInput::~SdlInput()
{
	Dispose();
}

void SdlInput::Dispose()
{
}

bool SdlInput::Setup()
{
	SDL_CaptureMouse(SDL_TRUE);
	SDL_ShowCursor(SDL_FALSE);

	return true;
}

void SdlInput::UpdateKeyState(int keyIndex, bool down)
{
	if (down)
	{
		if ((m_keyStates[keyIndex] & (int)KeyStateFlags::Down) != 0)
		{
			m_keyStates[keyIndex] = (int)KeyStateFlags::Down;
		}
		else
		{
			m_keyStates[keyIndex] = (int)KeyStateFlags::Down | (int)KeyStateFlags::Pressed;
		}
	}
	else
	{
		if ((m_keyStates[keyIndex] & (int)KeyStateFlags::Down) != 0)
		{
			m_keyStates[keyIndex] = (int)KeyStateFlags::Released;
		}
		else
		{
			m_keyStates[keyIndex] = 0;
		}
	}
}

bool SdlInput::IsWindowInFocus()
{
	std::shared_ptr<SdlWindow> window = std::dynamic_pointer_cast<SdlWindow>(m_window);
	Uint32 flags = SDL_GetWindowFlags(window->GetSdlHandle());

	return (window != nullptr && (flags & SDL_WINDOW_INPUT_FOCUS) != 0);
}

void SdlInput::PollInput()
{
	if (!IsWindowInFocus())
	{
		return;
	}

	m_mouseState = SDL_GetMouseState(&m_mouseX, &m_mouseY);
	m_keyboardState = SDL_GetKeyboardState(nullptr);

	for (int i = 0; i < (int)InputKey::Mouse_0; i++)
	{
		bool down = (m_keyboardState[InputKeyToSdlScancode[i]] != 0);
		UpdateKeyState(i, down);
	}

	for (int i = (int)InputKey::Mouse_0; i <= (int)InputKey::Mouse_5; i++)
	{
		int bit = i - (int)InputKey::Mouse_0;
		bool down = (m_mouseState & (1 << bit)) != 0;

		UpdateKeyState(i, down);
	}
}

Vector2 SdlInput::GetMousePosition()
{
	return Vector2(m_mouseX, m_mouseY);
}

void SdlInput::SetMousePosition(Vector2 position)
{
	std::shared_ptr<SdlWindow> window = std::dynamic_pointer_cast<SdlWindow>(m_window);
	if (IsWindowInFocus())
	{
		SDL_WarpMouseInWindow(window->GetSdlHandle(), (int)position.x, (int)position.y);
	}
}

bool SdlInput::IsKeyDown(InputKey key)
{
	return (m_keyStates[(int)key] & (int)KeyStateFlags::Down) != 0;
}

bool SdlInput::WasKeyPressed(InputKey key)
{
	return (m_keyStates[(int)key] & (int)KeyStateFlags::Pressed) != 0;
}

bool SdlInput::WasKeyReleased(InputKey key)
{
	return (m_keyStates[(int)key] & (int)KeyStateFlags::Released) != 0;
}

std::shared_ptr<IInput> SdlInput::Create(
	std::shared_ptr<Logger> logger,
	std::shared_ptr<IWindow> window
	)
{
	std::shared_ptr<SdlInput> handle = std::make_shared<SdlInput>(logger, window);

	if (!handle->Setup())
	{
		handle->Dispose();
		return nullptr;
	}

	return handle;
}